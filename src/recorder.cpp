// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.

#include "recorder.h"
#include <ctime>
#include <atomic>
#include <iostream>
#include <algorithm>
#include <thread>
#include <assert.h>
#include <time.h>

#include <k4a/k4a.h>
#include <k4arecord/record.h>

inline static uint32_t k4a_convert_fps_to_uint(k4a_fps_t fps)
{
    uint32_t fps_int;
    switch (fps)
    {
    case K4A_FRAMES_PER_SECOND_5:
        fps_int = 5;
        break;
    case K4A_FRAMES_PER_SECOND_15:
        fps_int = 15;
        break;
    case K4A_FRAMES_PER_SECOND_30:
        fps_int = 30;
        break;
    default:
        fps_int = 0;
        break;
    }
    return fps_int;
}

// call k4a_device_close on every failed CHECK
#define CHECK(x, device)                                                                                               \
    {                                                                                                                  \
        auto retval = (x);                                                                                             \
        if (retval)                                                                                                    \
        {                                                                                                              \
            std::cerr << "Runtime error: " << #x << " returned " << retval << std::endl;                               \
            k4a_device_close(device);                                                                                  \
            return 1;                                                                                                  \
        }                                                                                                              \
    }

std::atomic_bool exiting(false);

int do_recording(uint8_t device_index,
                 std::string base_filename,
                 int max_block_length,
                 k4a_device_configuration_t *device_config,
                 bool record_imu,
                 int32_t absoluteExposureValue,
                 int32_t gain)
{
    const uint32_t installed_devices = k4a_device_get_installed_count();
    if (device_index >= installed_devices)
    {
        std::cerr << "Device not found." << std::endl;
        return 1;
    }

    k4a_device_t device;
    if (K4A_FAILED(k4a_device_open(device_index, &device)))
    {
        std::cerr << "Runtime error: k4a_device_open() failed " << std::endl;
    }

    char serial_number_buffer[256];
    size_t serial_number_buffer_size = sizeof(serial_number_buffer);
    CHECK(k4a_device_get_serialnum(device, serial_number_buffer, &serial_number_buffer_size), device);

    std::cout << "Device serial number: " << serial_number_buffer << std::endl;

    k4a_hardware_version_t version_info;
    CHECK(k4a_device_get_version(device, &version_info), device);

    std::cout << "Device version: " << (version_info.firmware_build == K4A_FIRMWARE_BUILD_RELEASE ? "Rel" : "Dbg")
              << "; C: " << version_info.rgb.major << "." << version_info.rgb.minor << "." << version_info.rgb.iteration
              << "; D: " << version_info.depth.major << "." << version_info.depth.minor << "."
              << version_info.depth.iteration << "[" << version_info.depth_sensor.major << "."
              << version_info.depth_sensor.minor << "]"
              << "; A: " << version_info.audio.major << "." << version_info.audio.minor << "."
              << version_info.audio.iteration << std::endl;

    uint32_t camera_fps = k4a_convert_fps_to_uint(device_config->camera_fps);

    if (camera_fps <= 0 || (device_config->color_resolution == K4A_COLOR_RESOLUTION_OFF &&
                            device_config->depth_mode == K4A_DEPTH_MODE_OFF))
    {
        std::cerr << "Either the color or depth modes must be enabled to record." << std::endl;
        return 1;
    }

    if (absoluteExposureValue != defaultExposureAuto)
    {
        if (K4A_FAILED(k4a_device_set_color_control(device,
                                                    K4A_COLOR_CONTROL_EXPOSURE_TIME_ABSOLUTE,
                                                    K4A_COLOR_CONTROL_MODE_MANUAL,
                                                    absoluteExposureValue)))
        {
            std::cerr << "Runtime error: k4a_device_set_color_control() for manual exposure failed " << std::endl;
        }
    }
    else
    {
        if (K4A_FAILED(k4a_device_set_color_control(device,
                                                    K4A_COLOR_CONTROL_EXPOSURE_TIME_ABSOLUTE,
                                                    K4A_COLOR_CONTROL_MODE_AUTO,
                                                    0)))
        {
            std::cerr << "Runtime error: k4a_device_set_color_control() for auto exposure failed " << std::endl;
        }
    }

    if (gain != defaultGainAuto)
    {
        if (K4A_FAILED(
                k4a_device_set_color_control(device, K4A_COLOR_CONTROL_GAIN, K4A_COLOR_CONTROL_MODE_MANUAL, gain)))
        {
            std::cerr << "Runtime error: k4a_device_set_color_control() for manual gain failed " << std::endl;
        }
    }

    CHECK(k4a_device_start_cameras(device, device_config), device);
    if (record_imu)
    {
        CHECK(k4a_device_start_imu(device), device);
    }

    std::cout << "Device started" << std::endl;

    // Wait for the first capture before starting recording.
    k4a_capture_t capture;
    int32_t timeout_sec_for_first_capture = 60;
    if (device_config->wired_sync_mode == K4A_WIRED_SYNC_MODE_SUBORDINATE)
    {
        timeout_sec_for_first_capture = 360;
        std::cout << "[subordinate mode] Waiting for signal from master" << std::endl;
    }
    int first_capture_start = time(NULL);
    k4a_wait_result_t result;
    // Wait for the first capture in a loop so Ctrl-C will still exit.
    while (!exiting && (time(NULL) - first_capture_start) < timeout_sec_for_first_capture)
    {
        result = k4a_device_get_capture(device, &capture, 100);
        if (result == K4A_WAIT_RESULT_SUCCEEDED)
        {
            k4a_capture_release(capture);
            break;
        }
        else if (result == K4A_WAIT_RESULT_FAILED)
        {
            std::cerr << "Runtime error: k4a_device_get_capture() returned error: " << result << std::endl;
            return 1;
        }
    }

    if (exiting)
    {
        // sighandler sets exiting flag.. we can flush the record here
        k4a_device_close(device);
        return 0;
    }
    else if (result == K4A_WAIT_RESULT_TIMEOUT)
    {
        std::cerr << "Timed out waiting for first capture." << std::endl;
        return 1;
    }


    std::cout << "Started recording" << std::endl;
    std::cout << "Press Ctrl-C to stop recording." << std::endl;

    size_t file_counter = 0;

    auto current_recording = std::make_unique<k4a_record_t>();
    auto next_recording = std::make_unique<k4a_record_t>();
    std::thread backup_thread;

    std::atomic<bool> ext_flush_done{false};

    while(1) {

        std::string recording_filename = next_record_name(base_filename, file_counter);
        if (K4A_FAILED(k4a_record_create(recording_filename.c_str(), device, *device_config, current_recording.get())))
        {
            std::cerr << "Unable to create recording file: " << recording_filename << std::endl;
            return 1;
        }

        std::cout << "Created file: " << recording_filename << std::endl;

        if (record_imu)
        {
            CHECK(k4a_record_add_imu_track(*current_recording), device);
        }
        CHECK(k4a_record_write_header(*current_recording), device);

        clock_t recording_start = clock();
        int32_t timeout_ms = 1000 / camera_fps;
        do
        {
            result = k4a_device_get_capture(device, &capture, timeout_ms);
            if (result == K4A_WAIT_RESULT_TIMEOUT)
            {
                continue;
            }
            else if (result != K4A_WAIT_RESULT_SUCCEEDED)
            {
                std::cerr << "Runtime error: k4a_device_get_capture() returned " << result << std::endl;
                break;
            }
            CHECK(k4a_record_write_capture(*current_recording, capture), device);
            k4a_capture_release(capture);

            if (backup_thread.joinable() && ext_flush_done) {
                backup_thread.join();
                ext_flush_done = false;
            }

            if (record_imu)
            {
                do
                {
                    k4a_imu_sample_t sample;
                    result = k4a_device_get_imu_sample(device, &sample, 0);
                    if (result == K4A_WAIT_RESULT_TIMEOUT)
                    {
                        break;
                    }
                    else if (result != K4A_WAIT_RESULT_SUCCEEDED)
                    {
                        std::cerr << "Runtime error: k4a_imu_get_sample() returned " << result << std::endl;
                        break;
                    }
                    k4a_result_t write_result = k4a_record_write_imu_sample(*current_recording, sample);
                    if (K4A_FAILED(write_result))
                    {
                        std::cerr << "Runtime error: k4a_record_write_imu_sample() returned " << write_result << std::endl;
                        break;
                    }
                } while (!exiting && result != K4A_WAIT_RESULT_FAILED &&
                         (clock() - recording_start < max_block_length * CLOCKS_PER_SEC));
            }
        } while (!exiting && result != K4A_WAIT_RESULT_FAILED &&
                 (clock() - recording_start < max_block_length * CLOCKS_PER_SEC));

        std::cout << "Saving recording: " << recording_filename << std::endl;

        std::swap(current_recording, next_recording);

        // if ext_flush_done thread was unable to write.. can we recover?
        assert(!ext_flush_done);

        backup_thread = std::thread([&ext_flush_done](k4a_record_t record, k4a_device_t device) {
                CHECK(k4a_record_flush(record), device)
                k4a_record_close(record);
                ext_flush_done = true;
        }, *next_recording, device);

        ++file_counter;
    }

    if (!exiting)
    {
        exiting = true;
        std::cout << "Stopping recording..." << std::endl;
    }

    if (record_imu)
    {
        k4a_device_stop_imu(device);
    }
    k4a_device_stop_cameras(device);

    std::cout << "Done" << std::endl;

    k4a_device_close(device);

    return 0;
}

std::string next_record_name(std::string base, uint32_t counter) {
    const uint8_t N_zero = 6;
    std::vector<std::string> strs;
    boost::split(strs, base, boost::is_any_of("."));
    auto cn = std::to_string(counter);
    return strs[0] + '_' + std::string(N_zero - cn.length(), '0') + cn + '.' + strs[1];
}

#pragma once

#include <atomic>
#include <thread>
#include <k4a/k4a.h>
#include <k4arecord/record.h>

extern std::atomic_bool exiting;
extern std::thread backup_thread;

static const int32_t defaultExposureAuto = -12;
static const int32_t defaultGainAuto = -1;

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


int do_recording(uint8_t device_index,
                 std::string base_filename,
                 int max_block_length,
                 k4a_device_configuration_t *device_config,
                 bool record_imu,
                 int32_t absoluteExposureValue,
                 int32_t gain);

std::string next_record_name(std::string base, uint32_t counter);

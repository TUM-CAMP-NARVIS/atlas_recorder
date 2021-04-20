#pragma once

#include <atomic>
#include <k4a/k4a.h>
#include <k4arecord/record.h>

#include <boost/algorithm/string.hpp>

extern std::atomic_bool exiting;

static const int32_t defaultExposureAuto = -12;
static const int32_t defaultGainAuto = -1;

int do_recording(uint8_t device_index,
                 std::string base_filename,
                 int max_block_length,
                 k4a_device_configuration_t *device_config,
                 bool record_imu,
                 int32_t absoluteExposureValue,
                 int32_t gain);

std::string next_record_name(std::string base, uint32_t counter);

// SPDX-License-Identifier: MIT
#pragma once
#include <cstdint>

#if defined(HAPTIC_ATOMS3R_CAM)
// Official CAM Grove: G1 white / G2 yellow / 5V red / GND black.
// CAM reset is NOT an application button. Use an external momentary switch.
  #if defined(HAPTIC_U059)
constexpr uint8_t kMotor = 2, kButton = 1;
constexpr uint32_t kPwmFrequency = 10000;
  #else
constexpr uint8_t kMotor = 1, kButton = 2;
constexpr uint32_t kPwmFrequency = 20000;
  #endif
#else
// Classic ESP32-WROOM / DevKitC V4 only; do not apply to S3/C3 boards.
constexpr uint8_t kMotor = 25, kButton = 32;
constexpr uint8_t kSck = 18, kMiso = 19, kMosi = 23, kCs = 27;
constexpr uint32_t kPwmFrequency = 20000;
#endif
constexpr uint8_t kPwmChannel = 0;

// SPDX-License-Identifier: MIT
#pragma once
#include <M5Unified.h> // Public header supplies m5::board_t and IMU utility types.
#include "shake_detector.h"

// Use the vendor BMI270 initialization/configuration path. No M5.begin(),
// external I2C, camera/audio initialization, NVS writes or raw sample logging.
class MotionSensor {
 public:
  bool begin() {
    if (ready_) return true;
    if (!bus_.begin(I2C_NUM_0, 45, 0)) return false; // SDA45 / SCL0, internal
    ready_ = imu_.begin(&bus_, m5::board_t::board_M5AtomS3RCam)
             && imu_.getType() == m5::imu_t::imu_bmi270;
    // Verify Bosch INIT_OK independently of the older vendor driver's return.
    // INTERNAL_STATUS=0x21; config-load status mask=0x0F, INIT_OK=1.
    uint8_t status = 0;
    ready_ = ready_ && bus_.readRegister(0x68, 0x21, &status, 1, 400000)
             && (status & 0x0F) == 0x01;
    if (ready_) {
      imu_.clearOffsetData(); // Ignore another app's old calibration in RAM only.
      imu_.setCalibration(0, 0, 0);
    }
    return ready_;
  }
  bool read(float& x, float& y, float& z) {
    if (!ready_ || !(imu_.update() & m5::IMU_Class::sensor_mask_accel)) return false;
    const auto& data = imu_.getImuData();
    x = data.accel.x; y = data.accel.y; z = data.accel.z;
    return true;
  }
 private:
  m5::I2C_Class bus_;
  m5::IMU_Class imu_;
  bool ready_ = false;
};

// SPDX-License-Identifier: MIT
#pragma once
#include <cmath>
#include <cstdint>

namespace haptic {
// Starting thresholds for a deliberate double-pulse shake, not a learned model.
// Acceleration is in g. Real mounting/motor conditions require calibration tests.
class ShakeDetector {
 public:
  enum class State { Off, Calibrating, Armed };
  static constexpr uint32_t kCalibrationMs = 2000, kCalibrationTimeoutMs = 10000;
  static constexpr uint32_t kMaxSampleGapMs = 100, kSettleMs = 500;
  static constexpr float kPeakG = 0.75f, kReleaseG = 0.25f;

  void arm(uint32_t now) {
    state_ = State::Calibrating;
    arm_at_ = now;
    samples_ = 0;
    have_time_ = false;
    needs_quiet_ = false;
    has_fired_ = false;
    quiet_ = false;
    resetPeaks();
  }
  void disarm() { state_ = State::Off; have_time_ = false; resetPeaks(); }
  State state() const { return state_; }
  const char* stateName() const {
    return state_ == State::Off ? "off" : state_ == State::Calibrating ? "calibrating" : "armed";
  }
  // Known actuator activity is different from unexplained stale sensor data.
  // Pause reads during playback, then require fresh quiet samples before reuse.
  void suppress() {
    if (state_ == State::Calibrating) { disarm(); return; }
    if (state_ == State::Off) return;
    needs_quiet_ = true;
    quiet_ = false;
    have_time_ = false;
    resetPeaks();
  }
  bool sample(float x, float y, float z, uint32_t now, bool valid = true) {
    if (state_ == State::Off) return false;
    if (!valid || !std::isfinite(x) || !std::isfinite(y) || !std::isfinite(z)
        || std::fabs(x) >= 7.9f || std::fabs(y) >= 7.9f || std::fabs(z) >= 7.9f) {
      disarm();
      return false;
    }
    if (have_time_) {
      uint32_t dt = now - last_sample_;
      if (dt < 5 || dt > kMaxSampleGapMs) { disarm(); return false; }
    }
    have_time_ = true;
    last_sample_ = now;
    const float magnitude = std::sqrt(x * x + y * y + z * z);
    if (magnitude < 0.2f) { disarm(); return false; } // zero/fall is not a shake input
    if (state_ == State::Calibrating) {
      if (uint32_t(now - arm_at_) > kCalibrationTimeoutMs) { disarm(); return false; }
      float dx = x - anchor_x_, dy = y - anchor_y_, dz = z - anchor_z_;
      if (magnitude < 0.85f || magnitude > 1.15f
          || (samples_ && dx * dx + dy * dy + dz * dz > 0.08f * 0.08f)) samples_ = 0;
      if (!samples_) {
        if (magnitude < 0.85f || magnitude > 1.15f) return false;
        anchor_x_ = x; anchor_y_ = y; anchor_z_ = z;
        calibration_at_ = now;
        magnitude_sum_ = 0;
      }
      magnitude_sum_ += magnitude;
      ++samples_;
      if (uint32_t(now - calibration_at_) >= kCalibrationMs && samples_ >= 50) {
        gravity_ = magnitude_sum_ / samples_;
        state_ = State::Armed;
        resetPeaks();
      }
      return false;
    }
    const float dynamic_g = std::fabs(magnitude - gravity_);
    if (has_fired_ && uint32_t(now - last_fire_) < 2000) {
      needs_quiet_ = true;
      quiet_ = false;
      resetPeaks();
      return false;
    }
    if (needs_quiet_) {
      if (dynamic_g <= kReleaseG) {
        if (!quiet_) { quiet_ = true; quiet_at_ = now; }
        if (uint32_t(now - quiet_at_) >= kSettleMs) needs_quiet_ = false;
      } else quiet_ = false;
      return false;
    }
    // Two >=20ms high intervals, separated by >=40ms low and 80..600ms
    // between their rising edges. Sustained acceleration is not two peaks.
    if (first_peak_ && uint32_t(now - first_at_) > 600) resetPeaks();
    if (dynamic_g <= kReleaseG) {
      high_ = false;
      if (!low_) { low_ = true; low_at_ = now; }
      if (uint32_t(now - low_at_) >= 40) peak_latched_ = false;
      return false;
    }
    if (dynamic_g < kPeakG) { high_ = false; low_ = false; return false; }
    low_ = false;
    if (peak_latched_) return false;
    if (!high_) { high_ = true; high_at_ = now; return false; }
    if (uint32_t(now - high_at_) < 20) return false;
    high_ = false;
    peak_latched_ = true;
    if (!first_peak_) { first_peak_ = true; first_at_ = high_at_; return false; }
    uint32_t separation = high_at_ - first_at_;
    if (separation < 80) return false;
    last_fire_ = now;
    has_fired_ = true;
    needs_quiet_ = true;
    quiet_ = false;
    resetPeaks();
    return true;
  }
 private:
  void resetPeaks() { high_ = low_ = first_peak_ = peak_latched_ = false; }
  State state_ = State::Off;
  bool have_time_ = false, needs_quiet_ = false, quiet_ = false, has_fired_ = false;
  bool high_ = false, low_ = false, first_peak_ = false, peak_latched_ = false;
  uint32_t arm_at_ = 0, last_sample_ = 0, calibration_at_ = 0, quiet_at_ = 0;
  uint32_t high_at_ = 0, low_at_ = 0, first_at_ = 0, last_fire_ = 0;
  uint32_t samples_ = 0;
  float anchor_x_ = 0, anchor_y_ = 0, anchor_z_ = 0, magnitude_sum_ = 0, gravity_ = 1;
};
}  // namespace haptic

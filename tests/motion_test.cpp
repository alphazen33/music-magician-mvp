// SPDX-License-Identifier: MIT
#include <cassert>
#include <iostream>
#include <limits>
#include "shake_detector.h"
#include "generated_config.h"

struct Rig {
  haptic::ShakeDetector detector;
  uint32_t now = 0;
  bool feed(float x = 0, float y = 0, float z = 1, uint32_t dt = 20, bool valid = true) {
    now += dt;
    return detector.sample(x, y, z, now, valid);
  }
  void calibrate() {
    detector.arm(now);
    for (int i = 0; i < 101; ++i) assert(!feed());
    assert(detector.state() == haptic::ShakeDetector::State::Armed);
  }
  int shake() {
    int count = 0;
    for (int i = 0; i < 2; ++i) count += feed(0, 0, 2);
    for (int i = 0; i < 3; ++i) count += feed();
    for (int i = 0; i < 2; ++i) count += feed(0, 0, 2);
    return count;
  }
};

int main() {
  using State = haptic::ShakeDetector::State;
  Rig off;
  assert(off.detector.state() == State::Off);
  assert(off.shake() == 0);
  off.calibrate();
  assert(off.shake() == 1);
  off.detector.disarm();
  assert(off.shake() == 0);
  std::cout << "PASS default off / explicit arm / calibration / one shake / disarm\n";

  Rig moving;
  moving.detector.arm(0);
  for (int i = 0; i < 501; ++i) assert(!moving.feed(i % 2 ? 0.2f : -0.2f));
  assert(moving.detector.state() == State::Off);
  std::cout << "PASS calibration rejects movement and times out\n";

  Rig noise;
  noise.calibrate();
  for (int i = 0; i < 200; ++i) assert(!noise.feed(i % 2 ? 0.04f : -0.04f));
  // Slow orientation change preserves gravity magnitude and must not trigger.
  for (int i = 0; i < 100; ++i) {
    float angle = i * 0.015f;
    assert(!noise.feed(std::sin(angle), 0, std::cos(angle)));
  }
  assert(!noise.feed(0, 0, 2)); // Single sample spike.
  for (int i = 0; i < 50; ++i) assert(!noise.feed());
  for (int i = 0; i < 100; ++i) assert(!noise.feed(0, 0, 2));
  std::cout << "PASS small jitter / orientation / single spike / sustained acceleration\n";

  Rig cooldown;
  cooldown.calibrate();
  assert(cooldown.shake() == 1);
  assert(cooldown.shake() == 0);
  for (int i = 0; i < 150; ++i) assert(!cooldown.feed());
  assert(cooldown.shake() == 1);
  std::cout << "PASS detector holdoff and quiet re-arm\n";

  Rig motor;
  motor.calibrate();
  motor.detector.suppress();
  motor.now += 3000; // Explicit known playback pause is not stale sensor input.
  assert(motor.shake() == 0);
  for (int i = 0; i < 30; ++i) assert(!motor.feed());
  assert(motor.shake() == 1);
  haptic::Player player(1000);
  assert(player.start(config::kPatterns[0], motor.now) == haptic::Result::Started);
  assert(player.start(config::kPatterns[0], motor.now + 20) == haptic::Result::Busy);
  player.stop(motor.now + 40);
  motor.detector.disarm();
  assert(player.tick(motor.now + 40) == 0 && motor.shake() == 0);
  assert(player.start(config::kPatterns[0], motor.now) == haptic::Result::Cooldown);
  std::cout << "PASS motor suppression / settling / shared player busy-stop-cooldown\n";

  for (int kind = 0; kind < 6; ++kind) {
    Rig bad;
    bad.calibrate();
    if (kind == 0) assert(!bad.feed(0, 0, 1, 101));
    if (kind == 1) assert(!bad.feed(0, 0, 1, 0));
    if (kind == 2) assert(!bad.feed(std::numeric_limits<float>::quiet_NaN()));
    if (kind == 3) assert(!bad.feed(0, 0, 1, 20, false));
    if (kind == 4) assert(!bad.feed(0, 0, 8));
    if (kind == 5) assert(!bad.feed(0, 0, 0));
    assert(bad.detector.state() == State::Off && bad.shake() == 0);
  }
  Rig reversed;
  reversed.calibrate();
  assert(!reversed.detector.sample(0, 0, 2, reversed.now - 20));
  assert(reversed.detector.state() == State::Off);
  Rig wrap;
  wrap.now = UINT32_MAX - 1000;
  wrap.calibrate();
  assert(wrap.shake() == 1);
  std::cout << "PASS stale / duplicate / NaN / failed read / saturated / zero / reversed time / rollover\n";
}

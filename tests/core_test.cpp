// SPDX-License-Identifier: MIT
#include <cassert>
#include <iostream>
#include "generated_config.h"

int main() {
  using haptic::Result;
  const auto& hello = config::kPatterns[config::patternByName("hello")];
  // Boundaries and cooldown use physical scheduled end, not delayed service time.
  haptic::Player player(1000);
  assert(player.start(hello, 0) == Result::Started);
  assert(player.tick(89) == 180);
  assert(player.tick(90) == 0);
  assert(player.start(hello, 100) == Result::Busy);
  assert(player.tick(190) == 180);
  assert(player.tick(280) == 0 && !player.active());
  assert(player.start(hello, 1279) == Result::Cooldown);
  assert(player.start(hello, 1280) == Result::Started);
  player.stop(1300);
  assert(player.tick(1300) == 0 && !player.active());
  assert(player.start(hello, 2299) == Result::Cooldown);
  assert(player.start(hello, 2300) == Result::Started);
  assert(player.tick(10000) == 0); // Late ticks cannot extend a pulse.
  std::cout << "PASS pulse boundaries / busy / cooldown / stop / late tick\n";

  haptic::Player wrap(1000);
  const uint32_t near_wrap = UINT32_MAX - 49;
  assert(wrap.start(hello, near_wrap) == Result::Started);
  assert(wrap.tick(40) == 0); // 90 ms after start, across millis wrap.
  assert(wrap.tick(140) == 180);
  assert(wrap.tick(230) == 0);
  assert(wrap.start(hello, 1229) == Result::Cooldown);
  assert(wrap.start(hello, 1230) == Result::Started);
  std::cout << "PASS 32-bit millis rollover\n";

  haptic::PresenceGate gate(500);
  assert(gate.observe("12345678", 0));
  assert(!gate.observe("12345678", 2000));
  assert(!gate.observe("DEADBEEF", 2200)); // no swapping through a hold
  gate.observe(nullptr, 2300);
  gate.observe(nullptr, 2799);
  assert(!gate.observe("12345678", 2799)); // transient dropout reset
  gate.observe(nullptr, 3000);
  gate.observe(nullptr, 3500);
  assert(gate.observe("12345678", 3501));
  assert(!gate.observe("12345678", 100000)); // skipped polls are not absence
  std::cout << "PASS hold / UID switch / transient dropout / removal re-arm\n";

  haptic::Button button;
  assert(!button.update(true, 0));
  assert(!button.update(false, 10));
  assert(!button.update(true, 20));
  assert(!button.update(true, 49));
  assert(button.update(true, 50));
  assert(!button.update(true, 1000));
  assert(!button.update(false, 1100));
  assert(!button.update(false, 1130));
  assert(!button.update(true, 1200));
  assert(button.update(true, 1230));
  std::cout << "PASS button debounce / hold / second press\n";

  haptic::Player invalid(1000);
  haptic::Step too_long[] = {{501, 100}};
  haptic::Step too_strong[] = {{100, 255}};
  haptic::Step zero[] = {{0, 100}};
  haptic::Step all_off[] = {{100, 0}};
  haptic::Step too_much_on[] = {{500, 100}, {500, 100}, {500, 100}, {1, 100}};
  haptic::Pattern bad[] = {{"a", too_long, 1}, {"b", too_strong, 1},
    {"c", zero, 1}, {"d", all_off, 1}, {"e", nullptr, 0}, {"f", too_much_on, 4}};
  for (auto& p : bad) assert(invalid.start(p, 0) == Result::Invalid);
  assert(config::patternForTag("DEADBEEF") == config::patternByName("celebrate"));
  assert(config::patternForTag("11111111") == config::patternByName("hello"));
  assert(config::patternByName("nonexistent") == -1);
  std::cout << "PASS invalid pattern rejection / tag lookup\n";
}

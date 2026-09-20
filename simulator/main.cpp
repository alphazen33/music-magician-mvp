// SPDX-License-Identifier: MIT
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include "generated_config.h"
#include "shake_detector.h"

int main() {
  haptic::Player player(config::kCooldownMs);
  haptic::PresenceGate gate(config::kReleaseMs);
  haptic::ShakeDetector motion;
  uint32_t now = 0;
  std::string held, line;
  uint8_t last_amplitude = 0;
  auto output = [&]() {
    uint8_t amp = player.tick(now);
    if (amp != last_amplitude) {
      std::cout << now << "ms PWM " << int(amp);
      if (amp) std::cout << ' ' << std::string(amp / 15, '#');
      std::cout << '\n';
      last_amplitude = amp;
    }
  };
  auto play = [&](int index) {
    if (index < 0) { std::cout << now << "ms ignored\n"; return; }
    auto result = player.start(config::kPatterns[index], now);
    std::cout << now << "ms " << config::kPatterns[index].name << ' '
              << haptic::resultName(result) << '\n';
    output();
  };
  auto imu = [&](float x, float y, float z, uint32_t dt) {
    auto before = motion.state();
    for (uint32_t i = 0; i < dt; ++i) {
      ++now;
      output();
      if (player.active()) motion.suppress();
    }
    if (player.active()) motion.suppress();
    else if (motion.sample(x, y, z, now)) play(int(config::kButtonPattern));
    if (motion.state() != before) std::cout << now << "ms motion " << motion.stateName() << '\n';
  };
  std::cout << "DESKTOP SIMULATION: no NFC reader or physical vibration\n"
            << "Commands: play NAME | tag HEX | remove | advance MS | stop | list | quit\n"
            << "Motion: arm | disarm | still MS | imu AX AY AZ DELTA_MS (synthetic g samples)\n";
  while (std::getline(std::cin, line)) {
    std::istringstream in(line);
    std::string action, arg;
    in >> action >> arg;
    if (action.empty() || action[0] == '#') continue;
    if (action == "quit") break;
    if (action == "arm") {
      if (player.active()) std::cout << "stop playback before arm\n";
      else { motion.arm(now); std::cout << now << "ms motion calibrating\n"; }
    } else if (action == "disarm") {
      motion.disarm(); player.stop(now); output();
      std::cout << now << "ms motion off\n";
    } else if (action == "imu") {
      float x = 0, y = 0, z = 0;
      uint32_t dt = 0;
      std::istringstream sample_line(line.substr(4));
      if (!(sample_line >> x >> y >> z >> dt) || dt > 60000) {
        std::cout << "imu requires numeric AX AY AZ DELTA_MS (0..60000)\n";
      } else imu(x, y, z, dt);
    } else if (action == "still") {
      uint32_t duration = 0;
      std::istringstream duration_text(arg);
      if (!(duration_text >> duration) || duration > 60000 || duration % 20) {
        std::cout << "still requires a 0..60000 multiple of 20 ms\n";
      } else for (uint32_t i = 0; i < duration; i += 20) imu(0, 0, 1, 20);
    } else if (action == "list") {
      for (const auto& p : config::kPatterns) std::cout << p.name << '\n';
    } else if (action == "play") {
      play(config::patternByName(arg.c_str()));
    } else if (action == "tag") {
      bool valid = (arg.size() == 8 || arg.size() == 14)
                   && arg.find_first_not_of("0123456789ABCDEF") == std::string::npos;
      if (!valid) { std::cout << "invalid UID: uppercase 4/7-byte hex required\n"; continue; }
      held = arg;
      if (!player.active() && gate.observe(held.c_str(), now))
        play(config::patternForTag(held.c_str()));
      else std::cout << now << "ms tag held; no retrigger\n";
    } else if (action == "remove") {
      held.clear();
      if (!player.active()) gate.observe(nullptr, now);
      std::cout << now << "ms tag removed\n";
    } else if (action == "stop") {
      motion.disarm();
      player.stop(now);
      output();
    } else if (action == "advance") {
      unsigned long amount = 0;
      try {
        size_t used;
        amount = std::stoul(arg, &used);
        if (used != arg.size() || amount > 60000) throw std::invalid_argument("range");
      } catch (...) { std::cout << "advance must be 0..60000 ms\n"; continue; }
      for (unsigned long i = 0; i < amount; ++i) {
        ++now;
        output();
        if (player.active()) motion.suppress();
        if (!player.active() && now % 120 == 0) {
          if (gate.observe(held.empty() ? nullptr : held.c_str(), now))
            play(config::patternForTag(held.c_str()));
        }
      }
      std::cout << now << "ms time\n";
    } else std::cout << "unknown command\n";
  }
  player.stop(now);
  output();
}

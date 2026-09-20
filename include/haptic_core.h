// SPDX-License-Identifier: MIT
#pragma once
#include <cstddef>
#include <cstdint>
#include <cstring>

namespace haptic {
struct Step { uint16_t duration_ms; uint8_t amplitude; };
struct Pattern { const char* name; const Step* steps; size_t count; };
struct TagBinding { const char* uid; size_t pattern; };
enum class Result { Started, Busy, Cooldown, Invalid };

inline const char* resultName(Result r) {
  switch (r) {
    case Result::Started: return "started";
    case Result::Busy: return "busy";
    case Result::Cooldown: return "cooldown";
    default: return "invalid";
  }
}

// Hardware-independent scheduler. Late ticks skip expired steps instead of
// stretching a pulse. Unsigned subtraction also handles millis() rollover.
class Player {
 public:
  explicit Player(uint32_t cooldown_ms) : cooldown_ms_(cooldown_ms) {}
  Result start(const Pattern& p, uint32_t now) {
    tick(now);
    if (active_) return Result::Busy;
    if (ever_played_ && uint32_t(now - ended_at_) < cooldown_ms_)
      return Result::Cooldown;
    uint32_t total = 0, on = 0;
    if (!p.steps || p.count == 0 || p.count > 32) return Result::Invalid;
    for (size_t i = 0; i < p.count; ++i) {
      const Step& s = p.steps[i];
      if (!s.duration_ms || s.duration_ms > 500 || s.amplitude > 230)
        return Result::Invalid;
      total += s.duration_ms;
      if (s.amplitude) on += s.duration_ms;
    }
    if (total > 3000 || on == 0 || on > 1500) return Result::Invalid;
    pattern_ = &p;
    duration_ = total;
    started_at_ = now;
    active_ = ever_played_ = true;
    tick(now);
    return Result::Started;
  }
  uint8_t tick(uint32_t now) {
    if (!active_) return 0;
    uint32_t elapsed = now - started_at_;
    if (elapsed >= duration_) {
      ended_at_ = started_at_ + duration_;
      active_ = false;
      amplitude_ = 0;
      return 0;
    }
    uint32_t boundary = 0;
    for (size_t i = 0; i < pattern_->count; ++i) {
      boundary += pattern_->steps[i].duration_ms;
      if (elapsed < boundary) return amplitude_ = pattern_->steps[i].amplitude;
    }
    return 0;
  }
  void stop(uint32_t now) {
    if (active_) ended_at_ = now;
    active_ = false;
    amplitude_ = 0;
  }
  bool active() const { return active_; }
  uint8_t amplitude() const { return amplitude_; }
 private:
  const Pattern* pattern_ = nullptr;
  uint32_t cooldown_ms_, started_at_ = 0, ended_at_ = 0, duration_ = 0;
  bool active_ = false, ever_played_ = false;
  uint8_t amplitude_ = 0;
};

// One trigger per presentation, including switching directly to another UID.
// Re-arm only after continuously observed absence; skipped polls aren't absence.
class PresenceGate {
 public:
  explicit PresenceGate(uint32_t release_ms) : release_ms_(release_ms) {}
  bool observe(const char* uid, uint32_t now) {
    if (uid && *uid) {
      missing_ = false;
      if (latched_) return false;
      latched_ = true;
      return true;
    }
    if (!latched_) return false;
    if (!missing_) { missing_ = true; missing_since_ = now; }
    if (uint32_t(now - missing_since_) >= release_ms_) latched_ = false;
    return false;
  }
 private:
  uint32_t release_ms_, missing_since_ = 0;
  bool latched_ = false, missing_ = false;
};

class Button {
 public:
  bool update(bool pressed, uint32_t now) {
    if (pressed != raw_) { raw_ = pressed; changed_at_ = now; }
    if (raw_ != stable_ && uint32_t(now - changed_at_) >= 30) {
      stable_ = raw_;
      return stable_;
    }
    return false;
  }
 private:
  uint32_t changed_at_ = 0;
  bool raw_ = false, stable_ = false;
};
}  // namespace haptic

// SPDX-License-Identifier: MIT
#include <Arduino.h>
#if HAPTIC_NFC
  #include <SPI.h>
  #include <Adafruit_PN532.h>
#endif
#include "generated_config.h"
#include "board_pins.h"
#if HAPTIC_IMU
  #include "motion_sensor.h"
MotionSensor motion_sensor;
haptic::ShakeDetector shake;
uint32_t last_imu_poll = 0, last_imu_good = 0;
#endif

#if HAPTIC_NFC
constexpr uint32_t kPollMs = 120;
Adafruit_PN532 nfc(kCs, &SPI);
#endif
haptic::Player player(config::kCooldownMs);
haptic::PresenceGate presence(config::kReleaseMs);
haptic::Button button;
bool nfc_ready = false, learn_next = false, overflow = false;
uint32_t last_poll = 0;
char command[64];
size_t command_size = 0;

void play(int index, const char* source) {
  if (index < 0 || size_t(index) >= config::kPatternCount) {
    Serial.println("ignored: unknown pattern/tag");
    return;
  }
  auto result = player.start(config::kPatterns[index], millis());
  ledcWrite(kPwmChannel, player.amplitude());
  Serial.printf("%s %s %s\n", source, config::kPatterns[index].name,
                haptic::resultName(result));
}

void executeCommand() {
  command[command_size] = 0;
  if (!strcmp(command, "stop")) {
    player.stop(millis());
    ledcWrite(kPwmChannel, 0);
#if HAPTIC_IMU
    shake.disarm();
#endif
    Serial.println("stopped");
  } else if (!strcmp(command, "arm")) {
#if HAPTIC_IMU
    if (player.active()) {
      Serial.println("arm rejected: stop playback first");
    } else if (!motion_sensor.begin()) {
      shake.disarm();
      Serial.println("IMU unavailable; motion remains off");
    } else {
      shake.arm(millis());
      last_imu_poll = last_imu_good = millis();
      Serial.println("motion calibrating: leave the device still for 2 seconds; timeout 10 seconds");
    }
#else
    Serial.println("IMU not enabled in this board profile");
#endif
  } else if (!strcmp(command, "disarm")) {
#if HAPTIC_IMU
    shake.disarm();
#endif
    player.stop(millis());
    ledcWrite(kPwmChannel, 0);
    Serial.println("motion off; stopped");
  } else if (!strncmp(command, "play ", 5)) {
    play(config::patternByName(command + 5), "serial");
  } else if (!strcmp(command, "learn")) {
#if HAPTIC_NFC
    learn_next = true;
    Serial.println("next observed tag UID will print once; no tag data is written");
#else
    Serial.println("NFC is not enabled in this board profile");
#endif
  } else if (!strcmp(command, "list")) {
    for (const auto& p : config::kPatterns) Serial.println(p.name);
  } else {
    Serial.println("commands: list | play NAME | stop | learn | arm | disarm");
  }
}

void serviceSerial() {
  // Bound work per tick even when a host floods the UART.
  for (uint8_t budget = 0; budget < 32 && Serial.available(); ++budget) {
    char c = char(Serial.read());
    if (c == '\r') continue;
    if (c == '\n') {
      if (overflow) Serial.println("command rejected: too long");
      else if (command_size) executeCommand();
      command_size = 0;
      overflow = false;
    } else if (!overflow && command_size < sizeof(command) - 1) {
      command[command_size++] = c;
    } else {
      overflow = true;
    }
  }
}

void setup() {
  // External 100k gate pulldown also keeps the motor off before firmware starts.
  pinMode(kMotor, OUTPUT);
  digitalWrite(kMotor, LOW);
  ledcSetup(kPwmChannel, kPwmFrequency, 8);
  ledcAttachPin(kMotor, kPwmChannel);
  ledcWrite(kPwmChannel, 0);
  pinMode(kButton, INPUT_PULLUP);
  Serial.begin(115200);
#if HAPTIC_NFC
  SPI.begin(kSck, kMiso, kMosi, kCs);
  nfc_ready = nfc.begin() && nfc.getFirmwareVersion() && nfc.SAMConfig()
              && nfc.setPassiveActivationRetries(0x00);
#endif
  Serial.println("NFC Haptic MVP; no Wi-Fi, no cloud");
#if HAPTIC_NFC
  Serial.println(nfc_ready ? "NFC ready" : "NFC unavailable: button and serial remain usable; reconnect then reset");
#else
  Serial.printf("AtomS3R-CAM: G%d motor-driver signal; G%d external button; NFC disabled\n", kMotor, kButton);
#endif
  Serial.println("type list, play hello, stop, learn, arm, disarm; newline required; motion OFF at boot");
}

void loop() {
  ledcWrite(kPwmChannel, player.tick(millis()));
  serviceSerial();
  if (button.update(digitalRead(kButton) == LOW, millis()))
    play(int(config::kButtonPattern), "button");

#if HAPTIC_IMU
  auto before = shake.state();
  if (player.active()) {
    shake.suppress(); // Never read I2C while generating motor pulses.
    last_imu_good = millis();
  } else if (shake.state() != haptic::ShakeDetector::State::Off
             && uint32_t(millis() - last_imu_poll) >= 20) {
    last_imu_poll = millis();
    float x = 0, y = 0, z = 0;
    if (motion_sensor.read(x, y, z)) {
      last_imu_good = millis();
      if (shake.sample(x, y, z, last_imu_good)) play(int(config::kButtonPattern), "imu-shake");
    } else if (uint32_t(millis() - last_imu_good) >= haptic::ShakeDetector::kMaxSampleGapMs) {
      shake.sample(0, 0, 0, millis(), false);
    }
  }
  if (shake.state() != before) Serial.printf("motion %s\n", shake.stateName());
#endif

  // The vendor NFC call has bounded waits but is not async. Never enter it
  // while a pattern (including its silence gaps) is active, so pulse timing
  // does not depend on NFC response latency. No sleep-based vibration loops.
#if HAPTIC_NFC
  if (!player.active() && nfc_ready && uint32_t(millis() - last_poll) >= kPollMs) {
    last_poll = millis();
    // Vendor parser copies UID length from its 64-byte frame. Allocate 255
    // bytes defensively, then accept only supported ISO14443A UID lengths.
    uint8_t uid[255] = {}, uid_length = 0;
    bool found = nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, uid, &uid_length, 30);
    uint32_t now = millis();
    if (found && (uid_length == 4 || uid_length == 7)) {
      char hex[15] = {};
      for (uint8_t i = 0; i < uid_length; ++i)
        snprintf(hex + i * 2, 3, "%02X", uid[i]);
      if (learn_next) {
        Serial.printf("UID %s (local serial only)\n", hex);
        learn_next = false;
      }
      if (presence.observe(hex, now)) play(config::patternForTag(hex), "nfc");
    } else {
      presence.observe(nullptr, now);
    }
  }
#endif
  delay(1); // Yield to ESP32 RTOS; does not encode pulse durations.
}

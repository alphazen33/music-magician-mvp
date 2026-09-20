# Source and dependency notice

Application/core/simulator/config-generator code in this repository was authored
for this generic MVP. Third-party source is downloaded by PlatformIO and is not
vendored in the repository. Retain upstream licenses when redistributing binaries
or bundled source; the application MIT license does not relicense dependencies.

| Dependency | Pinned version | License / primary source |
| --- | --- | --- |
| PlatformIO Core | 6.1.18 | [Apache-2.0](https://github.com/platformio/platformio-core/tree/v6.1.18) |
| Espressif32 PlatformIO platform | 6.5.0 | [Apache-2.0; package versions](https://github.com/platformio/platform-espressif32/blob/v6.5.0/platform.json) |
| Arduino-ESP32 core | 2.0.14 via platform | [LGPL-2.1; component-specific notices also apply](https://github.com/espressif/arduino-esp32/tree/2.0.14) |
| Adafruit PN532 | 1.3.4 | [BSD](https://github.com/adafruit/Adafruit-PN532/tree/1.3.4) |
| Adafruit BusIO | 1.17.0 | [MIT](https://github.com/adafruit/Adafruit_BusIO/tree/1.17.0) |
| M5Unified (CAM profiles) | 0.2.7 | [MIT; official IMU initialization and conversion](https://github.com/m5stack/M5Unified/tree/0.2.7) |
| M5GFX (M5Unified dependency) | 0.2.29 | [MIT; bundled components retain their notices](https://github.com/m5stack/M5GFX/tree/0.2.29) |

The S3-CAM build uses Arduino GPIO/PWM/USB serial and, only after `arm`, the
official M5Unified IMU utility on the internal I2C bus. It does not call
`M5.begin()`, initialize a camera/display/audio endpoint, or connect a cloud assistant.
M5Stack's
[official CAM kit PlatformIO example](https://docs.m5stack.com/en/core/AtomS3R-CAM%20AI%20Chatbot)
uses the generic `esp32-s3-devkitc-1` target with `qio_opi`; our board profile follows
that memory setup and explicitly maps the external GPIO. This is a build target,
not a claim that the physical CAM board is a DevKitC.

[Arduino-ESP32 2.0.14 LEDC declarations](https://github.com/espressif/arduino-esp32/blob/2.0.14/cores/esp32/esp32-hal-ledc.h)
were checked for `ledcSetup`, `ledcAttachPin`, and `ledcWrite`. Those APIs differ in
Arduino-ESP32 3.x; upgrade deliberately and repeat compilation/hardware checks.
[PN532 source](https://github.com/adafruit/Adafruit-PN532/blob/1.3.4/Adafruit_PN532.cpp)
was checked for timeout/retry behavior. The read call is bounded-blocking; it runs
only while the haptic player is inactive. This is not a fully asynchronous NFC driver.

The generic functionality has public prior art. No originality/patentability claim,
patent filing, third-party patent license, or freedom-to-operate opinion is provided.
Product and company names identify compatibility or sources, not sponsorship.

# Verification status

Updated 2026-09-21 (Asia/Shanghai), including the optional BMI270 shake input. This file reports observed checks, not a release certification.

| Check | Result | Evidence |
| --- | --- | --- |
| Native C++ scheduler/presence/button tests | PASS, 5 behavior groups, ASan + UBSan enabled | `test-desktop.log` |
| Native C++ motion tests | PASS, 6 behavior groups, ASan + UBSan enabled | `test-desktop.log` |
| Python configuration checks | PASS, 6 tests with multiple invalid-input cases | `test-desktop.log` |
| Generated config matches JSON | PASS | `test-desktop.log` |
| Same-core desktop scenario | PASS: pulses, held-tag suppression, removal, stop/cooldown | `simulator-demo.log` |
| Synthetic IMU sample scenario | PASS: arm, calibration at 2020 ms, shake-triggered hello at 2160 ms, disarm | `simulator-motion-demo.log` |
| `git diff --cached --check` | PASS | Local staging check |
| ESP32-WROOM + PN532 firmware build | PASS; RAM 22,160 B / program flash 301,237 B | `build-firmware.log` |
| AtomS3R-CAM with optional IMU firmware build | PASS; RAM 22,652 B / program flash 485,905 B | `build-firmware.log` |
| AtomS3R-CAM + U059 with optional IMU firmware build | PASS; RAM 22,652 B / program flash 485,913 B | `build-firmware.log` |
| GitHub Actions | NOT RUN; current publishing OAuth authorization lacks workflow scope | Workflow saved as `ci/check.yml.template` |
| USB development board enumerated | None observed | macOS serial listing contained only Bluetooth/system devices |
| Firmware upload / PN532 exchange / real motor response | NOT RUN | No confirmed connected board/peripherals |
| BMI270 I2C, stationary calibration, physical shake accuracy and motor self-trigger rejection | NOT RUN | No connected CAM; tests use synthetic acceleration |

Reproduce desktop checks with `bash scripts/build_desktop.sh`, then
`./build/simulator < simulator/demo.txt` and
`./build/simulator < simulator/motion-demo.txt`. Reproduce firmware checks with
`.venv/bin/pio run -e m5atoms3r_cam -e m5atoms3r_cam_u059 -e esp32dev`.

Actual local firmware invocation reused a copied, installed toolchain cache:

```sh
PLATFORMIO_CORE_DIR="$PWD/build/pio-core" PLATFORMIO_BUILD_DIR="$PWD/build/firmware" .venv/bin/pio run -e m5atoms3r_cam -e m5atoms3r_cam_u059 -e esp32dev
```

All three used Espressif32 6.5.0, Arduino-ESP32 2.0.14 (package
3.20014.231204), Xtensa 8.4.0+2021r2-patch5. CAM profiles also used M5Unified
0.2.7 and M5GFX 0.2.29. The final invocation completed in 22.929 s with some
objects already cached. This duration excludes download/setup time and is not
a clean-build performance benchmark.
The default build command above obtains the same versioned dependencies online;
the private `build/` cache is ignored by Git and is not a repository dependency.

The vendor Arduino core emitted an existing `uartSetPins` return-without-value
warning in the rebuilt CAM environments. No application compile errors remained
in the recorded final build. Log trailing spaces were normalized for Git.
The original 2.0.17 setup attempt was superseded after a network download retry;
the completed builds and pinned manifest use 2.0.14, not that unfinished download.
Image sizes/hashes are in `firmware-artifacts.json`; generated binaries remain
in the local ignored build folder and are not published as tested hardware releases.

The IMU feature is a gated threshold double-pulse shake input, not snap detection,
facial/head gesture recognition, or a world model. Do not infer physical sensor
performance, camera, microphone, audio output, Bluetooth, or music generation
from successful compilation and synthetic tests.

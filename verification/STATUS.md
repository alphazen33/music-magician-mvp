# Verification status

Recorded 2026-09-20 (Asia/Shanghai). This file reports observed checks, not a release certification.

| Check | Result | Evidence |
| --- | --- | --- |
| Native C++ scheduler/presence/button tests | PASS, 5 behavior groups, ASan + UBSan enabled | `test-desktop.log` |
| Python configuration checks | PASS, 6 tests with multiple invalid-input cases | `test-desktop.log` |
| Generated config matches JSON | PASS | `test-desktop.log` |
| Same-core desktop scenario | PASS: pulses, held-tag suppression, removal, stop/cooldown | `simulator-demo.log` |
| `git diff --cached --check` | PASS | Local staging check |
| ESP32-WROOM + PN532 firmware build | PASS; RAM 22,160 B / program flash 301,093 B | `build-firmware.log` |
| AtomS3R-CAM firmware build | PASS; RAM 19,224 B / program flash 271,549 B | `build-firmware.log` |
| AtomS3R-CAM + U059 firmware build | PASS; RAM 19,224 B / program flash 271,545 B | `build-firmware.log` |
| GitHub Actions | NOT RUN; current publishing OAuth authorization lacks workflow scope | Workflow saved as `ci/check.yml.template` |
| USB development board enumerated | None observed | macOS serial listing contained only Bluetooth/system devices |
| Firmware upload / PN532 exchange / real motor response | NOT RUN | No confirmed connected board/peripherals |

Reproduce desktop checks with `bash scripts/build_desktop.sh`, then
`./build/simulator < simulator/demo.txt`. Reproduce firmware checks with
`.venv/bin/pio run -e m5atoms3r_cam -e m5atoms3r_cam_u059 -e esp32dev`.

Actual local firmware invocation reused a copied, installed toolchain cache:

```sh
PLATFORMIO_CORE_DIR="$PWD/build/pio-core" PLATFORMIO_BUILD_DIR="$PWD/build/firmware" .venv/bin/pio run -e m5atoms3r_cam -e m5atoms3r_cam_u059 -e esp32dev
```

All three used Espressif32 6.5.0, Arduino-ESP32 2.0.14 (package
3.20014.231204), Xtensa 8.4.0+2021r2-patch5. They completed in 48.403 s total
after dependencies were present. This duration excludes download/setup time.
The default build command above obtains the same versioned dependencies online;
the private `build/` cache is ignored by Git and is not a repository dependency.

The vendor Arduino core emitted an existing `uartSetPins` return-without-value
warning once per environment. No application compile errors were reported.
The original 2.0.17 setup attempt was superseded after a network download retry;
the completed builds and pinned manifest use 2.0.14, not that unfinished download.
Image sizes/hashes are in `firmware-artifacts.json`; generated binaries remain
in the local ignored build folder and are not published as tested hardware releases.

Do not infer camera, IMU, microphone, audio output, Bluetooth, music generation,
or embedded gesture sensing from these checks; those are outside this repository.

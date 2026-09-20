# Verification status

Recorded 2026-09-20 (Asia/Shanghai). This file reports observed checks, not a release certification.

| Check | Result | Evidence |
| --- | --- | --- |
| Native C++ scheduler/presence/button tests | PASS, 5 behavior groups, ASan + UBSan enabled | `test-desktop.log` |
| Python configuration checks | PASS, 6 tests with multiple invalid-input cases | `test-desktop.log` |
| Generated config matches JSON | PASS | `test-desktop.log` |
| Same-core desktop scenario | PASS: pulses, held-tag suppression, removal, stop/cooldown | `simulator-demo.log` |
| `git diff --cached --check` | PASS | Local staging check |
| ESP32-WROOM firmware build | In progress: first toolchain download | `build-esp32dev.log` |
| AtomS3R-CAM firmware build | Pending | Will follow shared framework installation |
| USB development board enumerated | None observed | macOS serial listing contained only Bluetooth/system devices |
| Firmware upload / PN532 exchange / real motor response | NOT RUN | No confirmed connected board/peripherals |

Reproduce desktop checks with `bash scripts/build_desktop.sh`, then
`./build/simulator < simulator/demo.txt`. Reproduce firmware checks with
`.venv/bin/pio run -e m5atoms3r_cam -e esp32dev`.

Do not infer camera, IMU, microphone, audio output, Bluetooth, music generation,
or embedded gesture sensing from these checks; those are outside this repository.

# Security and operating scope

- This is a local development example, not an access-control, medical, or safety device.
- NFC UID values can be copied and can change. They select a pattern; they do not prove a person's identity or authenticate another device.
- NFC absence is inferred from unsuccessful reads. RF interference or disconnection can therefore re-arm the presentation gate after the absence threshold. This requires real hardware testing; it is not a robust proximity proof.
- The PN532 upstream frame parser is not treated as a security boundary. Use your own development tags and a trusted module. The application accepts only 4/7-byte UIDs and does not parse tag content.
- No Wi-Fi, Bluetooth, audio, image, contacts, or location acquisition is enabled. No credentials are needed. `learn` prints a tag UID once on the local serial port when explicitly requested.
- Do not commit real personal UID mappings or credentials. Keep your private working configuration outside a public fork or carefully review it before committing.
- PWM limits reduce ordinary accidental overdrive but do not replace electrical/current limits, appropriate motor supply, or a hardware fail-safe. A frozen processor may hold the last PWM value until reset; this prototype has no independent hardware timeout circuit.
- Before flashing a previously configured board, preserve its recovery path. Uploading replaces the current application.

Report issues using a minimal reproduction without personal data. Do not post credentials or real contact information in GitHub issues.

# Changelog

## v1.12.0-rc.1

- Added hardware-validated left/right endpoint reversal for `SWEEP`, `RANDOM`, and `FLUSH`.
- Added stuck-limit, both-limits, and end-to-end travel timeout protection.
- Added timed `CENTERING`: establish both endpoints, measure full travel, return halfway, and stop.
- Permitted safe startup and motor START from one active endpoint while retaining ARM/FIRE inhibition.
- Verified active-LOW YL-99 modules using their onboard pull-ups and corrected the Deutsch harness polarity wiring.
- Updated the embedded firmware version to `Crosswind ESP32 Phase 1 v1.12.0-rc.1`.

- Alpha hardware revision: promoted the verified Hosyond 2.42-inch SSD1309 OLED to the official display and replaced the former four-conductor OLED lid connection with a five-conductor, five-position JST harness carrying GND, 3.3 V, SDA, SCL, and GPIO5 RESET.
- Refactored ESP32 firmware into PlatformIO-style modules.
- Added Arduino Uno/Nano fallback firmware folder.
- Added Phase 1 mechanical, electrical, testing, branding, and planning documentation.
- Established `SWEEP` as the Phase 1 default mode.

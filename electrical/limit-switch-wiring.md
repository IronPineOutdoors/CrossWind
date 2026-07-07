# Limit Switch Wiring

Crosswind Alpha currently uses YL-99 limit switch modules on GPIO34 and GPIO35.

For ESP32 Phase 1:

- Left limit module signal output goes to GPIO34.
- Right limit module signal output goes to GPIO35.
- Module `VCC` goes to ESP32 `3V3`.
- Module `GND` goes to common ground.
- GPIO34/GPIO35 are input-only pins and need external pullup resistors.
- Firmware uses `LIMIT_ACTIVE_STATE = LOW` for the current YL-99 behavior: triggered reads LOW/active.
- Limit switches are safety/calibration inputs only. They are not normal travel controls and must not be used as physical hard stops.

Check each switch in Serial diagnostics before connecting motor power.

# Limit Switch Wiring

Crosswind Alpha currently uses YL-99 limit switch modules on GPIO27 and GPIO5.

For ESP32 Phase 1:

- Left limit module signal output goes to GPIO27.
- Right limit module signal output goes to GPIO5.
- Module `VCC` goes to ESP32 `3V3`.
- Module `GND` goes to common ground.
- Firmware uses ESP32 `INPUT_PULLUP` and `LIMIT_ACTIVE_STATE = 0` for the current YL-99 behavior: triggered reads LOW/active.
- Do not hold the GPIO5/right-limit switch active while powering or resetting the ESP32.

Check each switch in Serial diagnostics before connecting motor power.

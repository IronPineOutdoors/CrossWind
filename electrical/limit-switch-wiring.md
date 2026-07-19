# Limit Switch Wiring

Crosswind Alpha currently uses YL-99 limit switch modules on GPIO34 and GPIO35.

For ESP32 Phase 1:

- Left limit module signal output goes to GPIO34.
- Right limit module signal output goes to GPIO35.
- Module `VCC` goes to ESP32 `3V3`.
- Module `GND` goes to common ground.
- GPIO34/GPIO35 are input-only pins and need external pullup resistors.
- Firmware uses `LIMIT_ACTIVE_STATE = LOW` for the current YL-99 behavior: triggered reads LOW/active.
- Limit switches are normal firmware travel endpoints and safety/calibration inputs. They must not carry mechanism loads or replace physical hard stops.

Check each switch in Serial diagnostics before connecting motor power.

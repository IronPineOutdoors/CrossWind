# Limit Switch Wiring

Use normally closed roller switches where possible. This gives a safer default because a disconnected or opened switch can be treated like an active limit.

For ESP32 Phase 1:

- One side of the switch goes to the GPIO input.
- The other side goes to ground.
- Firmware uses `INPUT_PULLUP`.
- `LIMIT_ACTIVE_STATE` is configurable in `config.h`.

Check each switch in Serial diagnostics before connecting motor power.

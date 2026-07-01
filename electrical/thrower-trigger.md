# Thrower Trigger Wiring

The VEVOR NH113 factory foot pedal appears to be a simple two-wire momentary switch using black and white wires.

- Pedal released: open circuit.
- Pedal pressed: black and white shorted together.

Confirm this with a continuity meter before wiring Crosswind to the thrower.

## Crosswind Relay Concept

Crosswind triggers the thrower by closing a relay contact in parallel with the factory pedal.

- Relay `COM` connects to one pedal wire.
- Relay `NO` connects to the other pedal wire.
- Factory pedal remains connected and usable.
- ESP32 GPIO drives only the relay module input.
- ESP32 never sends voltage into the pedal circuit.
- Relay contacts are used as a dry contact only.

## ESP32 Control

Default firmware pin:

- `GPIO14` -> opto-isolated relay module input

Configurable settings live in `firmware/crosswind-esp32/src/config.h`:

- `THROWER_TRIGGER_PIN`
- `TRIGGER_PULSE_MS`
- `MIN_TRIGGER_INTERVAL_MS`
- `TRIGGER_ACTIVE_STATE`
- `ENABLE_THROWER_TRIGGER`
- `ALLOW_TRIGGER_WHEN_STOPPED`

Current Alpha firmware also requires the ARM button to put the controller in `ARMED` state before the FIRE / TEST button or BLE trigger commands can pulse the relay.

## Safe Test Procedure

1. Disconnect thrower power.
2. Confirm pedal black/white wires with a continuity meter.
3. Wire relay `COM` and `NO` in parallel with the pedal.
4. Confirm the factory pedal still works.
5. Power ESP32 and verify Serial diagnostics show trigger inactive.
6. Press FIRE while `SAFE` and confirm the relay is blocked.
7. Press ARM, then test the relay with the thrower unloaded and pointed safely.
8. Confirm repeated trigger commands are blocked by `MIN_TRIGGER_INTERVAL_MS`.

Do not use thrower triggering during Phase 1 movement testing unless the thrower is secured, unloaded, and pointed in a safe direction.

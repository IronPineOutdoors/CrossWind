# Crosswind ESP32 Firmware

This folder contains the Phase 1 ESP32 controller firmware in a PlatformIO-style project.

The code is split into beginner-readable modules:

- `config.h` - pin assignments, constants, shared enums, and the main controller state.
- `motor.*` - BTS7960 / IBT-2 motor control, safe reversal, soft-start ramping, and stop behavior.
- `limits.*` - left/right roller switch reads and debounce.
- `inputs.*` - start/stop button, mode button, and speed potentiometer reads.
- `modes.*` - SWEEP state machine plus future hooks for RANDOM, FLUSH, and CENTERING.
- `storage.*` - Preferences-backed mode, last fault, and last speed storage.
- `ble_control.*` - optional BLE command interface.
- `environment.*` - DHT11 Alpha sensor support with BME280 upgrade hooks.
- `display.*` - SSD1306 OLED status display.
- `diagnostics.*` - Serial startup diagnostics and runtime status payloads.

## Phase 1 Behavior

Default mode is `SWEEP`.

1. Move right until the right limit switch opens/triggers.
2. Stop.
3. Dwell for `LIMIT_DWELL_MS`.
4. Move left until the left limit switch opens/triggers.
5. Stop.
6. Dwell.
7. Repeat.

The controller works fully from the physical buttons and speed potentiometer without BLE connected.

## Build

Install PlatformIO, then run:

```sh
pio run
```

Upload with:

```sh
pio run --target upload
```

## BLE Commands

- `START`
- `STOP`
- `MODE=SWEEP`
- `MODE=RANDOM`
- `MODE=FLUSH`
- `MODE=CENTERING`
- `SPEED=0-255`
- `STATUS`
- `CLEAR_FAULT`
- `TRIGGER`
- `FIRE`
- `LAUNCH`

Trigger commands pulse the optional thrower relay. They are ignored while faulted, and ignored while stopped unless `ALLOW_TRIGGER_WHEN_STOPPED` is enabled in `config.h`.

## Environmental Sensor

Alpha firmware reads a DHT11 on GPIO26 no faster than once every 2 seconds, stores the last valid reading, and reports `ENV ERR` if a read fails. `HOT` appears at `TEMP_WARNING_F`; `TEMP FAULT` stops the motor when `ENABLE_TEMP_FAULTS` is true and temperature reaches `TEMP_FAULT_F`.

Beta should swap this module to a BME280 on the existing OLED I2C bus, GPIO21/GPIO22, using address `0x76` or `0x77`.

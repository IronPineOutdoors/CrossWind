# Crosswind ESP32 Firmware

This folder contains the Phase 1 ESP32 controller firmware in a PlatformIO project.

Current PlatformIO settings:

- Board: `esp32dev`
- Framework: Arduino
- Upload port: `COM3`
- Monitor port: `COM3`
- Monitor speed: `115200`

The code is split into beginner-readable modules:

- `config.h` - pin assignments, constants, shared enums, and the main controller state.
- `motor.*` - BTS7960 / IBT-2 motor control, safe reversal, soft-start ramping, and stop behavior.
- `limits.*` - left/right roller switch reads and debounce.
- `inputs.*` - ARM/FIRE buttons, rotary encoder speed/menu input, and optional speed potentiometer reads.
- `modes.*` - Phase 1 sweep motor command plus future hooks for RANDOM, FLUSH, and CENTERING.
- `storage.*` - Preferences-backed mode, last fault, and last speed storage.
- `ble_control.*` - optional BLE command interface.
- `environment.*` - DHT11 Alpha sensor support and optional BME280 I2C support.
- `display.*` - SSD1306 OLED status display.
- `status_led.*` - non-blocking DIYables RGB status LED control.
- `trigger.*` - non-blocking thrower relay pulse control.
- `diagnostics.*` - Serial startup diagnostics and runtime status payloads.

## Phase 1 Behavior

Default mode is `SWEEP`. In Phase 1 the crank linkage creates the physical oscillation, so firmware runs the sweep motor continuously in the selected direction while the controller is running.

The Alpha limit switches are safety/calibration inputs, not normal travel controls. During powered motor operation, either active YL-99 limit switch immediately stops the BTS7960 output, disarms the system, blocks the FIRE relay, and latches `FAULT: LIMIT`. If both limits are active together, firmware latches `FAULT: BOTH LIMITS`. The fault can only be cleared after both limit switches are released.

The controller also refuses START, ARM, and FIRE requests while a limit switch is already active, even before a run has begun. If a limit becomes active while armed, the controller automatically returns to SAFE. Stored settings are sanity-checked on boot, and BLE speed commands must be numeric values from `0` to `255`.

The Alpha bench controller uses the rotary encoder for speed, encoder press for the `MAIN`/`SETUP` display menu, a dedicated ARM button on GPIO16, and a dedicated FIRE / TEST button on GPIO17. The relay can only fire when `systemArmed` is true and no fault is active. The encoder button never triggers the relay.

On boot the system always starts `SAFE` / unarmed and the relay is initialized off. Pressing ARM toggles `ARM ON` / `ARM OFF` in Serial and updates the OLED. Pressing FIRE while safe prints `FIRE BLOCKED - NOT ARMED`; pressing FIRE while armed pulses the relay using the existing non-blocking trigger timing.

The OLED home screen shows the current motor speed percentage, `SAFE`, `ARMED`, `FAULT`, `WARNING`, or `FIRING`, relay `ON`/`OFF`, and `Limit: OK`, `ACTIVE`, `FAULT: LIMIT`, or `FAULT: BOTH`. The RGB status LED mirrors the same safety state with green ready, blue armed, red fault, yellow warning/hot, and a white/purple firing flash.

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

Trigger commands and the FIRE / TEST button pulse the thrower relay only when the system is armed. They are ignored while faulted. The relay pulse remains non-blocking and uses `TRIGGER_PULSE_MS`.

Limit faults can be cleared from BLE with `CLEAR_FAULT`, or locally with the ARM/encoder button, only after both limit switches read clear.

Modes currently accepted by BLE are `SWEEP`, `RANDOM`, `FLUSH`, and `CENTERING`. In the current Phase 1 firmware these modes share the same safe sweep motor behavior; automatic triggering remains disabled unless `ENABLE_AUTOMATIC_TRIGGER` is intentionally enabled and safety-tested.

## Current Alpha Pinout

| Function | ESP32 GPIO |
| --- | ---: |
| BTS7960 RPWM | 18 |
| BTS7960 LPWM | 19 |
| BTS7960 R_EN | 23 |
| BTS7960 L_EN | 13 |
| Thrower relay input | 14 |
| ARM button | 16 |
| FIRE / TEST button | 17 |
| Rotary encoder CLK | 32 |
| Rotary encoder DT | 33 |
| Rotary encoder SW | 25 |
| Left limit | 34 |
| Right limit | 35 |
| OLED SDA | 21 |
| OLED SCL | 22 |
| DHT11 data | 26 |
| BME280 SDA | 21 |
| BME280 SCL | 22 |
| RGB LED red | 27 |
| RGB LED green | 12 |
| RGB LED blue | 4 |
| Potentiometer placeholder | 39 |

The DIYables RGB LED module is common cathode with built-in resistors: connect module `GND` to common ground, `R` to GPIO27, `G` to GPIO12, and `B` to GPIO4. GPIO12 is a boot strapping pin on many ESP32 boards; keep it for the Alpha wiring above, but if boot or upload problems appear, move green to another PWM-capable non-strapping pin such as GPIO5 and update `RGB_GREEN_PIN`.

## Environmental Sensor

Alpha firmware reads a DHT11 on GPIO26 no faster than once every 2 seconds by default, stores the last valid reading, and reports `ENV ERR` if a read fails. `HOT` appears at `TEMP_WARNING_F`; `TEMP FAULT` stops the motor when `ENABLE_TEMP_FAULTS` is true and temperature reaches `TEMP_FAULT_F`.

To use a BME280 instead, wire it to the existing OLED I2C bus on GPIO21/GPIO22 and set `ENV_SENSOR_TYPE` to `ENV_SENSOR_BME280` in `config.h`. Firmware tries address `0x76` first, then `0x77`, and adds `pressureHpa` to the Serial/BLE status payload.

## Quick Test Checklist

1. Upload firmware with PlatformIO.
2. Confirm the OLED boots and shows the home screen.
3. Confirm encoder rotation changes motor speed.
4. Confirm ARM toggles between `SAFE` and `ARMED`.
5. Confirm FIRE is blocked while `SAFE`.
6. Confirm FIRE activates the relay only while `ARMED`.
7. Confirm relay `COM`/`NO` act as dry-contact continuity only.
8. Confirm left/right limits show inactive and active correctly in Serial/OLED setup view.
9. Confirm motor PWM changes speed on the bench before connecting linkage.
10. Confirm a limit or temperature fault stops motor output and blocks triggering.

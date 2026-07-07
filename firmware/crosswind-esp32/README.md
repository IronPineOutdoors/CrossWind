# Crosswind ESP32 Firmware

This folder contains the Phase 1 ESP32 controller firmware in a PlatformIO-style project.

The code is split into beginner-readable modules:

- `config.h` - pin assignments, constants, shared enums, and the main controller state.
- `motor.*` - BTS7960 / IBT-2 motor control, safe reversal, soft-start ramping, and stop behavior.
- `limits.*` - left/right roller switch reads and debounce.
- `inputs.*` - ARM/FIRE buttons, rotary encoder speed/menu input, and optional speed potentiometer reads.
- `modes.*` - SWEEP state machine plus future hooks for RANDOM, FLUSH, and CENTERING.
- `storage.*` - Preferences-backed mode, last fault, and last speed storage.
- `ble_control.*` - optional BLE command interface.
- `environment.*` - DHT11 Alpha sensor support with BME280 upgrade hooks.
- `display.*` - SSD1306 OLED status display.
- `status_led.*` - non-blocking DIYables RGB status LED control.
- `trigger.*` - non-blocking thrower relay pulse control.
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

The Alpha bench controller uses the rotary encoder for speed, encoder press for the `MAIN`/`SETUP` display menu, a dedicated ARM button on GPIO16, and a dedicated FIRE / TEST button on GPIO17. The relay can only fire when `systemArmed` is true and no fault is active. The encoder button never triggers the relay.

On boot the system always starts `SAFE` / unarmed and the relay is initialized off. Pressing ARM toggles `ARM ON` / `ARM OFF` in Serial and updates the OLED. Pressing FIRE while safe prints `FIRE BLOCKED - NOT ARMED`; pressing FIRE while armed pulses the relay using the existing non-blocking trigger timing.

The OLED shows the current motor speed percentage, `SAFE`, `ARMED`, `FAULT`, `WARNING`, or `FIRING`, relay `ON`/`OFF`, and the active menu page. The RGB status LED mirrors the same safety state with green ready, blue armed, red fault, yellow warning/hot, and a white/purple firing flash.

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
| RGB LED red | 27 |
| RGB LED green | 12 |
| RGB LED blue | 4 |

The DIYables RGB LED module is common cathode with built-in resistors: connect module `GND` to common ground, `R` to GPIO27, `G` to GPIO12, and `B` to GPIO4. GPIO12 is a boot strapping pin on many ESP32 boards; keep it for the Alpha wiring above, but if boot or upload problems appear, move green to another PWM-capable non-strapping pin such as GPIO5 and update `RGB_GREEN_PIN`.

## Environmental Sensor

Alpha firmware reads a DHT11 on GPIO26 no faster than once every 2 seconds, stores the last valid reading, and reports `ENV ERR` if a read fails. `HOT` appears at `TEMP_WARNING_F`; `TEMP FAULT` stops the motor when `ENABLE_TEMP_FAULTS` is true and temperature reaches `TEMP_FAULT_F`.

Beta should swap this module to a BME280 on the existing OLED I2C bus, GPIO21/GPIO22, using address `0x76` or `0x77`.

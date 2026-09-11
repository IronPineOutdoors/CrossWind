# Crosswind ESP32 Firmware

Current release candidate: `v1.12.0-rc.1`. The Alpha bounded-motion behavior has been compiled and bench-validated with the installed motor and both YL-99 endpoints.

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
- `modes.*` - bounded single-axis sweep/reversal and timed centering, with future hooks for the second axis.
- `storage.*` - Preferences-backed mode, last fault, and last speed storage.
- `ble_control.*` - optional BLE command interface.
- `environment.*` - BME280 temperature, humidity, and pressure support.
- `display.*` - U8g2-driven SSD1309 128x64 OLED status display with I2C diagnostics and recovery.
- `status_led.*` - non-blocking DIYables RGB status LED control.
- `trigger.*` - non-blocking thrower relay pulse control.
- `diagnostics.*` - Serial startup diagnostics and runtime status payloads.

## Phase 1 Behavior

Default mode is `SWEEP`. The motor travels between the left and right limit switches, stops at each end, observes direction-change dead time, and reverses. `RANDOM` and `FLUSH` use the same bounded single-axis movement until the second axis and mode-specific motion are added. `CENTERING` establishes both endpoints, measures end-to-end travel time, returns for half that time, and stops.

`MOTOR_DIRECTION_INVERTED = true` corrects the installed motor's reported reverse
travel: logical RIGHT now drives LPWM, and logical LEFT drives RPWM. Direction
names mean plate travel toward the corresponding physical limit, viewed from one
consistent operator position, not shaft rotation viewed from underneath. Limit
wiring remains Left/Green/GPIO34 and Right/Blue/GPIO35. Startup logs the inversion
setting; output diagnostics continue to name the actual RPWM/LPWM terminals.
Do not also swap motor leads when applying this correction, which would reverse
it again. After flashing, first verify direction with unloaded motion away from
endpoints and accessible power cutoff, then verify each actual endpoint causes
reversal away and releases normally. Physical verification remains required.

The Alpha limit switches are normal travel boundaries as well as safety inputs. A single active limit commands movement away from that end; the switch must release within `LIMIT_DWELL_MS`. Both limits active together, failure to release, or failure to reach an endpoint within `MAX_TRAVEL_TIME_MS` latches a fault and stops the motor. A single active limit at startup is valid and establishes the initial direction away from that endpoint.

The controller permits motor START with one limit active because the mode controller selects the direction away from it. ARM and FIRE remain blocked while either limit is active, and a limit transition while armed returns the controller to SAFE. The powered limit modules provide defined HIGH outputs when released and LOW outputs when pressed; limit monitoring and fault protection are enabled. Stored settings are sanity-checked on boot, and BLE speed commands must be numeric values from `0` to `255`.

An E-stop input path exists as a disabled placeholder with `ESTOP_PIN = -1`. Assigning that pin in `config.h` enables a pulled-to-ground emergency stop input that latches `FAULT: ESTOP`. BLE command writes are rate-limited by `BLE_COMMAND_MIN_INTERVAL_MS`.

Motor session timeout and motor overcurrent fault hooks exist but are disabled by default. They require configuring `ENABLE_MOTOR_SESSION_TIMEOUT` or `ENABLE_MOTOR_OVERCURRENT_FAULT` plus the related timeout/current-sense constants in `config.h`.

The Alpha bench controller uses the rotary encoder for speed and local menu control, a dedicated ARM button on GPIO16, and a dedicated FIRE / TEST button on GPIO17. Press the encoder to open the menu, rotate to navigate, and press to select. The menu contains Motor, Mode, Environment, Diagnostics, and About pages. Motor speed has an explicit edit mode; direction and operating-mode changes require the motor to be stopped. Live environment, limit, fault, and relay information is available from the read-only status pages. Select `< Back` to return or `Exit` to close the menu. Any latched fault closes the menu so safety status takes priority. Menu motor control uses the same enabled fault, limit, E-stop, and temperature protections as BLE. The relay can only fire when `systemArmed` is true and no fault is active. The encoder button never triggers the relay.

On boot the system always starts `SAFE` / unarmed and the relay is initialized off. Pressing ARM toggles `ARM ON` / `ARM OFF` in Serial and updates the OLED. Pressing FIRE while safe prints `FIRE BLOCKED - NOT ARMED`; pressing FIRE while armed pulses the relay using the existing non-blocking trigger timing.

The verified Hosyond 2.42-inch 128x64 SSD1309 OLED uses U8g2 and the official five-conductor JST lid harness: GND, 3.3 V, GPIO21 SDA, GPIO22 SCL, and GPIO5 RESET. It retains the existing home-screen layout: motor speed percentage, `SAFE`, `ARMED`, `FAULT`, `WARNING`, or `FIRING`, relay `ON`/`OFF`, and `Limit: OK`, `ACTIVE`, `FAULT: LIMIT`, or `FAULT: BOTH`. At startup it briefly shows an animated Crosswind rotor, moving wind streaks, and a system-start progress bar before transitioning to the home screen. The RGB status LED mirrors the same safety state with green ready, blue armed, red fault, yellow warning/hot, and a white/purple firing flash.

## Build

Install PlatformIO, then run:

```sh
pio run
```

Upload with:

```sh
pio run --target upload
```

GitHub Actions also builds this PlatformIO project on pushes and pull requests that touch `firmware/crosswind-esp32/`.

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

Modes accepted by BLE are `SWEEP`, `RANDOM`, `FLUSH`, and `CENTERING`. `SWEEP`, `RANDOM`, and `FLUSH` currently share bounded end-to-end movement. `CENTERING` measures a complete traverse and returns halfway before stopping. Automatic triggering remains disabled unless `ENABLE_AUTOMATIC_TRIGGER` is intentionally enabled and safety-tested.

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
| OLED RES | 5 |
| BME280 SDA | 21 |
| BME280 SCL | 22 |
| RGB LED red | 27 |
| RGB LED green | 12 |
| RGB LED blue | 4 |
| Potentiometer placeholder | 39 |
| E-stop placeholder | disabled |
| Motor current sense placeholder | disabled |

The DIYables RGB LED module is common cathode with built-in resistors: connect module `GND` to common ground, `R` to GPIO27, `G` to GPIO12, and `B` to GPIO4. GPIO12 is a boot strapping pin on many ESP32 boards. Keep it for the Alpha wiring above; GPIO5 is reserved for OLED reset and is not available as an RGB fallback.

The electrical source of truth for display connector numbering, harness fabrication, and acceptance testing is [OLED Lid Harness Standard](../../electrical/oled-lid-harness.md).

## Environmental Sensor

Alpha firmware reads a BME280 on the existing OLED I2C bus at GPIO21/GPIO22 no faster than once every 2 seconds by default. It tries address `0x76` first and then `0x77`, stores the last valid temperature, humidity, and pressure reading, and reports `ENV ERR` if a read fails. `HOT` appears at `TEMP_WARNING_F`; `TEMP FAULT` stops the motor when `ENABLE_TEMP_FAULTS` is true and temperature reaches `TEMP_FAULT_F`. The Serial/BLE status payload includes `pressureHpa`.

## Quick Test Checklist

See `firmware-test-checklist.md` for a fuller bench checklist and `fault-matrix.md` for fault behavior.

1. Upload firmware with PlatformIO.
2. Confirm Serial reports the I2C scan and SSD1309 address, then confirm the OLED startup test transitions to the home screen.
3. Confirm encoder rotation changes motor speed.
4. Confirm ARM toggles between `SAFE` and `ARMED`.
5. Confirm FIRE is blocked while `SAFE`.
6. Confirm FIRE activates the relay only while `ARMED`.
7. Confirm relay `COM`/`NO` act as dry-contact continuity only.
8. Confirm left/right limits show inactive and active correctly in Serial/OLED setup view.
9. Confirm motor PWM changes speed on the bench before connecting linkage.
10. Confirm a normal endpoint reverses motion, while both limits, a stuck limit, travel timeout, or temperature fault stops motor output and blocks triggering.

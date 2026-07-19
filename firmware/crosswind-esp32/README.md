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
- `motion.*` - non-blocking state machine for endpoints, modes, centering, calibration, and motion diagnostics.
- `modes.*` - compatibility adapter between the controller loop and motion engine.
- `storage.*` - Preferences-backed mode, last fault, and last speed storage.
- `ble_control.*` - optional BLE command interface.
- `environment.*` - BME280 temperature, humidity, and pressure support.
- `battery_monitor.*` - non-blocking divided-voltage sampling, filtering, status, calibration, and events.
- `display.*` - SSD1306 OLED status display.
- `status_led.*` - non-blocking DIYables RGB status LED control.
- `trigger.*` - non-blocking thrower relay pulse control.
- `diagnostics.*` - Serial startup diagnostics and runtime status payloads.

## Phase 1 Behavior

Default mode is `SWEEP`. The motion engine uses debounced endpoints, `millis()` timing, the existing motor ramp, and the existing direction-change dead time. See `motion-engine.md` for mode and safety details.

The expected single YL-99 is a normal endpoint: motion stops, dwells, and reverses away. Both active, an unexpected limit, a stuck departure limit, driving into an active endpoint, and travel timeout remain latched faults handled by the application safety layer.

The controller refuses START and ARM when both limits are active. With exactly one endpoint active, motion may start only away from it. FIRE remains blocked while either limit is active, and endpoint activation returns an armed controller to SAFE. Stored settings are sanity-checked on boot, and BLE speed commands must be numeric values from `0` to `255`.

An E-stop input path exists as a disabled placeholder with `ESTOP_PIN = -1`. Assigning that pin in `config.h` enables a pulled-to-ground emergency stop input that latches `FAULT: ESTOP`. BLE command writes are rate-limited by `BLE_COMMAND_MIN_INTERVAL_MS`.

Motor session timeout and motor overcurrent fault hooks exist but are disabled by default. They require configuring `ENABLE_MOTOR_SESSION_TIMEOUT` or `ENABLE_MOTOR_OVERCURRENT_FAULT` plus the related timeout/current-sense constants in `config.h`.

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
- `MOTION_STATUS`
- `CENTER`
- `CALIBRATE`
- `BATTERY_STATUS`
- `BATTERY_MONITOR=ON|OFF`
- `BATTERY_PROFILE=TOOL_20V|BUS_12V|CUSTOM`
- `BATTERY_CALIBRATE=<known volts>`
- `BATTERY_CALIBRATION_RESET`
- `CLEAR_FAULT`
- `TRIGGER`
- `FIRE`
- `LAUNCH`

Trigger commands and the FIRE / TEST button pulse the thrower relay only when the system is armed. They are ignored while faulted. The relay pulse remains non-blocking and uses `TRIGGER_PULSE_MS`.

Limit faults can be cleared from BLE with `CLEAR_FAULT`, or locally with the ARM/encoder button, only after both limit switches read clear.

Modes accepted by BLE are `SWEEP`, `RANDOM`, `FLUSH`, and `CENTERING`. Automatic FLUSH trigger events remain disabled unless `ENABLE_AUTOMATIC_TRIGGER` is explicitly enabled and the normal armed/trigger interlocks accept them.

## Motion Simulation

Build `esp32dev_motion_sim` to disable motor GPIO output and emulate virtual endpoints. See `motion-engine-test.md` for the deterministic procedure. Simulation is not physical validation.

## Battery Monitoring

Battery monitoring defaults disabled until its external divider is installed and verified. The default design measures the raw tool-battery input through a configurable 100 kOhm/12 kOhm divider into ADC1 GPIO36. Low voltage warns; sustained critical voltage is reported to the application fault handler. See `battery-monitor.md` and `battery-monitor-test.md`. Build `esp32dev_battery_sim` for deterministic voltage input with motor GPIO disabled.

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
| BME280 SDA | 21 |
| BME280 SCL | 22 |
| RGB LED red | 27 |
| RGB LED green | 12 |
| RGB LED blue | 4 |
| Potentiometer placeholder | 39 |
| E-stop placeholder | disabled |
| Motor current sense placeholder | disabled |

The DIYables RGB LED module is common cathode with built-in resistors: connect module `GND` to common ground, `R` to GPIO27, `G` to GPIO12, and `B` to GPIO4. GPIO12 is a boot strapping pin on many ESP32 boards; keep it for the Alpha wiring above, but if boot or upload problems appear, move green to another PWM-capable non-strapping pin such as GPIO5 and update `RGB_GREEN_PIN`.

## Environmental Sensor

Alpha firmware reads a BME280 on the existing OLED I2C bus at GPIO21/GPIO22 no faster than once every 2 seconds by default. It tries address `0x76` first and then `0x77`, stores the last valid temperature, humidity, and pressure reading, and reports `ENV ERR` if a read fails. `HOT` appears at `TEMP_WARNING_F`; `TEMP FAULT` stops the motor when `ENABLE_TEMP_FAULTS` is true and temperature reaches `TEMP_FAULT_F`. The Serial/BLE status payload includes `pressureHpa`.

## Quick Test Checklist

See `firmware-test-checklist.md` for a fuller bench checklist and `fault-matrix.md` for fault behavior.

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

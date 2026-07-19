# Firmware Test Checklist

Run these checks with the thrower unloaded and the motor linkage disconnected until motor direction and speed are verified.

## Build And Upload

- Run `pio run`.
- Upload with `pio run --target upload`.
- Open the Serial monitor at `115200`.
- Confirm startup diagnostics print firmware version, reset reason, pin/config summary, limit states, fault state, trigger state, and motor PWM.

## Display And Inputs

- Confirm SSD1306 OLED boots at I2C address `0x3C`.
- Confirm OLED boot screen is brief and does not delay safety processing.
- Confirm Status shows ARM/run/mode/motion/speed/battery/warning/BLE state.
- Rotate on Status and confirm speed changes without flash writes per detent.
- Short press to enter/select; long press to cancel/back.
- Follow `display-menu-test.md` for hierarchy, editing, timeouts, BLE sync, calibration confirmation, and fault priority.
- Press ARM and confirm `SAFE`/`ARMED` toggles when no fault or active limit exists.
- Press FIRE while `SAFE` and confirm relay is blocked.

## Relay

- Confirm relay `COM`/`NO` are dry-contact only with a continuity meter.
- Confirm ESP32 GPIO14 drives only the relay input side.
- While `ARMED`, press FIRE and confirm the relay pulse is non-blocking.
- Confirm repeated FIRE requests inside `MIN_TRIGGER_INTERVAL_MS` are blocked.
- Confirm rapid BLE command writes are rate-limited by `BLE_COMMAND_MIN_INTERVAL_MS`.

## Limits And Faults

- Confirm left YL-99 on GPIO34 reads clear/inactive, then active when triggered.
- Confirm right YL-99 on GPIO35 reads clear/inactive, then active when triggered.
- Confirm FIRE is blocked while either limit is active and START is blocked only for both active limits.
- Confirm a run starting at one endpoint commands motion only away from that endpoint.
- While sweeping on the bench, trigger the expected endpoint and confirm output stops, the system disarms, dwell occurs, and motion reverses without a latched fault.
- Hold the departure switch active and confirm `LIMIT_STUCK`; activate the opposite switch for commanded travel and confirm `UNEXPECTED_LIMIT`.
- Trigger both limits together and confirm `BOTH_LIMITS` fault.
- Confirm limit faults cannot clear until both switches are released.
- If `ESTOP_PIN` is configured, trigger it and confirm motor output stops, the system disarms, relay firing is blocked, and the fault cannot clear until released.

## Motor

- Confirm BTS7960 `RPWM`, `LPWM`, `R_EN`, and `L_EN` wiring before motor power.
- Confirm motor PWM changes speed on the bench before connecting linkage.
- Confirm STOP or any fault sets BTS7960 outputs off.
- Run RANDOM and confirm speed/time/dwell remain inside configured bounds.
- Run CENTERING and confirm it never requests the relay.
- Run CALIBRATE and confirm only a valid completed measurement updates stored travel time.
- If motor session timeout is enabled, confirm it faults after `MOTOR_SESSION_TIMEOUT_MS`.
- If motor current sensing is installed and enabled, confirm raw readings are plausible before setting an overcurrent threshold.

## Environment

- Confirm the BME280 is detected at address `0x76` or `0x77` and temperature, humidity, and `pressureHpa` appear in the status payload.
- Confirm temperature fault behavior only after verifying sensor readings are plausible.

## Battery Monitor

- Run `scripts/test-battery-monitor.ps1` and build `esp32dev_battery_sim`.
- Confirm disabled monitoring displays and reports `DISABLED` without affecting motion.
- Follow `battery-monitor-test.md` for settling, filtering, low, critical, recovery, invalid input, calibration, BLE, and OLED cases.
- Before physical testing, verify divider output below 3.3 V with a multimeter and never connect source voltage directly to GPIO36.

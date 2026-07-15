# Firmware Test Checklist

Run these checks with the thrower unloaded and the motor linkage disconnected until motor direction and speed are verified.

## Build And Upload

- Run `pio run`.
- Upload with `pio run --target upload`.
- Open the Serial monitor at `115200`.
- Confirm startup diagnostics print firmware version, reset reason, pin/config summary, limit states, fault state, trigger state, and motor PWM.

## Display And Inputs

- Confirm SSD1306 OLED boots at I2C address `0x3C`.
- Confirm OLED home screen shows motor percent, safety state, relay state, and limit status.
- Rotate encoder and confirm speed changes.
- Press encoder switch and confirm `MAIN`/`SETUP` display toggle when not faulted.
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
- Confirm START, ARM, and FIRE are blocked while either limit is active.
- While motor is running on the bench, trigger one limit and confirm motor output stops, the system disarms, RGB shows fault, and firing is blocked.
- Trigger both limits together and confirm `BOTH_LIMITS` fault.
- Confirm limit faults cannot clear until both switches are released.
- If `ESTOP_PIN` is configured, trigger it and confirm motor output stops, the system disarms, relay firing is blocked, and the fault cannot clear until released.

## Motor

- Confirm BTS7960 `RPWM`, `LPWM`, `R_EN`, and `L_EN` wiring before motor power.
- Confirm motor PWM changes speed on the bench before connecting linkage.
- Confirm STOP or any fault sets BTS7960 outputs off.
- If motor session timeout is enabled, confirm it faults after `MOTOR_SESSION_TIMEOUT_MS`.
- If motor current sensing is installed and enabled, confirm raw readings are plausible before setting an overcurrent threshold.

## Environment

- With `ENV_SENSOR_TYPE = ENV_SENSOR_DHT11`, confirm DHT11 status and temperature/humidity payloads.
- With `ENV_SENSOR_TYPE = ENV_SENSOR_BME280`, confirm address `0x76` or `0x77` and `pressureHpa` status payload.
- Confirm temperature fault behavior only after verifying sensor readings are plausible.

# Battery Monitor

`battery_monitor.cpp` is a non-blocking telemetry and event module. It samples one ADC value every `BATTERY_SAMPLE_INTERVAL_MS`, converts the divider pin measurement with `analogReadMilliVolts()`, applies an exponential moving average, converts through the configured divider ratio, and applies calibration. It never controls motion, relay GPIO, ARM state, or flash storage.

Monitoring defaults disabled until the divider is installed and verified. While enabled, startup remains `UNKNOWN` through the settling interval and filter warmup. UNKNOWN, CRITICAL, RECOVERING, SENSOR_ERROR, and an impossible latest reading block motor start/output through `main.cpp`.

Low voltage is a warning. Critical voltage requires consecutive filtered readings, which rejects brief motor-start sag; sustained critical voltage reports an event that the application latches as `BATTERY_CRITICAL`. Recovery requires consecutive readings above the recovery threshold, followed by manual fault clearing. Sensor errors stop and disarm; `ENABLE_BATTERY_SENSOR_FAULT` optionally makes them latched faults.

## Electrical defaults

- Measurement point: raw tool-battery input
- ADC: GPIO36 / ADC1_CH0, chosen because it is input-only, unused, and does not conflict with BLE like ADC2 can
- Divider: 100 kOhm upper, 12 kOhm lower; nominal ratio 9.333:1
- Attenuation: 11 dB
- Monitor enabled: false

At 26 V input, the nominal divider output is about 2.79 V. A nominal 3.3 V divider output corresponds to about 30.8 V input, but resistor tolerance, transients, and ADC limitations require design margin. Verify the divider output with a multimeter before connecting GPIO36. Raw battery voltage must never connect directly to the ESP32.

## Profiles

`TOOL_20V` defaults to full 21.0 V, nominal 18.0 V, low 16.5 V, critical 15.0 V, recovery 17.0 V, and valid range 5–26 V. `BUS_12V` uses 12.6/12.0/11.5/10.8/11.8 V and valid range 8–15 V. `CUSTOM` uses the centralized generic constants and is intended for compile-time project-specific values.

Percentage is disabled by default. When explicitly enabled it is a clamped, piecewise approximation for the generic `TOOL_20V` profile only, not a chemistry-accurate state-of-charge claim.

## Calibration and BLE

- `BATTERY_MONITOR=ON|OFF`
- `BATTERY_STATUS`
- `BATTERY_PROFILE=TOOL_20V|BUS_12V|CUSTOM`
- `BATTERY_CALIBRATE=<known volts>`
- `BATTERY_CALIBRATION_RESET`

Stop motion, measure the divider source with a trusted multimeter, and submit that value. Firmware derives and validates a correction multiplier between configured limits, then persists it. Live readings are never persisted.

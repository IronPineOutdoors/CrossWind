# Roadmap

## Phase 1

- Validate single-axis sweep with no thrower mounted.
- Fit VEVOR NH113 on adjustable rails.
- Measure wiper motor current draw.
- Finalize left/right switch bracket placement.
- Validate Alpha DHT11, OLED, rotary encoder, ARM, and FIRE controls in the enclosure.
- Field test with conservative sweep angle.

## Phase 2

- Prototype pitch cradle.
- Add pitch actuator and limits.
- Expand firmware for yaw + pitch modes.
- Add battery voltage sensing.
- Upgrade the environmental sensor to BME280 on the shared I2C bus for better temperature, humidity, and pressure data. Expected addresses are `0x76` or `0x77`.

## Production Exploration

- Weatherproof connectors.
- Refined enclosure and harness.
- Repeatable calibration.
- Universal rail kit.
- Production identity and model naming.

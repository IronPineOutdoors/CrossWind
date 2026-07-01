# Pinout

## ESP32 Pins

| Function | ESP32 Pin | Notes |
| --- | --- | --- |
| BTS7960 RPWM | GPIO25 | Right direction PWM |
| BTS7960 LPWM | GPIO13 | Left direction PWM; moved off GPIO26 for DHT11 data |
| BTS7960 R_EN | GPIO27 | Right enable |
| BTS7960 L_EN | GPIO14 | Left enable |
| DHT11 data | GPIO26 | Alpha enclosure temperature/humidity sensor |
| OLED SDA | GPIO21 | Shared I2C bus |
| OLED SCL | GPIO22 | Shared I2C bus |
| Left limit | GPIO32 | INPUT_PULLUP |
| Right limit | GPIO33 | INPUT_PULLUP |
| Start/stop button | GPIO18 | INPUT_PULLUP |
| Mode button | GPIO19 | INPUT_PULLUP |
| Speed potentiometer | GPIO39 | 0-3.3V analog input |
| Thrower trigger relay | GPIO23 | Relay input only; relay contacts are dry contact across pedal wires |
| Status LED | GPIO2 | Onboard LED on many dev boards |

## BTS7960 Pins

- `B+` and `B-` connect to the 12V battery branch through the correct fuse.
- `M+` and `M-` connect to the wiper motor.
- `RPWM`, `LPWM`, `R_EN`, and `L_EN` connect to the ESP32 pins above.
- ESP32 ground, BTS7960 logic ground, buck ground, and battery negative must be common.

## Limit Switches

Phase 1 assumes normally closed roller switches if possible. With ESP32 `INPUT_PULLUP`, normal travel reads LOW and an opened/triggered switch reads HIGH.

## Buttons

Start/stop and mode buttons connect from their GPIO pin to ground and use internal pullups.

## Potentiometer

Use a potentiometer wired between 3.3V and GND, with the wiper to GPIO39. Do not connect 5V to an ESP32 analog pin.

## Environmental Sensor

Crosswind Alpha uses a DHT11 because it is already available and good enough for early Apache case enclosure temperature and humidity checks. Wire DHT11 `VCC` to ESP32 `3V3`, `GND` to ESP32 `GND`, and `DATA` to GPIO26.

Mount the sensor inside the Apache case where air can circulate. Keep it away from the BTS7960 heat sink, motor driver body, and direct contact with the case wall so readings are not dominated by a local hot surface.

Crosswind Beta should prefer a BME280 for better temperature, humidity, and pressure data. A future BME280 can share the OLED I2C bus on GPIO21/GPIO22 and usually appears at address `0x76` or `0x77`.

## Thrower Trigger Relay

Use an opto-isolated relay module or equivalent dry-contact relay output. ESP32 GPIO23 drives the relay input. The relay `COM` and `NO` contacts wire in parallel with the VEVOR NH113 factory foot pedal wires after confirming the pedal pair with a continuity test.

The ESP32 must never send voltage into the thrower pedal circuit. Use the relay contacts as a switch only.

## Future Encoder And Battery Sense

Reserved placeholders exist in firmware for:

- Rotary encoder A/B
- Battery voltage divider input
- Pitch actuator output

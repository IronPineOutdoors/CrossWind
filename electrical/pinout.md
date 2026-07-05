# Pinout

## ESP32 Pins

| Function | ESP32 Pin | Notes |
| --- | --- | --- |
| BTS7960 RPWM | GPIO18 | Right direction PWM |
| BTS7960 LPWM | GPIO19 | Left direction PWM |
| BTS7960 R_EN | GPIO23 | Right enable |
| BTS7960 L_EN | GPIO13 | Left enable |
| DHT11 data | GPIO26 | Alpha enclosure temperature/humidity sensor |
| OLED SDA | GPIO21 | Shared I2C bus |
| OLED SCL | GPIO22 | Shared I2C bus |
| Left limit | GPIO27 | YL-99 module signal output, active LOW |
| Right limit | GPIO5 | YL-99 module signal output, active LOW |
| ARM button | GPIO16 | Button to GND, `INPUT_PULLUP`, pressed LOW |
| FIRE / TEST button | GPIO17 | Button to GND, `INPUT_PULLUP`, pressed LOW |
| Speed potentiometer | GPIO39 | 0-3.3V analog input |
| Rotary encoder CLK | GPIO32 | Active speed input in current ESP32 build |
| Rotary encoder DT | GPIO33 | Active speed input in current ESP32 build |
| Rotary encoder SW | GPIO25 | Menu/select button |
| Thrower trigger relay | GPIO14 | Relay input only; relay contacts are dry contact across pedal wires |
| Status LED | GPIO2 | Startup test, slow blink stopped, solid running, fast blink fault |

## BTS7960 Pins

- `B+` and `B-` connect to the 12V battery branch through the correct fuse.
- `M+` and `M-` connect to the wiper motor.
- `RPWM`, `LPWM`, `R_EN`, and `L_EN` connect to the ESP32 pins above.
- ESP32 ground, BTS7960 logic ground, buck ground, and battery negative must be common.

## Limit Switches

For YL-99 limit switch modules, power each module from ESP32 `3V3`, connect module `GND` to common ground, and connect module signal/output to the limit GPIO. The firmware uses `INPUT_PULLUP` and `LIMIT_ACTIVE_STATE = 0`, matching the current YL-99 behavior where a triggered switch pulls the signal LOW. Do not hold the GPIO5/right-limit switch active while powering or resetting the ESP32.

## Buttons

ARM and FIRE / TEST buttons connect from their GPIO pin to ground and use internal pullups. Pressed reads LOW.

## Potentiometer

Use a potentiometer wired between 3.3V and GND, with the wiper to GPIO39. Do not connect 5V to an ESP32 analog pin. The current ESP32 build is configured for the rotary encoder instead; switch `SPEED_INPUT_TYPE` in firmware if using the potentiometer.

## Rotary Encoder

Wire encoder `CLK` to GPIO32, `DT` to GPIO33, `SW` to GPIO25, `+`/`VCC` to ESP32 `3V3`, and `GND` to common ground. The firmware uses internal pullups, so the encoder outputs and switch should pull the pins to ground when active.

The encoder switch toggles the display menu between `MAIN` and `SETUP`. It never triggers the relay.

## Environmental Sensor

Crosswind Alpha uses a DHT11 because it is already available and good enough for early Apache case enclosure temperature and humidity checks. Wire DHT11 `VCC` to ESP32 `3V3`, `GND` to ESP32 `GND`, and `DATA` to GPIO26.

Mount the sensor inside the Apache case where air can circulate. Keep it away from the BTS7960 heat sink, motor driver body, and direct contact with the case wall so readings are not dominated by a local hot surface.

Crosswind Beta should prefer a BME280 for better temperature, humidity, and pressure data. A future BME280 can share the OLED I2C bus on GPIO21/GPIO22 and usually appears at address `0x76` or `0x77`.

## Thrower Trigger Relay

Use an opto-isolated relay module or equivalent dry-contact relay output. ESP32 GPIO14 drives the relay input. The relay `COM` and `NO` contacts wire in parallel with the VEVOR NH113 factory foot pedal wires after confirming the pedal pair with a continuity test.

The ESP32 must never send voltage into the thrower pedal circuit. Use the relay contacts as a switch only.

## Future Battery Sense

Reserved placeholders exist in firmware for:

- Battery voltage divider input
- Pitch actuator output

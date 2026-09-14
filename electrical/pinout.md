# Pinout

## ESP32 Pins

| Function | ESP32 Pin | Notes |
| --- | --- | --- |
| BTS7960 RPWM | GPIO18 | RPWM terminal; logical LEFT with current inverted mapping |
| BTS7960 LPWM | GPIO19 | LPWM terminal; logical RIGHT with current inverted mapping |
| BTS7960 R_EN | GPIO23 | Right enable |
| BTS7960 L_EN | GPIO13 | Left enable |
| SSD1309 OLED SDA | GPIO21 | Shared I2C bus |
| SSD1309 OLED SCL | GPIO22 | Shared I2C bus |
| SSD1309 OLED RES | GPIO5 | Active-LOW hardware reset for five-pin module |
| BME280 SDA | GPIO21 | Optional environment sensor on shared I2C bus |
| BME280 SCL | GPIO22 | Optional environment sensor on shared I2C bus |
| Left limit | GPIO34 | Green, DT pin 1; powered module OUT, active LOW; module onboard pull-up |
| Right limit | GPIO35 | Blue, DT pin 2; powered module OUT, active LOW; module onboard pull-up |
| ARM button | GPIO16 | Button to GND, `INPUT_PULLUP`, pressed LOW |
| FIRE / TEST button | GPIO17 | Button to GND, `INPUT_PULLUP`, pressed LOW |
| Speed potentiometer | GPIO39 | 0-3.3V analog input |
| Rotary encoder CLK | GPIO32 | Active speed input in current ESP32 build |
| Rotary encoder DT | GPIO33 | Active speed input in current ESP32 build |
| Rotary encoder SW | GPIO25 | Menu/select button |
| Thrower trigger relay | GPIO14 | Relay input only; relay contacts are dry contact across pedal wires |
| RGB LED red | GPIO27 | DIYables common cathode RGB module R input, PWM |
| RGB LED green | GPIO12 | DIYables common cathode RGB module G input, PWM |
| RGB LED blue | GPIO4 | DIYables common cathode RGB module B input, PWM |

## BTS7960 Pins

- `B+` and `B-` connect to the fused 12V bus from the high-current buck converter.
- `M+` and `M-` connect to the wiper motor.
- `RPWM`, `LPWM`, `R_EN`, and `L_EN` connect to the ESP32 pins above.
- ESP32 ground, BTS7960 logic ground, buck ground, and battery negative must be common.

## Limit Switches

Use the [finalized limit-switch harness standard](limit-switch-harness.md): Green / DT pin 1 is Left OUT, Blue / pin 2 is Right OUT, White / pin 3 is Lower OUT, Yellow / pin 4 is Upper OUT, Black / pin 5 is shared ground, and Red / pin 6 is regulated 3.3 V. The powered modules output HIGH normally and short OUT to ground when pressed, so active is LOW.

During Alpha, Green, Blue, Black, and Red are connected. White and Yellow remain terminated but unused. GPIO34/GPIO35 do not support internal pull-ups, so Alpha relies on the powered modules' onboard pull-ups. A broken or disconnected OUT wire leaves the input undefined and is not automatically detected. These switches command normal reversal and provide safety input, but separate physical hard stops remain required.

## RGB Status LED

The DIYables RGB LED module is common cathode with built-in resistors. Connect module `GND` to common ground, `R` to GPIO27, `G` to GPIO12, and `B` to GPIO4. Because common cathode colors turn on when driven HIGH, PWM values above 0 illuminate the color and PWM 0 turns it off.

GPIO12 can affect boot mode on some ESP32 boards if externally pulled at reset. Keep GPIO12 for the current Alpha wiring unless it causes boot or upload issues. GPIO5 is now reserved for OLED reset and must not be used as an RGB fallback.

## Buttons

ARM and FIRE / TEST buttons connect from their GPIO pin to ground and use internal pullups. Pressed reads LOW.

## Potentiometer

Use a potentiometer wired between 3.3V and GND, with the wiper to GPIO39. Do not connect 5V to an ESP32 analog pin. The current ESP32 build is configured for the rotary encoder instead; switch `SPEED_INPUT_TYPE` in firmware if using the potentiometer.

## Rotary Encoder

Wire encoder `CLK` to GPIO32, `DT` to GPIO33, `SW` to GPIO25, `+`/`VCC` to ESP32 `3V3`, and `GND` to common ground. The firmware uses internal pullups, so the encoder outputs and switch should pull the pins to ground when active.

The encoder switch opens/selects the local Motor, Mode, Environment, Diagnostics, and About menus. It never triggers the relay.

The Hosyond 2.42-inch 128x64 SSD1309 OLED shares the I2C bus on GPIO21/GPIO22. Connect OLED `VCC` to ESP32 `3V3`, `GND` to system ground, `SDA` to GPIO21, `SCL` to GPIO22, and `RES` to GPIO5. Crosswind continues to supply 3.3 V even though the module listing indicates 3.3-5 V compatibility. Firmware pulses the active-LOW reset before scanning the bus, uses centralized address `0x3C`, and retains `0x3D` as a diagnostic fallback.

These five signals must run through the official five-position JST lid connector. Use the numbered pinout, diagram, BOM, and fabrication procedure in [OLED Lid Harness Standard](oled-lid-harness.md).

## Environmental Sensor

Crosswind Alpha uses a BME280 on the OLED I2C bus for temperature, humidity, and pressure. Wire BME280 `VCC` to ESP32 `3V3`, `GND` to common ground, `SDA` to GPIO21, and `SCL` to GPIO22. The firmware tries address `0x76`, then `0x77`.

Mount the sensor inside the enclosure where air can circulate. Keep it away from the BTS7960 heat sink, motor driver body, and direct contact with the case wall so readings are not dominated by a local hot surface.

## Thrower Trigger Relay

Use an opto-isolated relay module or equivalent dry-contact relay output. ESP32 GPIO14 drives the relay input. The relay `COM` and `NO` contacts wire in parallel with the VEVOR NH113 factory foot pedal wires after confirming the pedal pair with a continuity test.

The ESP32 must never send voltage into the thrower pedal circuit. Use the relay contacts as a switch only.

## Future Battery Sense

Reserved placeholders exist in firmware for:

- Battery voltage divider input, currently disabled with `BATTERY_VOLTAGE_PIN = -1`
- Motor current sense input, currently disabled with `MOTOR_CURRENT_SENSE_PIN = -1`
- Pitch actuator output

# Pinout

## ESP32 Pins

| Function | ESP32 Pin | Notes |
| --- | --- | --- |
| BTS7960 RPWM | GPIO25 | Right direction PWM |
| BTS7960 LPWM | GPIO26 | Left direction PWM |
| BTS7960 R_EN | GPIO27 | Right enable |
| BTS7960 L_EN | GPIO14 | Left enable |
| Left limit | GPIO32 | INPUT_PULLUP |
| Right limit | GPIO33 | INPUT_PULLUP |
| Start/stop button | GPIO18 | INPUT_PULLUP |
| Mode button | GPIO19 | INPUT_PULLUP |
| Speed potentiometer | GPIO39 | 0-3.3V analog input |
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

## Future OLED, Encoder, And Battery Sense

Reserved placeholders exist in firmware for:

- OLED SDA/SCL
- Rotary encoder A/B
- Battery voltage divider input
- Pitch actuator output

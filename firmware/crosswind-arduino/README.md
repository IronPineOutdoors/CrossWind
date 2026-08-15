# Crosswind Arduino AVR Firmware

This folder contains the Arduino Uno/Nano Phase 1 fallback firmware.

Open `Crosswind_Controller/Crosswind_Controller.ino` in the Arduino IDE, select an Uno or Nano board, and upload.

The AVR firmware is a legacy fallback and does not yet mirror the hardware-validated `v1.12.0-rc.1` ESP32 bounded-motion state machine:

- Default mode is `SWEEP`.
- The BTS7960 outputs are handled so both directions are never energized at once.
- Direction changes zero both PWM channels before reversing.
- Limit switches default to normally closed wiring with configurable active state.
- EEPROM stores only mode, last fault, and last speed.

Do not use this fallback for the current Alpha release candidate without separately reviewing and porting endpoint reversal, stuck-limit timeout, travel timeout, and timed centering. The ESP32 firmware is the validated controller for Alpha.

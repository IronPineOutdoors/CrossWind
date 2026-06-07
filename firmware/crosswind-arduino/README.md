# Crosswind Arduino AVR Firmware

This folder contains the Arduino Uno/Nano Phase 1 fallback firmware.

Open `Crosswind_Controller/Crosswind_Controller.ino` in the Arduino IDE, select an Uno or Nano board, and upload.

The AVR firmware mirrors the Phase 1 behavior of the ESP32 firmware:

- Default mode is `SWEEP`.
- The BTS7960 outputs are handled so both directions are never energized at once.
- Direction changes zero both PWM channels before reversing.
- Limit switches default to normally closed wiring with configurable active state.
- EEPROM stores only mode, last fault, and last speed.

Use this firmware when the prototype needs a simple local controller without BLE.

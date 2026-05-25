# CrossWind

CrossWind is a small Arduino-based project for Iron Pine Outdoors that includes support for both classic Arduino and ESP32 platforms. The repository contains two folder roots:

- `crosswind/` — contains the classic Arduino sketch and supporting files.
- `crosswind_esp32/` — contains the ESP32-specific sketch and files.

## Getting Started

1. Open the appropriate folder in the Arduino IDE:
   - `crosswind/crosswind.ino` for Arduino boards
   - `crosswind/crosswind_esp32/crosswind_esp32.ino` for ESP32 boards
2. Select the matching board and port in the Arduino IDE.
3. Upload to the device.

## Structure

- `crosswind/` — Classic Arduino project files.
- `crosswind_esp32/` — ESP32 project files.

## Notes

- Keep board-specific code separate between the two folders.
- Use the Arduino IDE or compatible build tools for compiling and uploading.

## License

This repository currently does not specify a license. Add a `LICENSE` file if you want to define usage and distribution terms.

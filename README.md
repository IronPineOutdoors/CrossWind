# CrossWind

CrossWind is a small motor control project for Iron Pine Outdoors with support for both classic Arduino and ESP32 hardware.

## Repository Layout

- `crosswind/` — Classic Arduino sketch for standard AVR boards.
- `crosswind_esp32/` — ESP32 sketch with BLE remote control and persistent settings.

## Features

### Classic Arduino (`crosswind/`)
- Manual, random, flush, and centering drive modes.
- Non-blocking flush sequencing to keep the controller responsive.
- Debounced start/stop and mode buttons.
- Safety stop if both limit switches are triggered.
- Optional serial debug support via `DEBUG_SERIAL`.

### ESP32 (`crosswind_esp32/`)
- BLE commands for remote control.
- Supported commands:
  - `START` / `RUN`
  - `STOP` / `PAUSE`
  - `STATUS`
  - `PING`
  - `RESET`
  - `MODE=MANUAL|RANDOM|AUTO|FLUSH|CENTERING|CENTER`
  - `DIRECTION=FORWARD|REVERSE`
  - `SPEED=<50-255>`
- Persisted mode, direction, and speed settings.
- Safety stop if both limit switches are active.

## Getting Started

1. Open the appropriate folder in the Arduino IDE or the ESP32-compatible environment.
2. Select the correct board and port.
3. Upload the sketch.

## Notes

- Use the Arduino IDE or `arduino-cli` for AVR boards.
- Use ESP32 board support for `crosswind_esp32/`.
- Add a `LICENSE` file if you want to define usage and distribution terms.

# Crosswind

Crosswind is an Iron Pine Outdoors prototype for programmable target presentation: a universal wobbler base that can add controlled yaw, and later pitch, to automatic clay throwers.

Phase 1 is a single-axis yaw/sweep prototype. It uses a rotating top plate on a lazy susan bearing, a crank-driven wiper motor, a BTS7960 / IBT-2 motor driver, and two YL-99 roller limit switch modules used as safety/calibration inputs.

Phase 2 is planned as a dual-axis yaw + pitch system with programmable presentation modes.

## Current Phase

Phase 1: single-axis sweep prototype.

The first mechanical fitment target is a VEVOR NH113 thrower, but the base, rails, and mounting approach should remain adjustable so Crosswind can support other automatic throwers later.

## Phase 1 Hardware

- ESP32 dev board
- Optional Arduino Uno/Nano fallback controller
- BTS7960 / IBT-2 motor driver
- 12V Mitsubishi Outlander rear wiper motor
- Left and right YL-99 limit switch modules
- Rotary encoder speed control with menu/select button
- Dedicated ARM and FIRE / TEST buttons
- SSD1306 OLED status display
- DHT11 enclosure temperature/humidity sensor by default, with optional BME280 support on I2C
- Dry-contact thrower trigger relay
- DIYables common-cathode RGB status LED
- 20V/24V tool battery input
- High-current buck converter for 12V bus
- Fused 5V buck converter for controller/support electronics
- Waterproof electronics box
- 28" x 28" 3/4" plywood base
- 20" x 20" rotating top plate
- 12" lazy susan bearing

## Folder Structure

- `firmware/crosswind-esp32/` - PlatformIO ESP32 firmware split into motor, limits, inputs, modes, storage, BLE, environment, display, RGB status LED, trigger, and diagnostics modules.
- `firmware/crosswind-arduino/` - Arduino Uno/Nano fallback firmware.
- `mechanical/` - Phase 1 and Phase 2 mechanical notes, dimensions, fitment, and cut lists.
- `electrical/` - Pinout, power system, fusing, the [Deutsch DT 6-pin limit-switch harness standard](electrical/limit-switch-harness.md), and wiring checklist.
- `cad/` - CAD export/drop folders for Phase 1 and Phase 2.
- `testing/` - Bench, motor, sweep, runtime, and field test records.
- `branding/` - Iron Pine Outdoors and Crosswind product identity notes.
- `docs/` - Requirements, build plans, safety notes, changelog, and roadmap.

## Quick Start

1. Read `docs/safety-notes.md`.
2. Review `electrical/pinout.md` before wiring.
3. Build the Phase 1 base from `mechanical/phase-1/dimensions.md` and `mechanical/phase-1/cut-list.md`.
4. Flash the ESP32 firmware from `firmware/crosswind-esp32/`.
5. Run `testing/bench-test-checklist.md` with the thrower removed.
6. Mount the thrower only after switch, motor, and fault behavior are verified.

## Safety Warning

Crosswind moves heavy equipment with a 12V motor. Keep hands clear of linkages, rotating plates, pinch points, and the thrower arm path. Use fuses, a shared ground, strain relief, and a reachable power disconnect. Bench test without the thrower mounted before any live thrower test.

## Next Milestones

- Finish Phase 1 wiring and enclosure layout.
- Validate YL-99 safety switch placement with rotating flags/tabs.
- Record no-load motor current and loaded sweep current.
- Fit the VEVOR NH113 on adjustable rails.
- Add Phase 2 pitch-axis mechanical sketches.
- Decide whether the production controller remains ESP32-only or keeps an AVR fallback.

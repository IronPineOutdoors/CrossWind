# Crosswind

Crosswind is an Iron Pine Outdoors prototype for programmable target presentation: a universal wobbler base that can add controlled yaw, and later pitch, to automatic clay throwers.

Phase 1 is a single-axis yaw/sweep prototype. It uses a rotating thrower deck on a lazy susan bearing, a reversing wiper-motor linkage, a BTS7960 / IBT-2 motor driver, and two YL-99 roller limit modules that define the normal left/right travel boundaries and provide fault protection.

Phase 2 is planned as a dual-axis yaw + pitch system with programmable presentation modes.

ESP32 source identifies as `v1.12.0-rc.1`. Earlier Alpha bench results passed bounded motion and fault checks; the later motor-direction correction still needs physical verification. See [current status](docs/STATUS.md) for evidence and remaining integration work. The hardware below describes the Alpha design, not a completed assembly checklist.

## Current Phase

Phase 1: single-axis sweep prototype.

The first mechanical fitment target is a VEVOR NH113 thrower, but the base, rails, and mounting approach should remain adjustable so Crosswind can support other automatic throwers later.

## Phase 1 Hardware

- ESP32 dev board
- Legacy Arduino Uno/Nano fallback (not Alpha-equivalent)
- BTS7960 / IBT-2 motor driver
- 12V Mitsubishi Outlander rear wiper motor
- Left and right YL-99 limit switch modules
- Rotary encoder speed control with menu/select button
- Dedicated ARM and FIRE / TEST buttons
- Hosyond 2.42-inch 128x64 SSD1309 I2C OLED status display (ASIN B0G2RFLG1L) with the official five-conductor JST lid harness
- BME280 temperature, humidity, and pressure sensor on the shared I2C bus
- Dry-contact thrower trigger relay
- DIYables common-cathode RGB status LED
- 20V/24V tool battery input
- High-current buck converter for 12V bus
- Fused 5V buck converter for controller/support electronics
- Waterproof electronics box
- 28" x 28" 3/4" plywood base
- 20" x 20" rotating thrower deck
- 12" lazy susan bearing

## Folder Structure

- `firmware/crosswind-esp32/` - PlatformIO ESP32 firmware split into motor, limits, inputs, modes, storage, BLE, environment, display, RGB status LED, trigger, and diagnostics modules.
- `firmware/crosswind-arduino/` - Arduino Uno/Nano fallback firmware.
- `mechanical/` - Phase 1 and Phase 2 mechanical notes, dimensions, fitment, and cut lists.
- `electrical/` - Pinout, power system, fusing, the [five-pin OLED lid-harness standard](electrical/oled-lid-harness.md), the [Deutsch DT 6-pin limit-switch harness standard](electrical/limit-switch-harness.md), and wiring checklist.
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

## Project Context

- [Current status](docs/STATUS.md): implemented behavior, physical evidence and open questions.
- [Roadmap](docs/roadmap.md): next engineering milestones.
- [Engineering decisions](docs/DECISIONS.md): durable choices and rationale.
- [Codex working guide](AGENTS.md): repository operating instructions.
- [Documentation index](docs/README.md): requirements, build plans, safety and history.

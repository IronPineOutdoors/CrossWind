# Roadmap

Current implementation and test evidence live in [STATUS](STATUS.md); rationale lives in [DECISIONS](DECISIONS.md).

## Phase 1

- Close out unloaded motion after the direction correction: verify physical direction, both endpoints and start-on-limit behavior; inspect and secure shaft/coupling retention.
- Mount feet and spacers, secure wiring, finalize adjustable endpoint flags/brackets and verify independent mechanical stops and reachable power disconnect.
- Record STOP, ARM/FIRE, fault/recovery, centering and mode-selection results against the flashed commit; resolve safety behavior questions identified in STATUS.
- Validate OLED cold startup, BME280, encoder menus, buttons, RGB and BLE status in the completed enclosure.
- Measure motor startup, no-load and loaded current; finalize branch fuse sizes.
- Validate VEVOR NH113 fit and stability on adjustable thrower-deck rails, then perform conservative loaded sweep and field/runtime tests.
- Fit-check the revised OLED window and assembled PCB-header prototype. Verify IPC-101 board/interface and adapter continuity, then implement explicit panel support while preserving legacy controls and independently validating STOP integration.

## Phase 2 / Alpha X2

- Prototype pitch cradle; select actuator and add pitch limits/feedback with axis-specific fault handling.
- Expand firmware for coordinated yaw/pitch and define distinct RANDOM/FLUSH presentation behavior before implementation.
- Add and validate battery voltage sensing and account for pitch current in the power budget.

## Production Exploration

- Qualify weather-resistant enclosure/connectors and refine serviceable harnesses.
- Establish repeatable calibration, a validated universal rail kit and durability/runtime evidence.
- Decide the production controller/fallback policy and refine product identity/model naming.

Use the [archived feature-work assessment](branch-consolidation-2026-09-13.md) when planning battery sensing, distinct modes or menu enhancements; port selectively against current hardware and safety behavior.

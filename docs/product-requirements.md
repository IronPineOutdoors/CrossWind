# Product Requirements

## Phase 1 Requirements

- Provide a single-axis yaw/sweep base for automatic clay throwers.
- Support the VEVOR NH113 as the first fitment target.
- Keep the thrower mounting approach adjustable for universal compatibility.
- Use two roller limit switches and one striker tab for left/right travel boundaries.
- Run from a shared 12V battery system.
- Use an ESP32 controller with rotary speed control, ARM/FIRE safety controls, OLED status, and dry-contact relay triggering.
- Default to `SWEEP` mode.
- Stop immediately if both limits are active.
- Stop and fault if travel exceeds the configured timeout.
- Work fully without BLE connected.

## Phase 2 Requirements

- Add pitch control using a tilt cradle or equivalent thrower support frame.
- Add pitch limit switches or position feedback.
- Expand ESP32 control to coordinate yaw and pitch.
- Preserve safe fault behavior for each motion axis.
- Explore programmable target presentation patterns.
- Account for higher battery draw from a second actuator.

## Future Production Requirements

- Weather-resistant enclosure and connectors.
- Serviceable wiring harness.
- Clearly labeled controls and fuse access.
- Stable mounting rails for multiple thrower footprints.
- Repeatable calibration procedure.
- Battery voltage monitoring.
- OLED or app-based status display.
- Field test logs for durability, runtime, and transport vibration.

## Non-Goals

- Phase 1 does not need pitch.
- Phase 1 does not need a mobile app to operate.
- Phase 1 does not need production-grade molded parts.
- Phase 1 does not need a universal rail kit finalized before basic motion is proven.

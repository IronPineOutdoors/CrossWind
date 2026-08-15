# Product Requirements

## Phase 1 Requirements

- Provide a single-axis yaw/sweep base for automatic clay throwers.
- Support the VEVOR NH113 as the first fitment target.
- Keep the thrower mounting approach adjustable for universal compatibility.
- Use two YL-99 roller limit modules as normal left/right travel boundaries and safety inputs. They are not physical hard stops and must actuate before separate mechanical stops.
- Run from a 20V/24V tool battery stepped down to a fused 12V bus and fused 5V electronics bus.
- Use an ESP32 controller with rotary speed control, ARM/FIRE safety controls, OLED status, and dry-contact relay triggering.
- Default to `SWEEP` mode.
- Stop and reverse after the limit in the direction of travel activates. Permit motion only away from a single active limit; fault if both limits activate, a departed limit fails to release, or the opposite endpoint is not reached before the travel timeout.
- Provide timed `CENTERING` by measuring one end-to-end traverse and returning for half the measured travel time.
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

# Phase 1 Motion Engine

`motion.cpp` owns motion sequencing but not GPIO, fault latching, or relay safety. It reads the debounced `limits` API, sends commands through the existing `motor` API, and reports faults/events to `main.cpp`. `main.cpp` remains responsible for stopping, disarming, cancelling the relay, recording faults, and applying ARM/automatic-trigger gates.

The internal states are IDLE, STARTING, MOVING_LEFT, MOVING_RIGHT, DECELERATING, ENDPOINT_REACHED, PAUSED, REVERSING, CENTERING, STOPPING, and FAULTED. Motion timing is non-blocking and uses `millis()`.

## Modes

- SWEEP travels to an endpoint, stops, dwells, reverses away, requires release, and repeats.
- RANDOM chooses a safe direction, bounded PWM, bounded dwell, and either a bounded partial move or endpoint-to-endpoint move. Physical limits and maximum travel timing always apply.
- FLUSH waits ready, moves for a configured interval or endpoint, stops, pauses, and repeats. It emits a trigger-request event only when `ENABLE_AUTOMATIC_TRIGGER` is true; `main.cpp` still requires ARM and the existing safe trigger API.
- CENTERING seeks the left reference endpoint slowly, dwells, then moves right for `CENTERING_PERCENT` of calibrated full travel. Without calibration it uses `DEFAULT_FULL_TRAVEL_TIME_MS`. It never emits a trigger event.

## Calibration

Calibration is explicitly requested with BLE `CALIBRATE`; it never runs at boot. It seeks left, dwells, measures left-to-right travel, validates the value against `MIN_VALID_CALIBRATION_TIME_MS` and `MAX_VALID_CALIBRATION_TIME_MS`, and only then persists it through Preferences. Failed or incomplete attempts never overwrite the last valid value.

The current routine measures one direction. A future branch may measure both directions and average them after physical characterization.

## Assumptions

`DIR_LEFT` must physically approach the left switch and `DIR_RIGHT` the right switch. Exactly one active switch is a valid starting condition only when moving away. Switches are firmware endpoints, not load-bearing hard stops. Time-based center and RANDOM partial travel are approximate because Phase 1 has no position encoder.

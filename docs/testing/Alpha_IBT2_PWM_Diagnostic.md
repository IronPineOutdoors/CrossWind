# Alpha IBT-2 PWM diagnostic

## Bench symptom and root cause

BLE `START` was accepted and the controller entered `RUNNING`. `R_EN` rose to
about 3.2 V, but RPWM and LPWM remained at 0 V and the motor did not move.

The command and ramp path was running. The defect was the final IBT-2 output
truth table in `motor.cpp`: for right motion it set `R_EN` high but forced
`L_EN` low, and for left motion it did the inverse. The BTS7960 module needs
both half-bridge enable inputs high while either PWM input commands motion.
Disabling one half prevented a complete motor-current path.

## Control-flow trace

`BLE write` -> `CommandCallback::onWrite()` -> `handleBleCommand(START)` ->
`state.running = true` -> main `loop()` -> `motorAllowed()` ->
`updateMode()` / bounded mode state machine -> `driveMotor(state.direction, state.speed)`
-> `updateMotorRamp()` -> `writeOutputs()` -> both enables high -> exactly one
of RPWM/LPWM receives the ramped duty.

The default speed is 64 (approximately 25%). With the rotary encoder selected,
`readSpeedPwm()` updates the requested speed each loop; a deliberately selected
speed of zero therefore remains a safe stop request.

## Hardware and PWM mapping

| Signal | ESP32 GPIO | PWM channel (Arduino-ESP32 2.x) |
| --- | ---: | ---: |
| RPWM | 18 | 0 |
| LPWM | 19 | 1 |
| R_EN | 23 | n/a |
| L_EN | 13 | n/a |

PWM is 5 kHz, 8-bit, with raw duty 0-255. Nonzero run requests below 45 are
raised to the configured minimum duty of 45. The normal default is 64
(approximately 25%). The installed build uses Arduino-ESP32 2.0.17, so PWM is
configured with `ledcSetup()`, attached with `ledcAttachPin()`, and written by
channel. The source retains the matching Arduino-ESP32 3.x pin-based API path.
Either setup failure leaves both enables low and all reported duties at zero.

## Safe output behavior

| Commanded state | R_EN | L_EN | RPWM | LPWM |
| --- | ---: | ---: | ---: | ---: |
| Right / forward | HIGH | HIGH | duty | 0 |
| Left / reverse | HIGH | HIGH | 0 | duty |
| Stop, zero speed, inhibit, or PWM init failure | LOW | LOW | 0 | 0 |

RPWM and LPWM are never nonzero together. A direction change first writes both
PWM values to zero and both enables low, waits the configured 50 ms using the
existing nonblocking dead-time state, and then applies the opposite output.
`STOP` immediately clears the request, duties, pending reversal, and enables.

## Diagnostics

Serial reports PWM initialization and the RPWM/LPWM configuration at boot.
BLE `START` logs state, requested speed and direction, fault, and both limit
states. BLE `STOP` is logged. Motor output changes log R_EN, L_EN, RPWM, and
LPWM. The periodic Serial/BLE status includes the motion state, requested
speed/direction, active fault, E-stop, left/right limits, both enable commands,
both PWM duties, and PWM readiness. Output logging is change-driven rather
than emitted on every loop iteration.

## Controlled bench test

The diagnostic option `ENABLE_IBT2_BENCH_TEST` is false by default. It cannot
run automatically. For a supervised test with the linkage unloaded:

1. Verify E-stop and both limits are clear and normal firmware reports
   `pwmReady=1`.
2. Set `ENABLE_IBT2_BENCH_TEST` to `true`, rebuild, and flash.
3. With the normal controller stopped, send `IBT2_BENCH_FORWARD`. The command
   applies duty 64 (about 25%) for one second, then performs a complete stop.
4. Test reverse only as a separate deliberate action by sending
   `IBT2_BENCH_REVERSE`.
5. Send `STOP` at any time to end the test immediately. A fault, E-stop, or
   the limit in the commanded direction also aborts it.
6. Restore the flag to `false` after testing.

A multimeter displays approximately the PWM average: at 3.3 V logic, 25% may
read about 0.8 V, 50% about 1.6 V, and 75% about 2.5 V. Use an oscilloscope or
logic analyzer to confirm frequency and waveform.

## Validation and remaining hardware checks

PlatformIO builds the `esp32dev` environment successfully. GPIO searches show
no duplicate use of GPIO18, GPIO19, GPIO23, or GPIO13, and the two legacy LEDC
channels are distinct. Static output-path review covers forward, reverse,
stop, zero speed, reversal dead time, and PWM setup failure. Runtime safety
gating remains in the main and bounded-mode state machines: E-stop, temperature,
endpoint direction constraints, both-limits/stuck-limit/travel-timeout checks,
session timeout, overcurrent hooks, and fault state all retain authority to stop
motor output.

On the bench, confirm both enable terminals reach about 3.3 V during motion,
only the selected PWM terminal shows a waveform/average, and all four control
terminals return low on `STOP`. If PWM is present but the motor still does not
move, then check IBT-2 logic ground continuity, motor supply at B+/B-, motor
connections at M+/M-, and the module itself. Pin assignments were not changed.

## Files changed

- `firmware/crosswind-esp32/src/config.h`
- `firmware/crosswind-esp32/src/motor.h`
- `firmware/crosswind-esp32/src/motor.cpp`
- `firmware/crosswind-esp32/src/main.cpp`
- `firmware/crosswind-esp32/src/diagnostics.cpp`
- `docs/testing/Alpha_IBT2_PWM_Diagnostic.md`

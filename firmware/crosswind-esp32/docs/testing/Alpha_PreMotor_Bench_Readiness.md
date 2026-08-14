# CrossWind Alpha Pre-Motor Bench Readiness

## 1. Original Bench Symptoms

BLE connected and accepted START/STOP. START changed the UI to RUNNING and raised R_EN to about 3.2 V, but RPWM and LPWM both remained at 0 V.

## 2. Root Cause

Repository history shows the original motor output function incorrectly used the IBT-2 enable pins as direction selects: RIGHT asserted only R_EN and LEFT asserted only L_EN. A BTS7960 needs both R_EN and L_EN asserted during either direction; RPWM versus LPWM selects direction. Commit `92c20dd` partially corrected that truth table and added checked LEDC initialization.

This pass closed remaining command-path defects: there was no BLE forward/reverse command, BLE disconnect retained the running command, a stored zero speed could make START silent, and a BLE speed setting was overwritten on the next loop because it was not synchronized to the active input value.

## 3. Pin Map

| Function | GPIO | Mode / notes |
|---|---:|---|
| R_EN | 23 | output |
| L_EN | 13 | output |
| RPWM | 18 | LEDC output |
| LPWM | 19 | LEDC output |
| Left limit | 34 | input-only, active LOW, external/module bias required |
| Right limit | 35 | input-only, active LOW, external/module bias required |
| E-stop | disabled (`-1`) | no Alpha GPIO assigned |

The repository-wide audit found no duplicate GPIO assignments. GPIO34/35 are valid inputs but cannot supply internal pull-ups; firmware now uses `INPUT`. GPIO12 is a boot-strapping pin used by the RGB green input and remains documented; no motor pin is a strapping or input-only pin. BLE uses the ESP32 radio and GATT UUIDs, not control GPIOs.

## 4. PWM Configuration

PlatformIO target is `esp32dev`, platform 7.0.1, Arduino-ESP32 2.0.17. Motor PWM is 5 kHz, 8-bit, range 0-255. With the installed 2.x core, RPWM uses LEDC channel 0 and LPWM channel 1 through `ledcSetup`/`ledcAttachPin`; RGB uses channels 2-4. The guarded 3.x branch uses pin-based `ledcAttach`/`ledcWrite`. Attachment failure disables all motor outputs. Boot calls the motor initializer before other peripherals and commands both PWM outputs and enables low.

## 5. IBT-2 Truth Table

| State | R_EN | L_EN | RPWM | LPWM |
|---|---:|---:|---:|---:|
| STOP / invalid / fault | 0 | 0 | 0 | 0 |
| FORWARD / RIGHT | 1 | 1 | requested duty | 0 |
| REVERSE / LEFT | 1 | 1 | 0 | requested duty |

All four pins are written only by `motor.cpp`. Its bounded output function clears the inactive PWM first and has no state that can assert both PWM duties.

## 6. Default Speed

The Alpha default is defined once as 64/255 (about 25%). Missing, corrupt, or zero stored speed is replaced by this default. START also applies it if state speed is unexpectedly zero. BLE `SPEED=0..255` and the encoder remain configurable and synchronized.

## 7. Direction Logic

Initial direction remains the existing deterministic RIGHT direction. BLE accepts `FORWARD`, `REVERSE`, `DIRECTION=RIGHT`, and `DIRECTION=LEFT`. RIGHT maps to RPWM and LEFT maps to LPWM.

## 8. Reversal Dead-Time

Changing direction while moving immediately commands both PWM outputs and both enables low, enters a nonblocking 50 ms dead-time, then ramps the opposite PWM output. No loop-blocking delay is used.

## 9. Limit-Switch Behavior

Both powered YL-99 outputs are active LOW with 20 ms software debounce. Clear is HIGH and triggered is LOW. GPIO34/35 have no internal pull-ups, so the module output or an external pull-up must provide a defined clear state.

An active RIGHT limit immediately inhibits RIGHT/RPWM; LEFT movement away remains permitted. An active LEFT limit inhibits LEFT/LPWM; RIGHT movement away remains permitted. Both active inhibits either direction. Automatic reversal was not present in the Alpha state machine and was not invented. `ENABLE_LIMIT_FAULTS` remains disabled pending hardware wiring verification; directional inhibition remains active independently.

## 10. BLE Start/Stop Behavior

Connect does not move the motor. START selects the current valid direction, guarantees nonzero default duty, and enters the normal ramp. STOP synchronously clears running, pattern state, trigger request, both PWM outputs, and both enables. A later START resumes normally.

## 11. BLE Disconnect Behavior

Disconnect now invokes an immediate safe stop and cancels the trigger before advertising restarts. No nonzero output can remain latched.

## 12. Diagnostic Logging

Event logs cover BLE connection/disconnection, START/STOP, direction/duty changes, output truth-table changes, limit transitions, PWM initialization, reversal state, and disconnect safe stop. The existing once-per-second status payload reports running, direction, requested speed, applied PWM, both enables, both PWM duties, limits, E-stop, and PWM readiness.

## 13. No-Motor Bench Test

Leave M+ and M- disconnected. Confirm B+ to B- is about 12 V, VCC to GND is about 5 V, and ESP32/IBT-2 grounds are common. At STOP verify all four logic outputs are near 0 V. Send START/`FORWARD`: both enables should be about 3.3 V, RPWM measurable, LPWM near 0 V. Send `REVERSE`: observe the zero/dead-time interval, then RPWM near 0 V and LPWM measurable. STOP again.

At 25% duty a multimeter typically averages 3.3 V PWM to about 0.8 V; an oscilloscope or logic analyzer is preferred. With motion commanded toward each limit, trigger it and verify the toward-limit PWM becomes zero; then command the opposite direction and verify movement-away PWM is permitted.

Optional commands `IBT2_BENCH_FORWARD` and `IBT2_BENCH_REVERSE` run a bounded one-second 64/255 output test only when `ENABLE_IBT2_BENCH_TEST` is deliberately enabled. It is disabled by default and never runs at boot.

## 14. Thursday Motor Test Procedure

1. Leave mechanical linkage disconnected.
2. Test new motor directly from fused 12 V briefly.
3. Reverse battery polarity and verify opposite rotation.
4. Record no-load current.
5. Connect motor to IBT-2 M+ / M-.
6. Start at approximately 25% PWM.
7. Test forward.
8. STOP.
9. Test reverse.
10. STOP.
11. Verify limit-switch response.
12. Only then connect the mechanical linkage.
13. Test upper plate unloaded.
14. Gradually increase speed as necessary.
15. Mount/test thrower only after unloaded motion is reliable.

## 15. Files Changed

`src/config.h`, `src/storage.cpp`, `src/inputs.h`, `src/inputs.cpp`, `src/ble_control.h`, `src/ble_control.cpp`, `src/main.cpp`, `src/motor.cpp`, and this document.

## 16. Build/Test Results

PlatformIO `esp32dev` release build: PASS. Arduino-ESP32 2.0.17 used the correct channel-based LEDC API. RAM 41,172/327,680 bytes (12.6%); flash 1,195,589/1,310,720 bytes (91.2%). Repository static search found distinct motor channels 0/1 and RGB channels 2/3/4, no GPIO duplication, and no motor-pin writes outside `motor.cpp`. `git diff --check`: PASS.

## 17. Remaining Hardware Checks

Confirm both limit module OUT pins measure a stable HIGH when clear and LOW when pressed; add external 3.3 V pull-ups if the module cannot guarantee this. Confirm L_EN rises with R_EN, verify real PWM with a scope if available, confirm commanded RIGHT corresponds to desired physical travel, and retain fused motor power. E-stop is not assigned on this Alpha wiring and must not be represented as installed.

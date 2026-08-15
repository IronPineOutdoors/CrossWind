# Bench Test Checklist

## ESP32 Power Test

- Power the ESP32 from the 5V buck converter.
- Open Serial Monitor at 115200.
- Confirm startup diagnostics print.

## Switch Test

- Confirm Red-to-Black measures approximately 3.3 V with controller power on.
- Confirm Green (Left OUT) and Blue (Right OUT) each read HIGH normally and LOW when pressed.
- With power removed, continuity-test Green and Blue end to end. Do not treat a disconnected live input as valid because GPIO34/GPIO35 have no internal pull-ups and will float.
- Trigger the left roller switch and confirm Serial status changes.
- Trigger the right roller switch and confirm Serial status changes.
- Trigger both and confirm a fault is reported.
- Turn the rotary encoder and confirm OLED motor percentage and Serial speed output change.
- Cold-cycle controller power with Serial Monitor closed and confirm the SSD1309 starts through the five-conductor lid harness.
- Press the encoder switch and confirm it opens the menu; verify Motor, Mode, Environment, Diagnostics, and About navigation.
- Press FIRE while OLED shows `SAFE` and confirm Serial prints `FIRE BLOCKED - NOT ARMED`.
- Press ARM and confirm OLED shows `ARMED` and Serial prints `ARM ON`.
- Press FIRE while armed and confirm the relay pulses briefly.
- Press ARM again and confirm OLED returns to `SAFE`.

## Motor Driver Test

- Disconnect the thrower from the rotating plate.
- Verify BTS7960 logic ground and ESP32 ground are common.
- Power the motor branch through a fuse.

## Low PWM Motor Test

- Set speed low.
- Send BLE `START` or use the current bench start mechanism.
- Confirm the wiper motor ramps rather than jerks.

## Sweep Mode No-Load Test

- Let the plate sweep with no thrower mounted.
- Confirm each endpoint stops the current output, observes reversal dead time, moves away, and repeats without a normal endpoint fault.
- Hold a departed switch active beyond `LIMIT_DWELL_MS` and confirm `FAULT_LIMIT`.
- Prevent the next endpoint from activating until `MAX_TRAVEL_TIME_MS` and confirm `FAULT_TRAVEL_TIMEOUT`.
- Activate both limits and confirm `FAULT_BOTH_LIMITS`.
- Run `CENTERING` and confirm a full measured traverse, half-time return, and automatic stop.

## Thrower-Mounted Test

- Mount the thrower only after no-load testing.
- Start at low PWM.
- Watch for tipping, binding, or switch bracket movement.

# Bench Test Checklist

## ESP32 Power Test

- Power the ESP32 from the 5V buck converter.
- Open Serial Monitor at 115200.
- Confirm startup diagnostics print.

## Switch Test

- Trigger the left roller switch and confirm Serial status changes.
- Trigger the right roller switch and confirm Serial status changes.
- Trigger both and confirm a fault is reported.
- Turn the rotary encoder and confirm OLED motor percentage and Serial speed output change.
- Press the encoder switch and confirm OLED menu toggles between `MAIN` and `SETUP`.
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
- Confirm right limit stop, dwell, left movement, left limit stop, dwell, repeat.

## Thrower-Mounted Test

- Mount the thrower only after no-load testing.
- Start at low PWM.
- Watch for tipping, binding, or switch bracket movement.

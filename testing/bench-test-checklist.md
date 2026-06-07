# Bench Test Checklist

## ESP32 Power Test

- Power the ESP32 from the 5V buck converter.
- Open Serial Monitor at 115200.
- Confirm startup diagnostics print.

## Switch Test

- Trigger the left roller switch and confirm Serial status changes.
- Trigger the right roller switch and confirm Serial status changes.
- Trigger both and confirm a fault is reported.

## Motor Driver Test

- Disconnect the thrower from the rotating plate.
- Verify BTS7960 logic ground and ESP32 ground are common.
- Power the motor branch through a fuse.

## Low PWM Motor Test

- Set speed low.
- Press start.
- Confirm the wiper motor ramps rather than jerks.

## Sweep Mode No-Load Test

- Let the plate sweep with no thrower mounted.
- Confirm right limit stop, dwell, left movement, left limit stop, dwell, repeat.

## Thrower-Mounted Test

- Mount the thrower only after no-load testing.
- Start at low PWM.
- Watch for tipping, binding, or switch bracket movement.

# Bench Test Checklist

## ESP32 Power Test

- Power the ESP32 from the 5V buck converter.
- Open Serial Monitor at 115200.
- Confirm startup diagnostics print.

## Switch Test

- Trigger the left roller switch and confirm Serial status changes.
- Trigger the right roller switch and confirm Serial status changes.
- Trigger both and confirm a fault is reported.
- Turn the rotary encoder on Status and confirm speed changes in bounded steps.
- Short-press the encoder and confirm menu entry/selection.
- Hold the encoder for at least `UI_LONG_PRESS_MS` and confirm cancel/back without saving.
- Confirm ARM and FIRE remain responsive from every menu page.
- Confirm an active fault immediately replaces the current page and unsafe clear remains blocked.
- Press FIRE while OLED shows `SAFE` and confirm Serial prints `FIRE BLOCKED - NOT ARMED`.
- Press ARM and confirm OLED shows `ARMED` and Serial prints `ARM ON`.
- Press FIRE while armed and confirm the relay pulses briefly.
- Press ARM again and confirm OLED returns to `SAFE`.

## Motor Driver Test

- Disconnect the thrower from the rotating plate.
- Verify BTS7960 logic ground and ESP32 ground are common.
- Power the motor branch through a fuse.

## Battery Monitor Test

- Keep monitoring disabled until the external divider is installed and verified.
- With GPIO36 disconnected, verify the monitor does not report a safe valid voltage.
- Verify divider midpoint voltage with a multimeter before connecting the ESP32; it must remain below 3.3 V.
- Calibrate at a stable source voltage with motor power off.
- With the thrower removed, record idle voltage, motor-start sag, minimum running voltage, and recovery.
- Confirm a brief sag is filtered/debounced while sustained critical voltage stops motion and latches the configured battery fault.

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

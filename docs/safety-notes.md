# Safety Notes

- Keep hands clear of the rotating top plate, motor linkage, roller switch striker, and thrower arm.
- Always bench test without the thrower mounted after firmware or wiring changes.
- Use a main fuse near the battery.
- Use branch fuses for the thrower, wobbler motor, actuator, and 5V electronics supply.
- Verify the ESP32 and motor driver share ground.
- Do not feed more than 3.3V into ESP32 analog or GPIO pins.
- Add a reachable power disconnect before field testing.
- Secure the base before powered motion.
- Treat stored faults as useful evidence; investigate before clearing and retesting.

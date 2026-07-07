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
- Treat trigger testing like live thrower testing: unload the thrower, point it safely, and keep people clear.
- Confirm the VEVOR NH113 pedal wires with a continuity test before wiring Crosswind in parallel.
- The ESP32 must not send voltage into the thrower pedal circuit; use relay contacts as dry contact only.
- Crosswind Alpha uses a DHT11 only as an enclosure temperature/humidity indicator. Treat it as low-precision telemetry, not a certified safety instrument.
- Keep the DHT11 away from the BTS7960 heat sink and motor driver. Do not mount it directly against the case wall, where sun or surface heating can distort readings.
- Temperature faults stop the motor only at `TEMP_FAULT_F`; investigate enclosure airflow, motor driver heating, and wiring before clearing a temperature fault.
- YL-99 limit switches are safety/calibration inputs only. Do not use the switch roller arms as mechanical hard stops.
- Mount separate rubber or metal hard stops if the rotating plate needs physical travel limits independent of firmware.
- A limit fault must stop motor output, disarm the system, block relay firing, and remain latched until both switches are released.

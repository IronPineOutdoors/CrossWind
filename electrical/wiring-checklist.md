# Wiring Checklist

- Battery positive has a main fuse near the battery.
- BTS7960 motor power branch is fused.
- Buck converter branch is fused.
- ESP32/Arduino ground and BTS7960 ground are common.
- ESP32 analog input never sees more than 3.3V.
- Limit switches read correctly in Serial diagnostics.
- Thrower pedal black/white wires are confirmed with a continuity test before relay wiring.
- Trigger relay COM and NO are wired in parallel with the factory pedal, not in series.
- Trigger relay contacts are dry contact only; no ESP32 voltage is sent into the pedal circuit.
- Factory pedal still works with Crosswind powered off.
- Motor wires are secured and strain relieved.
- Enclosure cable glands are tightened.
- A power disconnect is reachable during testing.

# Limit Switch Wiring

The canonical cable color standard, Deutsch DT 6-pin pinout, connector drawing, wiring diagram, and assembly procedure are in [Limit Switch Harness Standard](limit-switch-harness.md).

Crosswind uses normally closed limit-switch contacts. An intact, unactuated switch connects its colored signal conductor to the shared Black ground and reads LOW through the controller input. Actuating the switch or opening/breaking a wire produces an open circuit and must read HIGH through a 3.3 V pull-up.

For ESP32 Phase 1:

- Green / Deutsch pin 1 carries the Left limit signal to GPIO34.
- Blue / Deutsch pin 2 carries the Right limit signal to GPIO35.
- Black / Deutsch pin 5 is the shared switch ground.
- White / pin 3, Yellow / pin 4, and Red / pin 6 are terminated but electrically unused during Alpha.
- GPIO34/GPIO35 are input-only pins without internal pull-ups and therefore require external 3.3 V pull-up resistors. This is the Alpha exception to the preferred internal-pull-up design.
- Limit switches are safety/calibration inputs only. They are not normal travel controls and must not be used as physical hard stops.

Check that normal reads LOW and both actuation and a disconnected wire read HIGH in Serial diagnostics before connecting motor power.

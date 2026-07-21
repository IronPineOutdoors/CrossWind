# Limit Switch Wiring

The canonical cable color standard, Deutsch DT 6-pin pinout, connector drawing, wiring diagram, and assembly procedure are in [Limit Switch Harness Standard](limit-switch-harness.md).

Crosswind uses powered YL-99 limit switch modules. Each OUT signal is HIGH normally; pressing the switch shorts OUT to Black ground, producing LOW/active. Red supplies regulated 3.3 V to the modules and Black is their shared ground.

For ESP32 Phase 1:

- Green / Deutsch pin 1 carries the Left limit signal to GPIO34.
- Blue / Deutsch pin 2 carries the Right limit signal to GPIO35.
- Black / Deutsch pin 5 is the shared switch ground.
- Red / Deutsch pin 6 supplies regulated ESP32 3.3 V to the modules.
- White / pin 3 and Yellow / pin 4 are terminated but electrically unused during Alpha.
- GPIO34/GPIO35 are input-only pins without internal pull-ups and therefore require external 3.3 V pull-up resistors. This is the Alpha exception to the preferred internal-pull-up design.
- Limit switches are safety/calibration inputs only. They are not normal travel controls and must not be used as physical hard stops.

Check that normal reads HIGH/clear and pressing either switch reads LOW/active in Serial diagnostics before connecting motor power. A disconnected OUT wire reads HIGH/clear and is not automatically detected by this powered active-LOW arrangement.

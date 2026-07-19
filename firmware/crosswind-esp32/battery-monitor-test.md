# Battery Monitor Simulation and Bench Test

No physical validation is recorded here. Build deterministic simulation with motor GPIO disabled:

```sh
pwsh -File scripts/test-battery-monitor.ps1
pio run -e esp32dev_battery_sim
pio run -e esp32dev_battery_sim -t upload -t monitor
```

Enable monitoring with `BATTERY_MONITOR=ON`. Set simulated voltage using `SIM_BATTERY=<volts>` or use `SIM_BATTERY=INVALID`. Observe `BATTERY_STATUS` or the normal status payload.

1. Set 20.0 V and verify UNKNOWN during settling, then NORMAL.
2. Confirm startup needs the configured settling time and minimum samples.
3. Set 16.0 V long enough to confirm LOW and one warning event.
4. Raise to 17.5 V and verify stable RECOVERING/NORMAL hysteresis.
5. Set 14.5 V for the confirmation interval and verify BATTERY_CRITICAL.
6. Reset, briefly set 14.5 V for fewer than the configured critical samples, then restore 20.0 V; verify no critical event.
7. Hold 14.5 V and verify sustained sag does trigger the critical event/application fault.
8. Send `SIM_BATTERY=INVALID`; verify invalid status never reports a safe valid voltage and sustained invalid input reaches SENSOR_ERROR.
9. Set `BATTERY_MONITOR=OFF`; verify DISABLED and no battery fault.
10. At a stable simulated voltage, submit an in-range known voltage and verify the multiplier; reject calibration outside configured multiplier/known-voltage limits.
11. If approximate percentage is compiled on, test below/above the lookup endpoints and verify 0–100 clamping; verify other profiles return unknown.
12. Confirm Preferences writes occur only for enable/profile/calibration commands, never periodic samples.
13. Confirm BLE contains `batteryV`, status, validity, profile, percentage, raw/filtered ADC, calibration, and counters.
14. Confirm the OLED shows OFF, UNKNOWN, voltage/status, LOW, CRITICAL, and SENSOR_ERROR appropriately.

## Physical bench procedure

With motor and thrower disconnected, build the 100 kOhm/12 kOhm divider from the selected measurement point to common ground, with the midpoint to GPIO36. A small capacitor across the lower resistor may be evaluated for noise only after confirming response time. Measure midpoint voltage with a multimeter before attaching the ESP32 and ensure it remains below 3.3 V at the maximum possible supply voltage.

Calibrate against the source voltage measured at the same time. Then test under motor load with the thrower removed, recording idle voltage, startup sag, minimum running voltage, and recovery. Do not connect raw battery or 12 V bus voltage directly to GPIO36.

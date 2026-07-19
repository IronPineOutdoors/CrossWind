# Display/Menu Deterministic Test

Build the UI simulation with motor outputs disabled and virtual motion/battery inputs:

```sh
pwsh -File scripts/test-display-menu.ps1
pio run -e esp32dev_ui_sim
pio run -e esp32dev_ui_sim -t upload -t monitor
```

Use `SIM_UI=NEXT|PREV|PRESS|HOLD`, `SIM_LIMITS`, and `SIM_BATTERY` over BLE. Physical encoder tests remain required because BLE simulation does not validate detent polarity or switch feel.

1. Boot and verify safety processing continues while branding is visible.
2. Verify Status shows current ARM/run/mode/motion/speed/battery/warning/BLE data.
3. Press, rotate through all top-level items, and verify selection visibility.
4. Verify short press enters/selects/confirms.
5. Verify long press backs out and cancels edits.
6. Rotate every numeric edit beyond both ends and verify bounds.
7. Cancel an edit, reboot, and verify it was not saved.
8. Confirm then Save once; reboot and verify contrast/timeout persistence.
9. Start motion and verify Mode changes show STOP required.
10. Open any menu, then test physical ARM and FIRE responsiveness and all existing gates.
11. Inject `SIM_LIMITS=BOTH` and verify Fault overrides every page.
12. Attempt clear with limits active and verify the fault remains with a blocked reason.
13. Wait the configured menu timeout and verify return to Status.
14. Wait for OLED off and verify status/motion/BLE processing continues.
15. Inject low battery or a fault and verify immediate wake/priority display.
16. Change mode/speed through BLE and verify local notification/current display.
17. Change mode/speed locally and verify the normal BLE status payload updates.
18. Select Center and Motion calibration; verify confirmation is mandatory.
19. Verify CENTERING and all calibration pages contain no trigger action.
20. Select Restore Defaults; verify confirmation and stopped-state enforcement.
21. Rotate repeatedly without Save and verify Preferences receives no writes.
22. Run the source-check script and verify no menu/display `delay()` calls.

Physical usability testing must cover encoder direction and missed detents, long-press comfort, daylight/night visibility, viewing angle, glove use, notification readability, and enclosure ergonomics. No such validation is claimed here.

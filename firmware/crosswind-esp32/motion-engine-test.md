# Motion Engine Simulation and Bench Test

No hardware validation is recorded by this document. Perform physical testing with the thrower removed, the relay disconnected, and initially the motor linkage disconnected.

## Simulation build

```sh
pwsh -File scripts/test-motion-engine.ps1
pio run -e esp32dev_motion_sim
pio run -e esp32dev_motion_sim -t upload -t monitor
```

`CROSSWIND_MOTION_SIMULATION=1` disables BTS7960 GPIO writes, starts virtual position at mid-travel, advances it from commanded direction/PWM, exposes virtual endpoints, uses seed `1` when no fixed seed is configured, and logs state transitions without per-loop logging.

## Deterministic sequence

Use BLE commands and inspect `motionState`, limits, PWM, counters, centered, and travel time in `STATUS` or `MOTION_STATUS`:

1. Select SWEEP and START; verify a safe direction is commanded.
2. Reach the right endpoint; verify stop, dwell, and left reversal.
3. Reach the left endpoint; verify stop, dwell, and right reversal.
4. Override the target endpoint active before a direction request; verify motion cannot drive farther into it.
5. Override both limits active; verify a latched BOTH_LIMITS fault.
6. Start away from one overridden endpoint and hold it active; verify LIMIT_STUCK after the release timeout.
7. Override both limits inactive during continuous travel; verify TRAVEL_TIMEOUT.
8. Send STOP during movement; verify IDLE, zero PWM, and no sequence restart.
9. Run RANDOM and verify PWM, movement time, and dwell remain inside configured ranges.
10. Let RANDOM reach both virtual boundaries; verify neither endpoint is ignored.
11. Run CENTERING; verify no trigger-request event occurs.
12. Run FLUSH unarmed and with automatic triggering disabled; verify no trigger. If explicitly enabled, verify the existing ARM and trigger-interval gates still decide the request.
13. Run CALIBRATE with valid endpoints, then an interrupted/out-of-range attempt; verify only the valid completion changes persisted `travelMs`.
14. Set `MOTION_FIXED_RANDOM_SEED` nonzero and repeat RANDOM from the same reset state; verify the sequence repeats.
15. Run the source-check script; verify `motion.cpp` and `modes.cpp` contain no blocking `delay()`.

## Fault injection and physical bench checks

The virtual position covers normal endpoints. In the simulation build, use `SIM_LIMITS=AUTO|NONE|LEFT|RIGHT|BOTH` to override virtual inputs deterministically; return to `AUTO` after each case:

- Send `SIM_LIMITS=BOTH`: expect BOTH_LIMITS.
- Override the commanded target endpoint active: expect UNEXPECTED_LIMIT.
- Start away from one overridden endpoint and keep it active beyond `LIMIT_RELEASE_TIMEOUT_MS`: expect LIMIT_STUCK.
- Send `SIM_LIMITS=NONE` during travel and wait beyond `MAX_TRAVEL_TIME_MS`: expect TRAVEL_TIMEOUT.
- Confirm STOP and every fault immediately disable output and cancel the relay.
- Confirm no `delay()` exists in `motion.cpp` or `modes.cpp`.

Before a powered no-load test, verify direction naming, switch polarity, separate hard stops, clear linkage, common grounds, and fused motor power. Then test at low PWM with the thrower removed.

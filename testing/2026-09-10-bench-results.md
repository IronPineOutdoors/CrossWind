# Motor bench results and field-test preparation — 2026-09-10

User-reported observations; not independently witnessed. Flashed firmware revision,
PWM, supply voltage, runtime and current measurements were not supplied.

## Bench observations

- Motor system ran and alternated direction when a limit was actuated.
- Physical limit identities appear reversed: the opposite switch produces the
  expected response. Possible ESP32 reassembly wiring mismatch, not yet confirmed.
- Both switches active produces a fault. Holding a limit active produces a fault.
- Throwing operation is already known to work; full mode validation remains open.
- Threaded rod may be backing out of the motor, or flange set screws may be
  slipping on the rod threads. These are separate possible failures.
- Feet still need mounting.
- Logic and voltage plate spacers are printing; installation remains pending.

## Mechanical closeout

- Mark rod-to-motor and rod-to-flange interfaces separately to identify relative
  movement during an unloaded, low-speed test. Stop if either slips.
- Confirm rod thread diameter/pitch, engagement and the actual rotating motor
  output construction before selecting a jam-nut arrangement or changing length.
- A longer rod and top nut only clamp the flange if there is an opposing solid
  shoulder/nut and a suitable load path. A top nut alone does not establish torque
  retention or stop the rod unscrewing from the motor. Do not clamp against the
  stationary gearbox housing or add unintended bearing preload.
- Medium-strength removable threadlocker such as LOCTITE 243 is a candidate for
  correctly fitted metal threads; follow its preparation and cure instructions.
  Threadlocker on set-screw threads prevents loosening but does not repair poor
  shaft contact or establish the coupling's torque capacity.
- Mount feet; install printed spacers and plates; secure harnesses clear of all
  moving parts. Confirm independent mechanical stops and accessible power cutoff.

## Restore limit identity before powered endpoint testing

1. With motor power disconnected, press the physical left switch: diagnostics
   must show LEFT only; Green / Deutsch 1 must reach GPIO34.
2. Press right: RIGHT only; Blue / Deutsch 2 must reach GPIO35.
3. Correct only an established mismatch. If input identities are correct but
   commanded direction is physically reversed, investigate motor direction mapping
   instead of swapping correctly identified limit signals.
4. With the thrower unloaded, verify low-speed movement toward each endpoint
   reverses away from that same endpoint; starting on either limit moves away.

## Mode verification against saved source

Source: `firmware/crosswind-esp32/src/modes.cpp`, `config.h`, and firmware README.
Saved source is not proof of the revision flashed during this test.

| Mode | Current implementation | Remaining verification |
| --- | --- | --- |
| SWEEP | Bounded travel, stop, reversal dead time, move away, repeat | Both physical endpoints; repeated runs without slippage or normal endpoint faults |
| RANDOM | Same bounded sweep routine as SWEEP; placeholder | Confirm desired distinct behavior before claiming Random feature completion |
| FLUSH | Same bounded sweep routine as SWEEP; placeholder | Confirm desired movement and trigger behavior before claiming Flush feature completion |
| CENTERING | Find endpoint if needed, measure full traverse, return for half measured time, stop | Start mid-travel and at either endpoint; check practical center accuracy at fixed speed |

Centering estimates center by time, not a position sensor. Keep speed unchanged
during its measurement and return. Automatic triggering is documented as disabled
by default; mode selection alone does not establish automatic throw operation.

Saved constants: reversal dead time 50 ms, switch-release timeout 1000 ms,
maximum travel time 30000 ms. The release timeout is a fault threshold, not an
intentional one-second pause at each endpoint. Bench observations support the
fault paths but do not measure their timing.

Before field operation, verify STOP, disarmed FIRE blocking, armed relay pulse,
both-limit fault, stuck-limit fault, travel timeout and intentional fault recovery.
Simulate missing/stuck inputs only in an unloaded setup that cannot drive into a
hard stop. Do not hold a powered mechanism stalled to test a timeout.

Field readiness remains pending shaft retention, corrected direction/limit mapping,
feet and spacer installation, and documented mode/control checks. Initial field
work should begin with unloaded motion before live throwing trials. Random and
Flush remain explicitly unvalidated as distinct modes.

## Thread-retention references

- [Henkel LOCTITE 243 product information](https://next.henkel-adhesives.com/us/en/products/industrial-adhesives/central-pdp.html/loctite-243/BP000000316211.html)
- [Ruland: reasons why collars fail](https://www.ruland.com/media/wysiwyg/PDF/reason-why-collars-fail.pdf)

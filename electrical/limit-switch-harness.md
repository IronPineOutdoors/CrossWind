# Limit Switch Harness Standard

This document is the source of truth for the Crosswind limit switch cable, connector, and conductor colors. The standard supports the current single-axis Alpha assembly and the future dual-axis assembly without replacing or rewiring the harness.

## Cable and Connector

- Cable: 6-conductor
- Connector: Deutsch DT 6-pin
- Switch contacts: normally closed (NC)
- Common return: Black shared ground
- Signal bias: pull-up to 3.3 V
- Normal, intact circuit: LOW
- Actuated switch or open/broken wire: HIGH

## Pinout

| Deutsch DT pin | Conductor | Function | Alpha use |
| ---: | --- | --- | --- |
| 1 | Green | Left limit switch | Connected |
| 2 | Blue | Right limit switch | Connected |
| 3 | White | Lower / Axis 2 Down limit switch | Terminated, unused |
| 4 | Yellow | Upper / Axis 2 Up limit switch | Terminated, unused |
| 5 | Black | Shared ground for all limit switches | Connected |
| 6 | Red | Spare / future expansion | Terminated, reserved, unused |

Do not use the Red conductor as power or ground. Keep it isolated at both ends until a future design formally assigns it.

## Connector Diagram

The following is the mating-face view with the latch at the top. Always confirm the molded cavity numbers on the actual Deutsch housing before crimping; housing illustrations can be mirrored when viewed from the wire side.

```text
                    LATCH
              +-----------------+
              |  1     2     3  |
              | GRN   BLU   WHT  |
              | Left  Right Lower|
              |                 |
              |  4     5     6  |
              | YEL   BLK   RED  |
              | Upper GND  Spare |
              +-----------------+
                 MATING FACE
```

## Switch Wiring Diagram

Each NC switch connects its colored signal conductor to the shared Black ground conductor when the switch is not actuated.

```text
ESP32/input side                         Limit-switch side

3.3 V -- pull-up -- Green  (pin 1) -----o---o-----+
3.3 V -- pull-up -- Blue   (pin 2) -----o---o-----+
3.3 V -- pull-up -- White  (pin 3) -----o---o-----+-- Black (pin 5) -- GND
3.3 V -- pull-up -- Yellow (pin 4) -----o---o-----+

Red (pin 6) ---------------- terminated and isolated; no connection

o---o = normally closed contact shown in its normal, unactuated state
```

| Condition | Contact/cable state | ESP32 input |
| --- | --- | --- |
| Normal, switch not actuated | Closed to Black ground | LOW |
| Limit switch actuated | Open | HIGH |
| Signal conductor broken or disconnected | Open | HIGH |
| Shared Black ground broken | Open for affected/all switches | HIGH |

This fail-safe arrangement intentionally makes an actuated switch and an open wire equivalent. Either condition must stop or inhibit motion according to the firmware safety rules.

## ESP32 Pull-up Compatibility

The harness standard assumes each limit input is pulled up to 3.3 V. Use an ESP32 internal pull-up when the assigned GPIO supports one.

The current Alpha GPIO assignments remain GPIO34 for Left and GPIO35 for Right. On the classic ESP32, GPIO34 and GPIO35 do not have internal pull-up resistors, so the Alpha controller must retain external 3.3 V pull-up resistors on those two inputs. Moving to internal pull-ups would require different GPIO assignments and is outside this wiring-document update.

Before powered motion, confirm the installed firmware reports LOW for each intact, unactuated NC circuit and HIGH for both switch actuation and a disconnected conductor. A configuration that treats LOW as the activated state is not compatible with this finalized NC harness behavior.

## Alpha Assembly

During Alpha, connect only:

- Green, pin 1: Left limit
- Blue, pin 2: Right limit
- Black, pin 5: shared ground

Crimp, seal, and terminate White, Yellow, and Red in their assigned connector cavities, but leave them electrically unused. Insulate and secure their equipment-side ends separately so they cannot contact ground, power, or one another.

## Harness Assembly Procedure

1. Label both cable ends and orient each Deutsch DT housing by its molded cavity numbers.
2. Crimp the six conductors to the correct contacts and install the connector seals and wedge locks.
3. Connect one terminal of every NC limit switch to its assigned colored conductor.
4. Join the other terminal of every limit switch to the Black shared-ground conductor.
5. Leave the Red spare isolated. During Alpha, also isolate the equipment-side White and Yellow conductors.
6. Add strain relief and route the cable clear of bearings, linkages, crank arms, rotating plates, and pinch points.
7. With power disconnected, verify pin-to-pin continuity and confirm there are no shorts between adjacent conductors.
8. Confirm each installed switch is closed to Black when unactuated and opens when actuated.
9. With controller power only, confirm each input is LOW normally and HIGH when its switch is actuated or its signal conductor is disconnected.
10. Do not enable motor power until all continuity and input-state checks pass.

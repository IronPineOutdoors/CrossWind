# Limit Switch Harness Standard

This document is the source of truth for the Crosswind limit switch cable, connector, and conductor colors. The standard supports the current single-axis Alpha assembly and the future dual-axis assembly without replacing or rewiring the harness.

## Cable and Connector

- Cable: 6-conductor
- Connector: Deutsch DT 6-pin
- Switch type: powered YL-99 limit switch module
- Module supply: Red 3.3 V and Black shared ground
- Normal, unactuated OUT: HIGH
- Actuated OUT: LOW (module shorts OUT to ground)

## Pinout

| Deutsch DT pin | Conductor | Function | Alpha use |
| ---: | --- | --- | --- |
| 1 | Green | Left limit switch | Connected |
| 2 | Blue | Right limit switch | Connected |
| 3 | White | Lower / Axis 2 Down limit switch | Terminated, unused |
| 4 | Yellow | Upper / Axis 2 Up limit switch | Terminated, unused |
| 5 | Black | Shared ground for all limit switch modules | Connected |
| 6 | Red | Shared 3.3 V supply for all limit switch modules | Connected |

Red is dedicated to the regulated ESP32 3.3 V rail for the limit switch modules. Do not connect it to 5 V, battery voltage, or another supply.

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
              | Upper GND  3V3   |
              +-----------------+
                 MATING FACE
```

## Switch Wiring Diagram

Each powered limit module receives 3.3 V on Red and ground on Black. Its onboard pull-up holds the assigned OUT signal HIGH while clear, and the switch shorts OUT to Black ground when pressed.

```text
ESP32/input side                         Limit-switch side

ESP32 GPIO34 <--- Green  (pin 1) <--- Left module OUT
ESP32 GPIO35 <--- Blue   (pin 2) <--- Right module OUT
Future input <--- White  (pin 3) <--- Lower module OUT
Future input <--- Yellow (pin 4) <--- Upper module OUT

ESP32 GND ----> Black (pin 5) ----+---- module GND (all modules)
ESP32 3.3 V --> Red   (pin 6) ----+---- module VCC (all modules)

Each module provides its onboard 3.3 V pull-up. A pressed module shorts OUT to GND.
```

| Condition | Contact/cable state | ESP32 input |
| --- | --- | --- |
| Normal, switch not actuated | Module OUT powered HIGH | HIGH / clear |
| Limit switch actuated | Module shorts OUT to Black ground | LOW / active |
| Signal conductor broken or disconnected | GPIO34/GPIO35 has no internal pull-up | Undefined; treat as a wiring fault during inspection |
| Red 3.3 V or Black ground broken | Module state is not trustworthy | Treat as a wiring fault during inspection |

This powered active-LOW module arrangement does **not** provide open-wire fault detection: a broken OUT wire leaves GPIO34/GPIO35 electrically undefined. Inspect and continuity-test the harness before operation. Additional controller-side bias and end-of-line monitoring or different hardware would be required for automatic broken-wire detection.

## ESP32 Pull-up Compatibility

The current Alpha GPIO assignments remain GPIO34 for Left and GPIO35 for Right. Those pins do not have internal pull-ups, so Alpha relies on the installed YL-99 modules' onboard pull-ups. Bench measurements verified approximately 3.2 V/HIGH while released and 0 V/LOW while pressed; no additional external resistors are required for the installed modules.

Before powered motion, confirm the installed firmware reports HIGH/clear for each unactuated module and LOW/active when pressed. The firmware must use `LIMIT_ACTIVE_STATE = LOW`.

## Alpha Assembly

During Alpha, connect only:

- Green, pin 1: Left limit
- Blue, pin 2: Right limit
- Black, pin 5: shared ground
- Red, pin 6: shared regulated 3.3 V

Crimp, seal, and terminate White and Yellow in their assigned connector cavities, but leave them electrically unused. Insulate and secure their equipment-side ends separately so they cannot contact ground, power, or one another.

## Harness Assembly Procedure

1. Label both cable ends and orient each Deutsch DT housing by its molded cavity numbers.
2. Crimp the six conductors to the correct contacts and install the connector seals and wedge locks.
3. Connect every module `OUT` to its assigned Green, Blue, White, or Yellow conductor.
4. Connect every module `GND` to Black and every module `VCC` to Red.
5. Connect Red only to regulated ESP32 3.3 V. During Alpha, isolate the equipment-side White and Yellow conductors.
6. Add strain relief and route the cable clear of bearings, linkages, crank arms, rotating plates, and pinch points.
7. With power disconnected, verify pin-to-pin continuity and confirm there are no shorts between adjacent conductors.
8. With controller power only, confirm Red-to-Black measures approximately 3.3 V at the harness and each module powers normally.
9. Confirm each OUT input is HIGH normally and LOW when its switch is pressed. With power removed, continuity-test each OUT conductor; do not use a disconnected live input as a valid state because GPIO34/GPIO35 will float.
10. Do not enable motor power until all continuity and input-state checks pass.

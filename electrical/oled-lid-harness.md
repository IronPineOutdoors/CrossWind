# OLED Lid Harness Standard

This document is the source of truth for the Crosswind Alpha OLED lid harness. The verified production-standard display is the Hosyond 2.42-inch 128x64 monochrome OLED with SSD1309 controller and five electrical connections.

## Cable and Connector

- Harness: 5-conductor lid harness
- Connector: 5-position JST connector
- Connector family and pitch: match the installed Alpha JST mating pair; record the exact manufacturer series before ordering replacements
- Display supply: regulated ESP32 3.3 V
- Logic: 3.3 V I2C
- Display address: normally `0x3C`; firmware also diagnoses `0x3D`

The `RES` conductor is part of the official harness and is required for firmware-controlled display reset and reliable cold-power startup.

## Connector Pinout

Pin numbers are viewed from the mating face of the controller-side connector with the latch/key in its normal top orientation. Verify the molded cavity order for the selected JST series before crimping.

| Pin | Signal | ESP32 connection | OLED connection | Notes |
| ---: | --- | --- | --- | --- |
| 1 | GND | System ground | `GND` | Shared logic ground |
| 2 | +3.3 V | ESP32 `3V3` | `VCC` | Do not use 5 V in Crosswind |
| 3 | OLED SCL | GPIO22 | `SCL` | Shared I2C bus |
| 4 | OLED SDA | GPIO21 | `SDA` | Shared I2C bus |
| 5 | OLED RESET | GPIO5 | `RES` | Active LOW; firmware controlled |

Conductor colors are not standardized until the installed cable colors are recorded. Label both ends by signal and cavity number; never infer function from color alone.

## Wiring Diagram

```text
ESP32 / controller                 JST 5-pin lid harness                 SSD1309 OLED

System GND  --------------------------- pin 1 -------------------------- GND
3V3         --------------------------- pin 2 -------------------------- VCC
GPIO22 SCL  --------------------------- pin 3 -------------------------- SCL
GPIO21 SDA  --------------------------- pin 4 -------------------------- SDA
GPIO5 RES   --------------------------- pin 5 -------------------------- RES
```

The BME280 remains on the controller side of the shared GPIO21/GPIO22 I2C bus. Adding OLED reset does not change the BME280 or any encoder, button, motor, relay, or limit-switch wiring.

## Harness BOM

| Qty | Item | Requirement |
| ---: | --- | --- |
| 1 | Five-position JST plug housing | Must mate with installed controller/lid connector |
| 1 | Five-position JST receptacle housing | Matching series and pitch |
| 10 | Crimp contacts | Correct contacts and wire range for selected JST series |
| 1 | Five-conductor cable or five discrete conductors | Flexible stranded wire suitable for lid movement |
| 2 | Harness labels | Identify connector, pin numbers, and signal names |
| As required | Sleeving and strain relief | Protect conductors at lid and enclosure transitions |

Record the exact JST series, pitch, housing part numbers, contact part numbers, conductor gauge, cable colors, and finished length after measuring the installed Alpha hardware. Do not order by the generic term “JST” alone.

## Fabrication Procedure

1. Cut five conductors to the measured lid-routing length, including service loop and strain-relief allowance.
2. Label both ends `GND`, `3V3`, `SDA`, `SCL`, and `RES`.
3. Crimp contacts with the tooling specified for the selected JST series and wire gauge.
4. Insert each contact into its assigned cavity and perform a light pull test.
5. Verify cavity numbering from the mating face before joining the connector halves.
6. Perform point-to-point continuity testing for all five conductors.
7. Confirm no conductor is shorted to an adjacent cavity.
8. With the OLED disconnected, confirm pin 2 measures approximately 3.3 V relative to pin 1.
9. Connect the OLED and confirm the Serial I2C scan reports it, normally at `0x3C`.
10. Cold-cycle controller power without opening Serial Monitor and confirm the startup test and normal interface appear.

## Acceptance Criteria

- All five signals pass continuity testing.
- No pin-to-pin shorts are present.
- OLED power is 3.3 V.
- GPIO5 reaches the OLED `RES` input.
- Firmware reports `SSD1309 display ready` at the detected I2C address.
- The display starts on a cold power-up and continues to operate while the BME280 shares the bus.

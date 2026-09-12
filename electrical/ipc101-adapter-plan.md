# IPC-101 panel adapter and connector development

Status: development specification, 2026-09-12. User selected IPC-101 as the panel
for Alpha and future IPC-100. User expects the PCB around September 14-16;
arrival estimate is not independently verified. No harness has been electrically
validated and no IPC-101 firmware support has been installed by this change.

## Architecture

IPC-101 panel -> printed connector carrier -> replaceable equipment adapter
-> Alpha ESP32 now, IPC-100 later.

Use one measured four-position housing for controls (K) and one measured
six-position housing for OLED (D). The previously considered second six-position
connection is unnecessary for these functions and is omitted. These are new
IPC-101 adapter assignments, not a reassignment of the existing Alpha harness.
Keep the original harness intact and label the new assembly IPC101.

STOP uses its own distinctive two-position connector, physically incompatible
with K and D. Neither STOP conductor is logic ground. IPC-101 J3 is an isolated
pass-through, not a keypad input.

## Circuit assignment draft

K1-K4 and D1-D6 below are logical cavity labels, not a released crimp-face drawing.
Mark and verify cavity orientation on the actual housings and continuity through
the middle header before releasing a harness drawing; mating faces can mirror.

| Carrier circuit | IPC-101 endpoint | Alpha adapter endpoint | IPC-100 adapter endpoint |
| --- | --- | --- | --- |
| K1 | J1.1 keypad 3.3 V | regulated 3.3 V branch | J10.1 EXPANSION_VCC |
| K2 | J1.2 GND | logic GND | J10.2 GND |
| K3 | J1.3 SDA | GPIO21 SDA | J10.3 SDA |
| K4 | J1.4 SCL | GPIO22 SCL | J10.4 SCL |
| D1 | J4.1 OLED_VCC | regulated 3.3 V branch | J6.1 OLED_VCC |
| D2 | J4.2 OLED_GND | logic GND | J6.2 OLED_GND |
| D3 | J4.3 OLED_SDA | GPIO21 SDA | J6.3 OLED_SDA |
| D4 | J4.4 OLED_SCL | GPIO22 SCL | J6.4 OLED_SCL |
| D5 | J4.5 OLED_RESET | GPIO5 | J6.5 OLED_RESET |
| D6 | no connection | no connection | no connection |
| Separate STOP 1 | J3.1 STOP_IN_RAW | hardware integration pending | J8A STOP_IN_RAW |
| Separate STOP 2 | J3.2 STOP_RETURN | hardware integration pending | J8A STOP_RETURN |

IPC-101 J2 remains the local five-wire connection to the actual OLED module.
Verify module markings and reset continuity; its physical pin order is not
inferred from the carrier circuit order. Use proper mating GH pigtails at J1/J4;
the tested XH-style kit housings do not mate directly with those GH headers.

Keep K and D power, return and bus wires independent throughout the panel harness.
Only the Alpha equipment adapter joins their corresponding buses/rails. The
future IPC-100 adapter routes them to the separate J10 and J6 branches. Never
carry the Alpha branch bridges into that adapter.

Design to the existing IPC-101 interface limits: complete keypad branch <=0.30 m;
complete OLED branch, including J2-to-display wiring, <=0.20 m and 50 pF.
Inspect pull-ups and the 3.3 V supply budget before bench operation; retain the
IPC-100 protected power and pull-up contracts when migrating. Power-off mating.

## Carrier geometry

- K: 11.25 x 4.45 mm main aperture, 2.45 mm hooked-rib notches (fresh four-pin
  housing accepted in revised three-dot gauge).
- D: 16.37 x 4.38 mm main aperture, 2.25 mm hooked-rib notches (six-pin housing
  accepted in revised two-dot gauge).
- Retain the gauge's rib positions and 0.79 mm notch extension.
- Label K / CONTROLS and D / DISPLAY; use asymmetric carrier assembly features
  to prevent reversing the carrier. Housing orientation still needs verification.
- Add a removable retainer and cable strain relief after measuring fully joined
  pair length, flange contact faces and header engagement. Aperture fit alone
  does not establish connector retention, contact engagement or sealing.

No final carrier STL is released yet: joined six- and four-position pair lengths
and the retaining-shoulder geometry are still missing.

## Alpha firmware work required

Current inputs.cpp reads ARM GPIO16, FIRE GPIO17 and encoder GPIO32/33/25;
there is no TCA8418 driver. Existing Wire/I2C initialization supports OLED/BME280,
but does not interpret IPC-101 key events automatically.

Add an explicit IPC-101 panel mode while retaining the legacy input mode:

1. Probe TCA8418 at 0x34, initialize R0-R6/C0 and poll its debounced event FIFO
   every 10-20 ms on a 100 kHz bus. Verify register programming against TI's
   datasheet and the fabricated Rev C netlist when implementing.
2. Map R0 LEFT, R1 RIGHT, R2 UP, R3 DOWN, R4 SELECT, R5 ARM (called START in
   earlier architecture notes), R6 PULL into the existing application controls.
   Define navigation/speed behavior explicitly in the implementation.
3. Preserve ARM/PULL edge qualification and existing actuator interlocks. Handle
   missing panel, I2C errors, overflow and reconnect without replaying stale
   commands or treating a held button as a fresh press; bench-test these cases.
4. Drive Rev C common-anode RGB through TCA8418 COL6/COL7/COL8. Current Alpha
   status_led.cpp instead uses ESP32 PWM for a common-cathode module. Do not
   assume the same PWM brightness/fade behavior is available through the scanner.
5. Disable legacy GPIO input/output initialization in IPC-101 mode before
   treating those pins as available. No firmware is flashed by this plan.

| Removed Alpha function | GPIOs potentially available after migration |
| --- | --- |
| Individual ARM and FIRE buttons | 16, 17 |
| Rotary encoder, replaced by P504 navigation | 32, 33, 25 |
| External RGB, replaced by IPC-101 RGB | 27, 12, 4 |

Five pins become available with controls migration, eight including RGB.
GPIO21/22 remain I2C; GPIO5 remains OLED reset. GPIO39 is already unused in the
current encoder configuration. Pin reuse remains subject to ESP32 pin constraints.
The current ESTOP_PIN is -1: STOP hardware integration is a separate unfinished
part of the Alpha adapter, not supplied by the I2C migration.

## Compatibility and verification

IPC-100's documented interface already targets IPC-101 at address 0x34 through
J10, OLED through J6 and STOP through J8A. Its driver/population and hardware
acceptance still need verification; the interface contract is not evidence of
completed firmware or a validated physical connection.

On PCB arrival, follow IPC-101's Rev C arrival checklist, verify harness continuity
and supply polarity, then test keys, display and LED with motor/trigger outputs
disconnected. Validate disconnect/reconnect and STOP independently before motion.

Sources: sibling IPC101/docs/interface/IPC100_IPC101_INTERFACE.md;
IPC101/docs/architecture/IPC101_ARCHITECTURE.md;
IPC101/hardware/kicad/README.md (Rev C RGB assignment);
Crosswind firmware/crosswind-esp32/src/{config.h,inputs.cpp,status_led.cpp};
cad/phase-1/controller-connector/README.md (physical fit results).

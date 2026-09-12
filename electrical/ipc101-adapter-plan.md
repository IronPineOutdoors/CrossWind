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

## Selected mating arrangement (2026-09-12)

User confirmed that both four- and six-position cable housings mate with the
kit's shrouded PCB pin headers and are retained by the previously measured hooks.
Photo 20260912_100515.jpg shows the assembled pairs. Use these matching headers
as the fixed halves, replacing the earlier two-cable-housings/middle-header idea.
This confirms user-observed mechanical mating and hook retention, not electrical
continuity, endurance or environmental qualification.

Mount the headers on a small adapter PCB supported by the printed carrier.
Keep hook engagement and unplugging access clear; the carrier supports the board
and fixed headers and provides cable strain relief. Do not use the tested
cable-housing apertures as header mounting-pocket dimensions.

## Circuit assignment draft

K1-K4 and D1-D6 below are logical cavity labels, not a released crimp-face drawing.
Mark and verify cavity orientation on the actual housings and continuity through
the matching PCB header before releasing a harness drawing; mating faces can mirror.

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

## Cable-housing clearance references and carrier geometry

- K: 11.25 x 4.45 mm main aperture, 2.45 mm hooked-rib notches (fresh four-pin
  housing accepted in revised three-dot gauge).
- D: 16.37 x 4.38 mm main aperture, 2.25 mm hooked-rib notches (six-pin housing
  accepted in revised two-dot gauge).
- Retain the gauge's rib positions and 0.79 mm notch extension.
- Label K / CONTROLS and D / DISPLAY; use asymmetric carrier assembly features
  to prevent reversing the carrier. Housing orientation still needs verification.
- Design board supports and a removable board retainer around the fixed PCB
  headers; leave room for the cable housings to seat and release their hooks.
- The aperture dimensions above are accepted cable-body clearance references.
  Final access openings must also clear the header shrouds and mating motion.

No final carrier STL or adapter PCB is released yet. Still needed for each
header: plastic body width, thickness and height excluding tails; solder-tail
length and cross-section; first-to-last pin center span; pin-row offset from the
body edges; and the fully mated envelope. Confirm these with calipers rather than
inferring a nominal kit pitch from ruler photos. Retainer/support geometry must
be based on the fixed headers and board, not the superseded loose middle header.

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

## Header pin measurements (2026-09-12)

User measured a 1.83 mm clear gap between adjacent pins and a 0.64 mm pin width
in the same direction. Calculated adjacent center spacing is 2.47 mm (1.83 +
0.64). This is a measured estimate, not a released nominal footprint pitch;
the header size measured was not specified. Do not assume it establishes both
header sizes or the other dimension of the pin cross-section.

Confirm the full row span before PCB layout: measure the outside-to-outside span
of the first and last pins. For six pins, pitch = (span - 0.64 mm) / 5, assuming
the measured pin width applies. This reduces sensitivity to a single-gap reading.

User subsequently measured 13.20 mm outside-to-outside across the six-pin row.
Using the 0.64 mm pin width, the first-to-last center span is 12.56 mm and
average pitch is 12.56 / 5 = 2.512 mm. Prefer this full-row estimate over the
earlier 2.47 mm single-gap estimate. Use 2.50 mm as a provisional layout target,
not a confirmed manufacturer specification; verify with a physical footprint
fit before fabrication. Four-pin row span remains unconfirmed.

## PCB-header coupon results (2026-09-12)

User corrected the pin-hole result: TWO dots fits (1.1 mm square printed holes
at provisional 2.50 mm pitch); ONE dot (0.9 mm) was too tight. This supersedes
the initial one-dot report. Original one-dot body openings (18.2 x 7.2 mm six-pin,
13.2 x 7.2 mm four-pin) were closest but still slightly large. Smaller body
coupons have been generated; final header body fit remains pending. Printed tail
fit does not independently establish nominal pitch or final PCB drill diameter.

Subsequent body result: refinement THREE-dot preferred (six-pin 18 x 7 mm,
four-pin 13 x 7 mm), but four-pin outer side slightly snug, inducing a subtle
angle. Six-pin dimension is the current selection; four-pin seating remains
unresolved. New tests relieve only the presumed outboard short edge by 0.2 / 0.4
mm. Pin coupon selection remains TWO-dot (1.1 mm square holes).

## Fit clarification: raised identification dots (2026-09-12)

User subsequently attributed the apparent four-pin tilt to the raised coupon
dots preventing the plates from sitting flush together, and believes the fit is
OK. Retain refinement THREE-dot body openings: six-pin 18.0 x 7.0 mm and four-pin
13.0 x 7.0 mm. Retain original TWO-dot tail holes: 1.1 mm square at provisional
2.50 mm pitch. The outer-edge-relief coupons are superseded diagnostic options;
do not carry their enlargement into the carrier. Final mating/support surfaces
must be flat, with identification outside those surfaces. Integrated header
seating and mating still need an assembly fit check.

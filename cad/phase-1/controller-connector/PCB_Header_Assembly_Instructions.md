# Assembled PCB-header fit test

Print PCB_Header_Assembly_All_PRINT.stl: three separate pieces, 44 x 84 mm total.
PETG, 0.4 mm nozzle, 0.2 mm layers, four walls, solid fill, flat as exported,
no supports. Individual Guide, Pin_Plate and Stand STLs are also supplied.
No raised identification dots: all contacting faces are flat.

## Parts and assembly

1. Place the 3 mm tall open rectangular STAND on the table.
2. Rest the 1.2 mm PIN PLATE on it. Its six-hole group goes with the wide header,
   its four-hole group with the narrow header. The stand leaves 3 mm of space
   beneath the plate; 3.19 mm tails through 1.2 mm plastic protrude about 1.99 mm.
3. Place the 2 mm GUIDE on the pin plate, wide opening over the six-hole group.
   Align the short ends, leaving approximately 2 mm of pin plate visible along
   each long edge. Do not glue or clamp these pieces yet.
4. Insert one UNMATED header tails-first through the guide and into the pin
   holes. Slide the guide relative to the pin plate as needed, rather than
   pushing sideways on the pins. The header base rests on the pin plate.
5. Repeat with the other header. If both cannot seat together, test individually
   and report which header needs a different guide position/orientation.
6. Holding the stack and fixed header by hand, gently try the matching cable
   plug. Check full seating, hook engagement and release access. Support the
   header during unplugging; this loose stack does not retain it mechanically.

The guide can slide in both planar directions; there are no alignment pegs or
fasteners to force an unverified offset. Recessed/raised markings are omitted
from this initial stack so neither face is obstructed. If the plate is flipped,
its row offset reverses: use whichever orientation lets the shrouds align, and
report the orientation relative to the hooks. Never bend pins to align the stack.

## Dimensions and scope

Guide openings: six-pin 18 x 7 mm; four-pin 13 x 7 mm (accepted refinement 3-dot).
Pin holes: 1.1 mm square, 2.50 mm provisional pitch (accepted original 2-dot).
The four-pin row is centered at the actual opening center, 32.5 mm from the
left edge. Earlier standalone tail coupons did not establish this registration.

Nominal row placement uses the inferred 3.83 / 2.45 mm body-edge offsets,
allowing for an inferred 6.28 mm body within the accepted 7 mm opening. Because
measurement orientation and size applicability remain uncertain, the guide is
intentionally free to slide. This is a dry mechanical assembly test, not a
finished panel mount or an adapter PCB. Do not solder to the printed pin plate.

Report: do both headers sit square together; do plugs engage/release their hooks;
and how far does the guide need to move from its centered position (a photo is
useful). After this check, the verified registration can be used for the fixed
carrier and adapter board. No power or wiring is needed for this test.

Validation: generator checks each STL is manifold, watertight, consistently
wound, positive-volume and has the expected separate-solid count. Nominal stack
parts have no volume overlap and the measured tail length clears the stand.

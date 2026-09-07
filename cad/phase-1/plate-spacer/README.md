# Crosswind Alpha plate spacer

Hollow spacer for mounting the electronics/logo plate and power/driver plate above the base-box floor using 1/4-20 hex bolts and T-nuts.

- Height: 15 mm (requested).
- Through bore: 7 mm, clearance for a nominal 6.35 mm bolt.
- Outside diameter: 16 mm (proposed; verify room around each plate hole).
- Radial wall: 4.5 mm.
- No internal threads; the bolt passes freely through to the T-nut.

Print `Crosswind_Plate_Spacer_15mm_QuarterInch_PRINT.stl` upright as exported, with the flat annular end on the bed. No supports are needed. Use the same material as the mounting plates where practical and enough walls to make the annular wall solid. Print one first and check bolt passage and seating before duplicating for every mounting hole. The hole count and plate mounting-hole dimensions have not been supplied; no existing plate geometry has been changed.

Stack, from above: bolt head, flat washer, mounting plate, 15 mm spacer, box floor, T-nut installed from the underside. Seat the T-nut in the wood before tightening through the printed plate/spacer. Tighten only enough to retain the assembly without crushing the plastic. Keep metal washers and bolt heads clear of conductors and component undersides.

Select bolt length from the actual stack and T-nut thread engagement; it is not fixed by the spacer height alone. Verify any T-nut barrel protrusion does not interfere with the spacer bore or prevent flat seating. Confirm underside solder joints/wiring clear the floor with the plate supported at all mounting holes. These spacers support the carrier plates, not the individual electronic modules.

Regenerate with `python generate_stl.py` (requires `manifold3d`). The generator checks solid validity, connected geometry, overall dimensions, and volume. Physical fit and sustained clamp loading remain to be checked on the assembly.

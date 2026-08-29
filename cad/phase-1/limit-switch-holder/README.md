# YL-99 Adjustable Limit-Switch Holder

This folder contains a printable holder for the Crosswind Alpha left and right
YL-99 limit-switch modules.

## Files

- `crosswind_yl99_holder.stl` — one holder
- `crosswind_yl99_holder_pair.stl` — two holders arranged for one print job
- `generate_stl.py` — dependency-free source/generator

## Dimensions and hardware

- Overall envelope: 38 mm wide x 38 mm deep x 42 mm high
- Base: 4 mm thick with two 4.5 mm x 20 mm adjustment slots
- Upright: 26 mm wide and 4 mm thick, with one 3.6 mm x 16 mm module slot
- Module fastener: M3 x 12 mm bolt, washers, and locknut
- Base fasteners: M4 or #8 pan-head screws with washers

The holder is sized from the measured Crosswind module: 18 mm PCB length, 14 mm
PCB width, approximately 6 mm switch block, 30 mm complete pin-to-switch
envelope, and a 3 mm mounting hole whose center is 6 mm from the pin end. The
lever rests approximately 4 mm above the switch block and travels about 2 mm
before clicking. The vertical slot retains 16 mm of setup adjustment.

Mount the board with its pins downward and its open switch mouth/offset mounting
hole toward the center slot. During setup, move the bracket until the flag just
touches the lever, then add approximately 1.5 mm of engagement. Confirm the
electrical transition by hand before powering the motor; do not use all of the
lever's available overtravel.

## Printing

Print in the supplied orientation with the flat base on the bed. Use PETG, ASA,
or another heat- and weather-tolerant material, 0.2 mm layers, at least four
walls, and 35% or greater infill. No supports are required. Install a washer on
each side of every printed slot and tighten only enough to prevent movement.

The roller arm is a sensor actuator, not a hard stop. Install an independent
mechanical stop beyond each switch actuation point as required by the Crosswind
safety notes.

Regenerate both STL files from this directory with:

```powershell
python .\generate_stl.py
```

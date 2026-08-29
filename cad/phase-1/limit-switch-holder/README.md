# YL-99 Adjustable Limit-Switch Holder

This folder contains a printable holder for the Crosswind Alpha left and right
YL-99 limit-switch modules.

## Files

- `crosswind_yl99_holder.stl` — one holder
- `crosswind_yl99_holder_pair.stl` — two holders arranged for one print job
- `generate_stl.py` — dependency-free source/generator

## Dimensions and hardware

- Overall envelope: 50 mm wide x 44 mm deep x 46 mm high
- Base: 4 mm thick with two 5 mm x 24 mm adjustment slots
- Upright: 4 mm thick with one 4 mm x 25 mm vertical module slot
- Module fastener: M3 x 12 mm bolt, washers, and locknut
- Base fasteners: M4 or #8 pan-head screws with washers

The YL-99 name is used for several board sizes. The long module slot accepts the
common single-M3-hole variants; confirm fit against the actual module before
mounting the powered mechanism.

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

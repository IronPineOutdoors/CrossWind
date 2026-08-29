# Adjustable Limit-Switch Striker Flag

These flags mount to the rotating Crosswind top plate and actuate the stationary
YL-99 switches. The same part works at both travel limits.

## Files

- `crosswind_limit_flag.stl` - one flag
- `crosswind_limit_flag_pair.stl` - two flags arranged for one print job
- `generate_stl.py` - dependency-free source/generator

## Part and hardware

- Overall envelope: 34 mm wide x 34 mm deep x 36 mm high
- Mounting flange: 4 mm thick with two 4.5 mm x 21 mm adjustment slots
- Striker face: 20 mm wide x 32 mm high x 4 mm thick
- Fasteners: two M4 or #8 pan-head screws and large washers per flag

Print with the mounting flange flat on the bed. Use PETG or ASA, 0.2 mm layers,
at least four walls, and at least 35% infill. No supports are required.

Install the flange on the underside of the rotating plate so the striker points
down toward the stationary switch. It may instead mount to a suitable edge tab
if that better clears the linkage. Use washers over both slots, leave the screws
slightly loose, and slide the flag until it pushes the lever approximately 1.5
mm beyond first contact. Tighten and test by hand before powering the motor.

The flag must actuate the switch before a separate mechanical hard stop engages.
It is not designed to arrest the rotating plate or carry mechanism loads.

Regenerate the STL files with:

```powershell
python .\generate_stl.py
```

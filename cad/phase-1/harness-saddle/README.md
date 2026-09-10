# Harness saddle for 13.8 mm automotive loom

First-fit prototype for tubing measured at 13.8 mm outside diameter. Install over
an existing harness with the open side against the wooden mounting surface.

- Installed envelope: 42 x 18 x 17 mm.
- Smooth channel: 14.2 mm wide, 13.8 mm high above the mounting surface.
- Arch wall: 3.2 mm; mounting ears: 4 mm thick.
- Two 4.6 mm clearance holes, 30 mm center to center, for #6 or #8 screws.
- Flat screw seats accept pan heads; small flat washers can spread the load.
  Use washers no larger than 9 mm OD to retain clearance to the arch.

## Bambu A1 printing

Open `Crosswind_Harness_Saddle_13.8mm_PRINT.stl` in Bambu Studio and select your
A1 and actual filament profile. The STL is already resting on its flat end face:
the print is 18 mm tall and the arch lies in the layer plane. Do not auto-orient
it onto the mounting ears. Use PETG, a 0.4 mm nozzle, 0.2 mm layers, five walls,
five top/bottom layers, and 35% gyroid infill. Supports are not intended; the
horizontal 4.6 mm screw bores have short bridges. Deburr/check screw passage.
Use a 3 mm outer brim if adhesion requires it. Print one for fit before making more.

## Mounting and grip

For 3/4-inch plywood use two #6 or #8 x 3/4-inch pan-head WOOD screws.
Machine screws require nuts or threaded inserts. The 4 mm ears leave at most
15.05 mm of screw penetration before adding washers, below 19.05 mm plywood
thickness. Verify the actual wood thickness and bearing clearance first.
Pilot drill to suit the wood and screw root diameter; limit drill depth.
Snug the ears against the wood without crushing the print or tubing.

This is a loom hold-down, with strain relief dependent on actual friction and
loom stiffness. Corrugation pitch and groove diameter were not supplied, so
there are no groove-locking teeth. The channel has lateral clearance and nominal
contact at its crown; it does not guarantee axial retention. With power off,
check whether the loom slides under the expected pull. A thin rubber strip may
improve grip if it fits without forcing the ears down or crushing the loom.
Wires may slide independently inside split loom: retain the harness separately
where connector strain relief is required. Leave a service loop at moving joints.

`generate_stl.py` recreates the STL and preview and checks one connected,
watertight, consistently wound solid, positive volume, and bounding dimensions.
Mesh checks do not establish physical fit or pull strength.

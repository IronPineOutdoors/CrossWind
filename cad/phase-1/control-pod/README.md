# Angled Control Pod

`crosswind_alpha_control_pod.stl` is a provisional body for the 150 x 100 mm Crosswind control panel. It mounts to the vertical face of the 4-inch-tall wooden base and tilts the display 47 degrees upward from vertical. The original 30-degree prototype produced side-cheek cantilever warnings when printed face-down; 47 degrees provides margin beyond the usual 45-degree self-support threshold.

## Geometry

- Open rear against the wooden base
- 156.6 mm overall width and 101.6 mm overall height
- 3 mm nominal walls and 7 mm faceplate rim
- Four reinforced 9 mm-diameter x 8 mm-deep bosses with 2.7 mm M3 pilot holes, matching the existing panel
- Solid rear side-wall margins intended to be drilled after printing for wood mounting
- Centered 32 x 14 mm lower cable notch
- Recorded 19.05 mm (3/4 inch) wooden top thickness as an overhead keep-out

The body intentionally contains no main electronics. It only protects the OLED, encoder, LED, buttons, and their harnesses. Check the cable notch, fastener lengths, button depth, and connector bend radius on a fit print before outdoor use.

The STL is saved in its print orientation with the inclined face rim on the build plate. The open shell, 47-degree side cheeks, and M3 bosses rise from that rim without the unsupported projections present in the earlier prototype. The M3 pilot bores pass completely through the corner shell so they do not end in downward-facing blind-hole ceilings.

Clamp the pod in its installed position and drill through the solid rear margins of the left and right side walls for the selected wood screws. Use two well-spaced holes per side, keep each hole center at least 8 mm from an edge, back the wall with scrap wood while drilling, and do not overtighten against the printed plastic. Separate projecting tabs and their round holes were removed because Bambu Studio classified their undersides and upper arcs as floating cantilevers.

The 2.7 mm boss holes are intended for M3 screws to form threads directly in PETG/ASA. Start the screws square, use moderate torque, and avoid repeated removal. Drill and tap the bosses or revise the constants for heat-set inserts if frequent panel service is expected.

## Regeneration

```powershell
python .\generate_stl.py
```

Requires `manifold3d`.

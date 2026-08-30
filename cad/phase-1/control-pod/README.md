# Angled Control Pod

`crosswind_alpha_control_pod.stl` is a provisional body for the 150 x 100 mm Crosswind control panel. It mounts to the vertical face of the 4-inch-tall wooden base and tilts the display 30 degrees upward from vertical.

## Geometry

- Open rear against the wooden base
- 156.6 mm overall width and 101.6 mm overall height
- 3 mm nominal walls and 7 mm faceplate rim
- Four reinforced 9 mm-diameter x 8 mm-deep bosses with 2.7 mm M3 pilot holes, matching the existing panel
- Four external mounting ears with 5 mm wood-screw clearance
- Centered 32 x 14 mm lower cable notch
- Recorded 19.05 mm (3/4 inch) wooden top thickness as an overhead keep-out

The body intentionally contains no main electronics. It only protects the OLED, encoder, LED, buttons, and their harnesses. Check the cable notch, fastener lengths, button depth, and connector bend radius on a fit print before outdoor use.

The STL is saved in its print orientation with the inclined face rim on the build plate. The open shell and M3 bosses rise from that rim, avoiding the large front-shell cantilever produced by the installed orientation. A slicer may still request localized support beneath the external wooden mounting ears; use build-plate-only support there if required.

The 2.7 mm boss holes are intended for M3 screws to form threads directly in PETG/ASA. Start the screws square, use moderate torque, and avoid repeated removal. Drill and tap the bosses or revise the constants for heat-set inserts if frequent panel service is expected.

## Regeneration

```powershell
python .\generate_stl.py
```

Requires `manifold3d`.

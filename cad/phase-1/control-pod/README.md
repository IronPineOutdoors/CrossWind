# Angled Control Pod

`crosswind_alpha_control_pod.stl` is a provisional body for the 150 x 100 mm Crosswind control panel. It mounts to the vertical face of the 4-inch-tall wooden base and tilts the display 30 degrees upward from vertical.

## Geometry

- Open rear against the wooden base
- 156.6 mm overall width and 101.6 mm overall height
- 3 mm nominal walls and 7 mm faceplate rim
- Four 3.2 mm faceplate holes matching the existing panel
- Four external mounting ears with 5 mm wood-screw clearance
- Centered 32 x 14 mm lower cable notch
- Recorded 19.05 mm (3/4 inch) wooden top thickness as an overhead keep-out

The body intentionally contains no main electronics. It only protects the OLED, encoder, LED, buttons, and their harnesses. Check the cable notch, fastener lengths, button depth, and connector bend radius on a fit print before outdoor use.

The default STL orientation is the installed orientation, not necessarily the strongest or fastest print orientation. Rotate it onto a side wall for printing and use supports under the mounting ears if required by the chosen orientation.

## Regeneration

```powershell
python .\generate_stl.py
```

Requires `manifold3d`.

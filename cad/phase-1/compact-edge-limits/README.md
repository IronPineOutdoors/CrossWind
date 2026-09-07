# Compact edge limits — first-fit prototype

For the existing measured Crosswind YL-99 modules. This is an alternative to the
tall holder and hanging flag, not a mount for an unspecified replacement switch.
Dimensions are millimetres. Physical fit and switch operation are not yet verified.

## Print files

- `yl99_edge_bracket.stl`: print two, 30 x 24 x 32 mm each.
- `edge_ramp.stl`: print two, 48 x 25 x 4 mm each.
- `complete_two_endpoint_kit.stl`: all four pieces on one bed, 68 x 90 mm footprint.

Print supplied orientation, flat bases down, PETG or ASA, 0.2 mm layers,
four walls and 40% infill. The upright slot may need a small bridge or local
support depending on the printer. Deburr slots before fitting hardware.

## Arrangement and assumptions

The switch and its entire bracket sit OUTSIDE the moving plate's swept edge.
The 8 mm gap is not used to house a switch. A 4 mm flat cam attaches to the
moving plate near its edge; its ramped edge projects beyond the plate to meet
the roller. The roller rides along the cam's edge, not over its flat face.
The cam's long dimension follows the local direction of plate travel.
The double ramp lets the same part serve either endpoint.

This requires accessible plate edges and a stationary mounting surface for the
bracket. Deck overhang, plate radius, roller height and available mounting area
are unknown. Check these before drilling. If the cam occupies the 8 mm gap,
its 4 mm thickness leaves only approximately 4 mm for fastener heads and running
clearance: keep nuts out of that gap and measure the actual remaining clearance.
The cam contact edge and switch must remain outside the deck obstruction.

## Hardware and setup

1. Fasten each bracket to stationary structure through its two 4.5 x 12 mm
   rounded slots using M4 screws and washers. Fastener length depends on the
   structure. Bracket adjustment is tangential, not radial.
2. Attach the YL-99 through its measured 3 mm PCB hole using an M3 screw,
   insulating washers and locknut in the 3.6 x 16 mm upright slot. Select screw
   length for actual PCB/spacer thickness. Keep solder joints clear of the mount.
   Place the module on the face away from the base, roller toward the cam, pins
   downward. Verify the particular board can take this orientation and cannot
   pivot under actuation; do not overtighten its single mounting hole.
3. Set bracket location radially before drilling so the roller is clear of the
   moving plate and meets only the projecting cam. Adjust board height until
   the roller is centered on the 4 mm cam edge. Check full travel by hand.
4. Fasten each cam with two M4 screws and washers in its 4.5 x 14 mm slots.
   Both slots run along travel, providing 9.5 mm of screw-center adjustment.
   Loosen both screws, slide to set reversal position, then tighten both.
   Two screws resist rotation during repeated contact.
5. The cam has 3 mm radial rise over a 15 mm entry ramp, 18 mm dwell, and a
   15 mm exit ramp. This is geometry, NOT a specified switch depression.
   Set radial placement so the electrical input changes before the plateau,
   while preserving the actual switch's permissible overtravel. Do not assume
   the previous design's 1.5 mm engagement setting is sufficient.
6. Test electrical actuation by hand with motor power disconnected, then test
   low-speed reversal. Confirm no roller bottoming, board movement, cam slip or
   interference. Actual angular adjustment and dwell depend on mounting radius;
   stopping travel must remain within the actuated portion of the cam.

Maintain separate mechanical stops beyond actuation and before damaging
overtravel. These printed parts sense position; they do not arrest the mechanism.
If the PCB rotates on its single fastener or the roller cannot stay on the thin
cam edge, revise the mount from measured fit rather than running the mechanism.

## Regeneration and validation

Run `python generate_stl.py` with numpy, trimesh and manifold3d installed.
Boolean-unioned meshes are checked for watertight positive volume and one
connected solid per part; the kit is checked for four separate solids.
These checks establish printable mesh integrity, not installed mechanical fit.

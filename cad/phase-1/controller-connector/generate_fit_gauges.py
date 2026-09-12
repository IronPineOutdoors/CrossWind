"""Fit gauges for the user's measured 6+6+4 controller connector housings.

Requires manifold3d, numpy and trimesh. Dimensions in millimetres.
"""
from pathlib import Path
import manifold3d as m
import numpy as np
import trimesh

OUT = Path(__file__).resolve().parent
SIX = (15.97, 3.98)
FOUR = (10.65, 3.85)
CLEARANCES = (0.2, 0.4, 0.6)  # Added to total width AND thickness, not per side.
RIB_END_INSET = 2.51  # Housing end to nearest rib edge, both connector sizes.
RIB_WIDTH = 0.71  # Straight shaft width; retains the measured rib locations.
HOOK_WIDTH = 1.28  # Maximum hooked-tip width, confirmed 2026-09-12.
# Tip offset is unmeasured: cover the extra width on either side of the shaft.
HOOK_SIDE_ALLOWANCE = HOOK_WIDTH - RIB_WIDTH
RIB_ENVELOPE_WIDTH = RIB_WIDTH + 2 * HOOK_SIDE_ALLOWANCE
RIB_PROJECTION = 0.79

def cube(size, pos):
    return m.Manifold.cube(size).translate(pos)

def gauge(clearance, dots):
    corners = [m.Manifold.cylinder(2, 2, circular_segments=32).translate((x,y,0))
               for x in (2,36) for y in (2,14)]
    solid = m.Manifold.batch_hull(corners)
    for (width, height), x in ((SIX,3),(FOUR,22)):
        cutter = cube((width+clearance,height+clearance,4),(x,4,-1))
        # Center the nominal housing in its aperture. Both ribs face -Y,
        # away from the identification dots. Expand rib sides by clearance/2.
        for rib_left in (RIB_END_INSET, width-RIB_END_INSET-RIB_WIDTH):
            notch = cube((RIB_ENVELOPE_WIDTH+clearance,RIB_PROJECTION+clearance/2,4),
                         (x+rib_left-HOOK_SIDE_ALLOWANCE,4-RIB_PROJECTION,-1))
            cutter += notch
        solid -= cutter
        assert (solid ^ cutter).volume() < 1e-6
    for i in range(dots):
        solid += m.Manifold.cylinder(.6,.8,circular_segments=24).translate((4+i*3,12,2))
    assert solid.status() == m.Error.NoError
    return solid

def export(solid, name, expected_bodies):
    raw = solid.to_mesh()
    mesh = trimesh.Trimesh(vertices=np.asarray(raw.vert_properties)[:,:3],
                           faces=np.asarray(raw.tri_verts),process=True)
    assert mesh.is_watertight and mesh.is_winding_consistent and mesh.volume > 0
    assert len(mesh.split()) == expected_bodies
    mesh.export(OUT/name)
    print(f'{name}: {expected_bodies} closed solid(s), bounds {mesh.extents.round(2)}, volume {mesh.volume:.1f} mm3')

if __name__ == '__main__':
    parts=[]
    for i, clearance in enumerate(CLEARANCES,1):
        solid=gauge(clearance,i)
        export(solid,f'Connector_Fit_{i}dot_{clearance:.1f}mm_PRINT.stl',1)
        parts.append(solid.translate((0,(i-1)*21,0)))
    export(m.Manifold.batch_boolean(parts,m.OpType.Add),'Connector_Fit_All_Three_PRINT.stl',3)

"""PCB header body and tail fit coupons; dimensions in mm, photo estimates."""
from pathlib import Path
import manifold3d as m
import numpy as np
import trimesh

OUT = Path(__file__).resolve().parent
# Approximate shroud envelopes from user ruler photos, not manufacturer dimensions.
BODY_SIX = (18.0, 7.0)
BODY_FOUR = (13.0, 7.0)
CLEARANCES = (0.2, 0.6, 1.0)

def box(size, pos):
    return m.Manifold.cube(size).translate(pos)

def dots(solid, count, y, z):
    for i in range(count):
        solid += m.Manifold.cylinder(.6, .8, circular_segments=24).translate((4+3*i,y,z))
    return solid

def body_coupon(clearance, count):
    solid = box((44,18,2), (0,0,0))
    for dims,x in ((BODY_SIX,3),(BODY_FOUR,26)):
        w,h = (v+clearance for v in dims)
        cutter = box((w,h,4),(x,4,-1))
        solid -= cutter
        assert (solid ^ cutter).volume() < 1e-7
    return dots(solid,count,15,2)

def tail_coupon():
    # Separate thin board isolates tail-hole fit from uncertain shroud/pin offsets.
    solid = box((44,32,1.2),(0,0,0))
    for row,hole in enumerate((.9,1.1)):
        y = 7+15*row
        for pins,cx in ((6,12),(4,33)):
            for i in range(pins):
                x=cx+(i-(pins-1)/2)*2.50
                cutter=box((hole,hole,3),(x-hole/2,y-hole/2,-1))
                solid -= cutter
                assert (solid ^ cutter).volume() < 1e-7
        solid = dots(solid,row+1,y+5,1.2)
    return solid

def export(solid,name,bodies):
    assert solid.status() == m.Error.NoError
    raw=solid.to_mesh()
    mesh=trimesh.Trimesh(vertices=np.asarray(raw.vert_properties)[:,:3],
                         faces=np.asarray(raw.tri_verts),process=True)
    assert mesh.is_watertight and mesh.is_winding_consistent and mesh.volume>0
    assert len(mesh.split())==bodies
    mesh.export(OUT/name)
    print(name, 'closed bodies:', bodies, 'bounds:',mesh.extents.round(2))
    return mesh

if __name__ == '__main__':
    parts=[]
    for n,c in enumerate(CLEARANCES,1):
        part=body_coupon(c,n)
        export(part,f'PCB_Header_Body_{n}dot_PRINT.stl',1)
        parts.append(part.translate((0,(n-1)*23,0)))
    tails=tail_coupon()
    export(tails,'PCB_Header_Tails_PRINT.stl',1)
    parts.append(tails.translate((0,69,0)))
    export(m.Manifold.batch_boolean(parts,m.OpType.Add),
           'PCB_Header_All_Tests_PRINT.stl',4)

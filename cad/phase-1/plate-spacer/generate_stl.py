"""Generate a 15 mm hollow plate spacer for 1/4-20 through bolts; millimetres."""
import math
from pathlib import Path
import struct
import manifold3d as m

HEIGHT = 15.0
OUTSIDE_DIAMETER = 16.0
BORE_DIAMETER = 7.0
SEGMENTS = 128


def spacer():
    outer = m.Manifold.cylinder(HEIGHT, OUTSIDE_DIAMETER / 2, circular_segments=SEGMENTS)
    bore = m.Manifold.cylinder(HEIGHT + 2, BORE_DIAMETER / 2, circular_segments=SEGMENTS).translate((0, 0, -1))
    return (outer - bore).translate((OUTSIDE_DIAMETER / 2, OUTSIDE_DIAMETER / 2, 0))


if __name__ == '__main__':
    solid = spacer()
    assert solid.status() == m.Error.NoError and len(solid.decompose()) == 1
    expected = math.pi / 4 * (OUTSIDE_DIAMETER**2 - BORE_DIAMETER**2) * HEIGHT
    assert abs(solid.volume() - expected) / expected < .001
    assert tuple(solid.bounding_box()) == (0., 0., 0., 16., 16., 15.)
    mesh = solid.to_mesh()
    points = mesh.vert_properties[:, :3]
    path = Path(__file__).with_name('Crosswind_Plate_Spacer_15mm_QuarterInch_PRINT.stl')
    with path.open('wb') as output:
        output.write(b'Crosswind 15mm spacer / 7mm bore / 16mm OD'.ljust(80, b'\0'))
        output.write(struct.pack('<I', len(mesh.tri_verts)))
        for triangle in mesh.tri_verts:
            a, b, c = points[triangle]
            u, v = b-a, c-a
            normal = (u[1]*v[2]-u[2]*v[1], u[2]*v[0]-u[0]*v[2], u[0]*v[1]-u[1]*v[0])
            length = math.sqrt(sum(float(n)**2 for n in normal))
            output.write(struct.pack('<12fH', *(float(n)/length for n in normal), *a, *b, *c, 0))
    print(f'Generated {path.name}: {solid.num_tri()} triangles; dimensions and annular volume verified')

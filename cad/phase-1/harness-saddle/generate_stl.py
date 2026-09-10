"""13.8 mm loom saddle; millimetres. Requires manifold3d, numpy, trimesh, Pillow."""
from pathlib import Path
import numpy as np
import manifold3d as m
import trimesh
from PIL import Image, ImageDraw

OUT = Path(__file__).resolve().parent
WIDTH, DEPTH, FOOT = 42.0, 18.0, 4.0
BORE_RADIUS, CENTER_HEIGHT, WALL = 7.1, 6.7, 3.2
HOLE_DIAMETER, HOLE_SPACING = 4.6, 30.0

def box(size, origin):
    return m.Manifold.cube(size).translate(origin)

def axial_cylinder(radius, length):
    return m.Manifold.cylinder(length, radius, circular_segments=128).rotate((90, 0, 0)).translate((0, length / 2, CENTER_HEIGHT))

def build():
    # Rounded mounting ears, with a continuous thick arch between them.
    posts = [m.Manifold.cylinder(FOOT, 3, circular_segments=48).translate((x,y,0))
             for x in (-WIDTH/2+3, WIDTH/2-3) for y in (-DEPTH/2+3, DEPTH/2-3)]
    feet = m.Manifold.batch_hull(posts)
    outer = axial_cylinder(BORE_RADIUS+WALL, DEPTH)
    outer += box((2*(BORE_RADIUS+WALL), DEPTH, CENTER_HEIGHT), (-(BORE_RADIUS+WALL),-DEPTH/2,0))
    solid = feet + outer
    tunnel = axial_cylinder(BORE_RADIUS, DEPTH+2)
    tunnel += box((2*BORE_RADIUS, DEPTH+2, CENTER_HEIGHT+1), (-BORE_RADIUS,-DEPTH/2-1,-1))
    solid -= tunnel
    solid = m.Manifold.batch_boolean([solid, box((60,40,30),(-30,-20,0))], m.OpType.Intersect)
    for x in (-HOLE_SPACING/2, HOLE_SPACING/2):
        solid -= m.Manifold.cylinder(FOOT+2, HOLE_DIAMETER/2, circular_segments=64).translate((x,0,-1))
    assert solid.status() == m.Error.NoError and not solid.is_empty()
    raw = solid.to_mesh()
    mesh = trimesh.Trimesh(vertices=np.asarray(raw.vert_properties)[:,:3], faces=np.asarray(raw.tri_verts), process=True)
    assert mesh.is_watertight and mesh.is_winding_consistent and mesh.volume > 0
    assert len(mesh.split()) == 1
    assert np.allclose(mesh.extents, [42,18,17], atol=.02)
    return mesh

def preview(mesh):
    # Orthographic shaded view of installed orientation, without extra render dependencies.
    right = np.array([.83,-.55,0]); right /= np.linalg.norm(right)
    up = np.array([.28,.42,.86]); up -= right*np.dot(up,right); up /= np.linalg.norm(up)
    front = np.cross(right,up)
    vertices = mesh.vertices @ np.stack([right,up,front],axis=1)
    canvas = Image.new('RGB',(1100,740),'#f1f4f6'); draw = ImageDraw.Draw(canvas)
    scale = 19
    points = np.column_stack([550+vertices[:,0]*scale,440-vertices[:,1]*scale])
    light=np.array([-.3,-.5,.8]); light/=np.linalg.norm(light)
    for i in np.argsort(vertices[mesh.faces,2].mean(axis=1)):
        shade=.55+.45*abs(float(np.dot(mesh.face_normals[i],light)))
        color=tuple(int(c*shade) for c in (53,139,164))
        draw.polygon([tuple(p) for p in points[mesh.faces[i]]],fill=color)
    draw.text((32,25),'CROSSWIND | 13.8 mm HARNESS SADDLE',fill='#183443')
    draw.text((32,660),'42 x 18 x 17 mm | 30 mm screw spacing | 4.6 mm clearance holes',fill='#183443')
    draw.text((32,688),'Shown installed. STL rests on an end face for support-free printing.',fill='#183443')
    canvas.save(OUT/'preview.png')

if __name__ == '__main__':
    mesh=build()
    preview(mesh)
    mesh.apply_transform(trimesh.transformations.rotation_matrix(np.pi/2,[1,0,0]))
    mesh.apply_translation(-mesh.bounds[0])
    assert np.allclose(mesh.extents,[42,17,18],atol=.02)
    mesh.export(OUT/'Crosswind_Harness_Saddle_13.8mm_PRINT.stl')
    print(f'Validated: one watertight solid; print bounds {mesh.extents}; volume {mesh.volume:.1f} mm3')

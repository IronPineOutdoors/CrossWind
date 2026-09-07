"""First-fit YL-99 edge mounts. Millimetres; pip install trimesh manifold3d numpy."""
from pathlib import Path
import numpy as np
import trimesh as tm

OUT = Path(__file__).resolve().parent


def box(size, center):
    mesh = tm.creation.box(extents=size)
    mesh.apply_translation(center)
    return mesh


def union(parts):
    return tm.boolean.union(parts, engine="manifold")


def slot(length, diameter, depth, center, axis="z"):
    # Rounded slot runs along X; cut direction Z, or Y for upright.
    spacing = length - diameter
    parts = [box((spacing, diameter, depth), (0, 0, 0))]
    for x in (-spacing / 2, spacing / 2):
        c = tm.creation.cylinder(radius=diameter / 2, height=depth, sections=48)
        c.apply_translation((x, 0, 0))
        parts.append(c)
    mesh = union(parts)
    if axis == "y":
        # Slot length becomes vertical; cut direction becomes Y.
        mesh.apply_transform(np.array([[0, 1, 0, 0], [0, 0, 1, 0], [1, 0, 0, 0], [0, 0, 0, 1]]))
    mesh.apply_translation(center)
    return mesh


def bracket():
    # Base sits outside moving plate; module mounts on outward upright face.
    body = union([box((30, 24, 4), (0, 12, 2)),
                  box((24, 4, 32), (0, 22, 16))])
    holes = [slot(12, 4.5, 8, (0, y, 2)) for y in (6, 15)]
    holes.append(slot(16, 3.6, 8, (0, 22, 19), axis="y"))
    return tm.boolean.difference([body, *holes], engine="manifold")


def ramp():
    # Plan-view double ramp: 15 mm lead-in, 18 mm dwell, 15 mm lead-out.
    # Radial cam lift 3 mm; only 4 mm thick axially.
    xy = [(-24, 0), (24, 0), (24, 22), (9, 25), (-9, 25), (-24, 22)]
    vertices = [(x, y, z) for z in (0, 4) for x, y in xy]
    faces = []
    for i in range(1, 5):
        faces.extend([(0, i + 1, i), (6, 6 + i, 7 + i)])
    for i in range(6):
        j = (i + 1) % 6
        faces.extend([(i, j, j + 6), (i, j + 6, i + 6)])
    body = tm.Trimesh(vertices=vertices, faces=faces)
    holes = [slot(14, 4.5, 8, (x, 8, 2)) for x in (-13, 13)]
    return tm.boolean.difference([body, *holes], engine="manifold")


def shifted(mesh, xyz):
    result = mesh.copy()
    result.apply_translation(xyz)
    return result


if __name__ == "__main__":
    parts = {"yl99_edge_bracket": bracket(), "edge_ramp": ramp()}
    for name, mesh in parts.items():
        assert mesh.is_watertight and mesh.is_volume
        assert len(mesh.split()) == 1
        mesh.export(OUT / f"{name}.stl")
        print(f"{name}: {mesh.extents.round(2)} mm; watertight solid; {mesh.volume:.1f} mm3")
    kit = tm.util.concatenate([
        shifted(parts["yl99_edge_bracket"], (-19, 0, 0)),
        shifted(parts["yl99_edge_bracket"], (19, 0, 0)),
        shifted(parts["edge_ramp"], (0, 32, 0)),
        shifted(parts["edge_ramp"], (0, 65, 0)),
    ])
    assert kit.is_watertight and len(kit.split()) == 4
    kit.export(OUT / "complete_two_endpoint_kit.stl")

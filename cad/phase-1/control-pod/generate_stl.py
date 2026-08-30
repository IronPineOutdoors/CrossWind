#!/usr/bin/env python3
"""Generate the provisional angled Crosswind control-pod body.

Requires manifold3d (`python -m pip install manifold3d`). Dimensions are mm.
The existing 150 x 100 mm control panel fastens to the inclined front rim.
"""

from __future__ import annotations

import math
import struct
from pathlib import Path

import manifold3d as m3d

PANEL_WIDTH = 150.0
PANEL_HEIGHT = 100.0
PANEL_TILT_FROM_VERTICAL = 30.0
PANEL_CLEARANCE = 0.6
BODY_WIDTH = PANEL_WIDTH + 2.0 * (3.0 + PANEL_CLEARANCE / 2.0)
WOOD_SIDE_HEIGHT = 101.6  # 4 inches
WOOD_TOP_THICKNESS = 19.05  # 3/4 inch; recorded as an overhead keep-out.
WALL = 3.0
FACE_RIM = 7.0
LOWER_EDGE_HEIGHT = 7.5
M3_CLEARANCE_DIAMETER = 3.2
CIRCLE_SEGMENTS = 64

MOUNT_EAR_WIDTH = 18.0
MOUNT_EAR_HEIGHT = 22.0
MOUNT_EAR_THICKNESS = 4.0
WOOD_SCREW_DIAMETER = 5.0
CABLE_NOTCH_WIDTH = 32.0
CABLE_NOTCH_HEIGHT = 14.0


def beam(x_size: float, y: float, z: float) -> m3d.Manifold:
    """Small full-width beam used as a stable convex-hull vertex."""
    return m3d.Manifold.cube((x_size, 0.2, 0.2)).translate((0.0, y, z))


def wedge(width: float, x_offset: float, profile: list[tuple[float, float]]) -> m3d.Manifold:
    solid = m3d.Manifold.batch_hull([beam(width, y, z) for y, z in profile])
    return solid.translate((x_offset, 0.0, 0.0))


def panel_feature(feature: m3d.Manifold, x: float, vertical: float,
                  normal: float) -> m3d.Manifold:
    """Place local panel geometry onto the 30-degree-from-vertical face."""
    return feature.translate((x, vertical, normal)).rotate((
        90.0 + PANEL_TILT_FROM_VERTICAL, 0.0, 0.0,
    )).translate((
        3.0 + PANEL_CLEARANCE / 2.0,
        55.0,
        LOWER_EDGE_HEIGHT,
    ))


def build_pod() -> m3d.Manifold:
    run = PANEL_HEIGHT * math.sin(math.radians(PANEL_TILT_FROM_VERTICAL))
    rise = PANEL_HEIGHT * math.cos(math.radians(PANEL_TILT_FROM_VERTICAL))
    bottom_front = (55.0, LOWER_EDGE_HEIGHT)
    top_front = (55.0 - run, LOWER_EDGE_HEIGHT + rise)

    outer = wedge(BODY_WIDTH, 0.0, [
        (0.0, 0.0), bottom_front, top_front, (0.0, WOOD_SIDE_HEIGHT - 0.2),
    ])

    # The cavity passes through the wooden side and inclined front, leaving a
    # 7 mm front rim plus 3 mm side/top/bottom walls.
    inset_rise = FACE_RIM * math.cos(math.radians(PANEL_TILT_FROM_VERTICAL))
    inset_run = FACE_RIM * math.sin(math.radians(PANEL_TILT_FROM_VERTICAL))
    cavity = wedge(BODY_WIDTH - 2.0 * FACE_RIM, FACE_RIM, [
        (-1.0, WALL),
        (bottom_front[0] + 5.0 - inset_run, bottom_front[1] + inset_rise),
        (top_front[0] + 5.0 + inset_run, top_front[1] - inset_rise),
        (-1.0, WOOD_SIDE_HEIGHT - WALL),
    ])
    pod = outer - cavity

    # Faceplate fasteners follow the inclined panel normal and match its holes.
    face_holes: list[m3d.Manifold] = []
    for x in (5.0, PANEL_WIDTH - 5.0):
        for y in (5.0, PANEL_HEIGHT - 5.0):
            hole = m3d.Manifold.cylinder(12.0, M3_CLEARANCE_DIAMETER / 2.0,
                                         circular_segments=CIRCLE_SEGMENTS)
            face_holes.append(panel_feature(hole, x, y, -3.0))
    pod -= m3d.Manifold.batch_boolean(face_holes, m3d.OpType.Add)

    # External rear ears allow the pod to screw directly to the wooden side.
    ears: list[m3d.Manifold] = []
    ear_holes: list[m3d.Manifold] = []
    for x in (0.0, BODY_WIDTH - MOUNT_EAR_WIDTH):
        for z in (8.0, WOOD_SIDE_HEIGHT - MOUNT_EAR_HEIGHT - 8.0):
            ears.append(m3d.Manifold.cube((MOUNT_EAR_WIDTH, MOUNT_EAR_THICKNESS,
                                           MOUNT_EAR_HEIGHT)).translate((x, -MOUNT_EAR_THICKNESS, z)))
            ear_holes.append(m3d.Manifold.cylinder(MOUNT_EAR_THICKNESS + 2.0,
                                                   WOOD_SCREW_DIAMETER / 2.0,
                                                   circular_segments=CIRCLE_SEGMENTS)
                             .rotate((90.0, 0.0, 0.0))
                             .translate((x + MOUNT_EAR_WIDTH / 2.0, 1.0,
                                         z + MOUNT_EAR_HEIGHT / 2.0)))
    pod += m3d.Manifold.batch_boolean(ears, m3d.OpType.Add)
    pod -= m3d.Manifold.batch_boolean(ear_holes, m3d.OpType.Add)

    # A centered lower notch allows harnesses to leave the open back downward.
    notch = m3d.Manifold.cube((CABLE_NOTCH_WIDTH, 12.0, CABLE_NOTCH_HEIGHT)).translate((
        (BODY_WIDTH - CABLE_NOTCH_WIDTH) / 2.0, -2.0, -1.0,
    ))
    return pod - notch


def write_binary_stl(path: Path, solid: m3d.Manifold) -> None:
    mesh = solid.to_mesh()
    vertices = mesh.vert_properties[:, :3]
    triangles = mesh.tri_verts
    with path.open("wb") as out:
        out.write(b"Crosswind Alpha angled control pod".ljust(80, b"\0"))
        out.write(struct.pack("<I", len(triangles)))
        for indices in triangles:
            points = [vertices[int(i)] for i in indices]
            a, b, c = points
            ux, uy, uz = b - a
            vx, vy, vz = c - a
            normal = (uy * vz - uz * vy, uz * vx - ux * vz, ux * vy - uy * vx)
            length = math.sqrt(sum(float(value) ** 2 for value in normal))
            normal = tuple(float(value) / length for value in normal) if length else (0.0, 0.0, 0.0)
            values = (*normal, *(float(value) for point in points for value in point))
            out.write(struct.pack("<12fH", *values, 0))


if __name__ == "__main__":
    pod = build_pod()
    if pod.is_empty() or pod.status() != m3d.Error.NoError:
        raise RuntimeError(f"Pod generation failed: {pod.status()}")
    destination = Path(__file__).resolve().parent / "crosswind_alpha_control_pod.stl"
    write_binary_stl(destination, pod)
    print(f"Generated {destination.name}: {pod.num_tri()} triangles, {pod.volume():.1f} mm^3")

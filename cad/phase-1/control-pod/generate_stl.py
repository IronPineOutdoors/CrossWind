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
PANEL_TILT_FROM_VERTICAL = 47.0
PANEL_TOP_PROJECTION = 5.0
PANEL_BOTTOM_PROJECTION = PANEL_TOP_PROJECTION + PANEL_HEIGHT * math.sin(
    math.radians(PANEL_TILT_FROM_VERTICAL)
)
PANEL_CLEARANCE = 0.6
BODY_WIDTH = PANEL_WIDTH + 2.0 * (3.0 + PANEL_CLEARANCE / 2.0)
WOOD_SIDE_HEIGHT = 101.6  # 4 inches
WOOD_TOP_THICKNESS = 19.05  # 3/4 inch; recorded as an overhead keep-out.
WALL = 3.0
FACE_RIM = 7.0
LOWER_EDGE_HEIGHT = 7.5
M3_PILOT_DIAMETER = 2.7
M3_BOSS_DIAMETER = 9.0
M3_BOSS_DEPTH = 8.0
CIRCLE_SEGMENTS = 64

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
        PANEL_BOTTOM_PROJECTION,
        LOWER_EDGE_HEIGHT,
    ))


def build_pod() -> m3d.Manifold:
    run = PANEL_HEIGHT * math.sin(math.radians(PANEL_TILT_FROM_VERTICAL))
    rise = PANEL_HEIGHT * math.cos(math.radians(PANEL_TILT_FROM_VERTICAL))
    bottom_front = (PANEL_BOTTOM_PROJECTION, LOWER_EDGE_HEIGHT)
    top_front = (PANEL_BOTTOM_PROJECTION - run, LOWER_EDGE_HEIGHT + rise)

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

    # Reinforced pilot-hole bosses receive the faceplate's four M3 screws.
    face_bosses: list[m3d.Manifold] = []
    face_holes: list[m3d.Manifold] = []
    for x in (5.0, PANEL_WIDTH - 5.0):
        for y in (5.0, PANEL_HEIGHT - 5.0):
            boss = m3d.Manifold.cylinder(M3_BOSS_DEPTH, M3_BOSS_DIAMETER / 2.0,
                                         circular_segments=CIRCLE_SEGMENTS)
            face_bosses.append(panel_feature(boss, x, y, 0.0))
            # Extend past both ends of the boss. A cutter ending coplanar with
            # the boss left microscopic downward-facing caps that Bambu Studio
            # reported as floating cantilevers.
            hole = m3d.Manifold.cylinder(100.0, M3_PILOT_DIAMETER / 2.0,
                                         circular_segments=CIRCLE_SEGMENTS)
            face_holes.append(panel_feature(hole, x, y, -2.0))
    pod += m3d.Manifold.batch_boolean(face_bosses, m3d.OpType.Add)
    pod -= m3d.Manifold.batch_boolean(face_holes, m3d.OpType.Add)

    # A centered lower notch allows harnesses to leave the open back downward.
    notch = m3d.Manifold.cube((CABLE_NOTCH_WIDTH, 12.0, CABLE_NOTCH_HEIGHT)).translate((
        (BODY_WIDTH - CABLE_NOTCH_WIDTH) / 2.0, -2.0, -1.0,
    ))
    pod -= notch

    # Save the STL with its inclined face rim on the build plate. This is the
    # inverse of the installed panel transform and avoids printing the front
    # shell as a large unsupported cantilever.
    print_ready = pod.translate((
        -(3.0 + PANEL_CLEARANCE / 2.0), -PANEL_BOTTOM_PROJECTION, -LOWER_EDGE_HEIGHT,
    )).rotate((-(90.0 + PANEL_TILT_FROM_VERTICAL), 0.0, 0.0))
    bounds = print_ready.bounding_box()
    if bounds[2] < 0.0:
        below_face = m3d.Manifold.cube((
            bounds[3] - bounds[0] + 2.0,
            bounds[4] - bounds[1] + 2.0,
            -bounds[2] + 1.0,
        )).translate((bounds[0] - 1.0, bounds[1] - 1.0, bounds[2] - 1.0))
        print_ready -= below_face
    bounds = print_ready.bounding_box()
    return print_ready.translate((-bounds[0], -bounds[1], -bounds[2]))


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

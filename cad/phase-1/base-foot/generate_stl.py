#!/usr/bin/env python3
"""Generate one wide bolt-through foot for the Crosswind wooden base."""

from __future__ import annotations

import math
import struct
from pathlib import Path

import manifold3d as m3d

FOOT_WIDTH = 50.0
FOOT_DEPTH = 50.0
FOOT_HEIGHT = 20.0
CORNER_RADIUS = 6.0
THROUGH_HOLE_DIAMETER = 6.5  # M6 / 1/4-inch-class fastener clearance.
WASHER_RECESS_DIAMETER = 20.0
WASHER_RECESS_DEPTH = 5.0
RUBBER_PAD_WIDTH = 40.0
RUBBER_PAD_DEPTH = 40.0
RUBBER_PAD_RECESS = 2.0
CIRCLE_SEGMENTS = 64


def rounded_box(width: float, depth: float, height: float, radius: float) -> m3d.Manifold:
    posts = []
    for x in (radius, width - radius):
        for y in (radius, depth - radius):
            posts.append(m3d.Manifold.cylinder(height, radius,
                                               circular_segments=CIRCLE_SEGMENTS)
                         .translate((x, y, 0.0)))
    return m3d.Manifold.batch_hull(posts)


def build_foot() -> m3d.Manifold:
    foot = rounded_box(FOOT_WIDTH, FOOT_DEPTH, FOOT_HEIGHT, CORNER_RADIUS)
    center = (FOOT_WIDTH / 2.0, FOOT_DEPTH / 2.0)
    through = m3d.Manifold.cylinder(FOOT_HEIGHT + 2.0, THROUGH_HOLE_DIAMETER / 2.0,
                                    circular_segments=CIRCLE_SEGMENTS).translate((*center, -1.0))
    washer = m3d.Manifold.cylinder(WASHER_RECESS_DEPTH + 1.0, WASHER_RECESS_DIAMETER / 2.0,
                                   circular_segments=CIRCLE_SEGMENTS).translate((
                                       *center, FOOT_HEIGHT - WASHER_RECESS_DEPTH,
                                   ))
    pad = m3d.Manifold.cube((RUBBER_PAD_WIDTH, RUBBER_PAD_DEPTH,
                             RUBBER_PAD_RECESS + 0.1)).translate((
                                 (FOOT_WIDTH - RUBBER_PAD_WIDTH) / 2.0,
                                 (FOOT_DEPTH - RUBBER_PAD_DEPTH) / 2.0,
                                 -0.1,
                             ))
    return foot - m3d.Manifold.batch_boolean([through, washer, pad], m3d.OpType.Add)


def write_binary_stl(path: Path, solid: m3d.Manifold) -> None:
    mesh = solid.to_mesh()
    vertices = mesh.vert_properties[:, :3]
    triangles = mesh.tri_verts
    with path.open("wb") as out:
        out.write(b"Crosswind Alpha wide base foot".ljust(80, b"\0"))
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
    foot = build_foot()
    if foot.is_empty() or foot.status() != m3d.Error.NoError:
        raise RuntimeError(f"Foot generation failed: {foot.status()}")
    destination = Path(__file__).resolve().parent / "crosswind_alpha_base_foot.stl"
    write_binary_stl(destination, foot)
    print(f"Generated {destination.name}: {foot.num_tri()} triangles, {foot.volume():.1f} mm^3")

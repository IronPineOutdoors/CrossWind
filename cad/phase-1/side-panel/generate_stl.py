#!/usr/bin/env python3
"""Generate the provisional Crosswind Alpha side panel.

Requires manifold3d (`python -m pip install manifold3d`). Dimensions are mm.
All assumptions are constants below so measured revisions remain straightforward.
"""

from __future__ import annotations

import math
import struct
from pathlib import Path

import manifold3d as m3d

# Provisional panel envelope; replace with enclosure measurements.
PANEL_WIDTH = 150.0
PANEL_HEIGHT = 100.0
PANEL_THICKNESS = 3.0
CORNER_HOLE_INSET = 5.0
M3_CLEARANCE_DIAMETER = 3.2

# OLED PCB: 73 W x 43 H, left edge 10 and top edge 15 from panel top.
OLED_PCB_LEFT = 10.0
OLED_PCB_BOTTOM = PANEL_HEIGHT - 15.0 - 43.0
OLED_WINDOW_WIDTH = 57.0
OLED_WINDOW_HEIGHT = 28.0

# Reported mounting spans are outside-to-outside. Subtract the clearance-hole
# diameter to obtain provisional center spans; verify against the physical PCB.
OLED_HOLE_X_SPAN = 70.0 - M3_CLEARANCE_DIAMETER
OLED_HOLE_Y_SPAN = 42.0 - M3_CLEARANCE_DIAMETER

ARM_CENTER = (46.5, 20.75)
FIRE_CENTER = (ARM_CENTER[0] + 70.0, ARM_CENTER[1])
ENCODER_CENTER = (105.0, OLED_PCB_BOTTOM + 43.0 / 2.0)
LED_CENTER = ((ARM_CENTER[0] + FIRE_CENTER[0]) / 2.0, OLED_PCB_BOTTOM - 6.0)

BUTTON_DIAMETER = 16.5
ENCODER_DIAMETER = 6.0
LED_DIAMETER = 5.2  # Provisional lens clearance; replace from LED measurement.
CIRCLE_SEGMENTS = 64


def cutter(diameter: float, center: tuple[float, float]) -> m3d.Manifold:
    return m3d.Manifold.cylinder(
        PANEL_THICKNESS + 2.0, diameter / 2.0,
        circular_segments=CIRCLE_SEGMENTS,
    ).translate((center[0], center[1], -1.0))


def build_panel() -> m3d.Manifold:
    panel = m3d.Manifold.cube((PANEL_WIDTH, PANEL_HEIGHT, PANEL_THICKNESS))

    holes: list[m3d.Manifold] = []
    for x in (CORNER_HOLE_INSET, PANEL_WIDTH - CORNER_HOLE_INSET):
        for y in (CORNER_HOLE_INSET, PANEL_HEIGHT - CORNER_HOLE_INSET):
            holes.append(cutter(M3_CLEARANCE_DIAMETER, (x, y)))

    pcb_center = (OLED_PCB_LEFT + 73.0 / 2.0, OLED_PCB_BOTTOM + 43.0 / 2.0)
    for dx in (-OLED_HOLE_X_SPAN / 2.0, OLED_HOLE_X_SPAN / 2.0):
        for dy in (-OLED_HOLE_Y_SPAN / 2.0, OLED_HOLE_Y_SPAN / 2.0):
            holes.append(cutter(M3_CLEARANCE_DIAMETER, (pcb_center[0] + dx, pcb_center[1] + dy)))

    window_left = pcb_center[0] - OLED_WINDOW_WIDTH / 2.0
    window_bottom = pcb_center[1] - OLED_WINDOW_HEIGHT / 2.0
    holes.append(m3d.Manifold.cube((OLED_WINDOW_WIDTH, OLED_WINDOW_HEIGHT, PANEL_THICKNESS + 2.0))
                 .translate((window_left, window_bottom, -1.0)))

    holes.extend((
        cutter(BUTTON_DIAMETER, ARM_CENTER),
        cutter(BUTTON_DIAMETER, FIRE_CENTER),
        cutter(ENCODER_DIAMETER, ENCODER_CENTER),
        cutter(LED_DIAMETER, LED_CENTER),
    ))
    return panel - m3d.Manifold.batch_boolean(holes, m3d.OpType.Add)


def write_binary_stl(path: Path, solid: m3d.Manifold) -> None:
    mesh = solid.to_mesh()
    vertices = mesh.vert_properties[:, :3]
    triangles = mesh.tri_verts
    with path.open("wb") as out:
        out.write(b"Crosswind Alpha provisional side panel".ljust(80, b"\0"))
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
    panel = build_panel()
    if panel.is_empty() or panel.status() != m3d.Error.NoError:
        raise RuntimeError(f"Panel generation failed: {panel.status()}")
    destination = Path(__file__).resolve().parent / "crosswind_alpha_side_panel.stl"
    write_binary_stl(destination, panel)
    print(f"Generated {destination.name}: {panel.num_tri()} triangles, {panel.volume():.1f} mm^3")

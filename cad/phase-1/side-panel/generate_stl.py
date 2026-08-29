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
# Fit-check correction: increase the horizontal center span by 2 mm, moving
# each left hole 1 mm left and each right hole 1 mm right.
OLED_HOLE_X_SPAN += 2.0

ARM_CENTER = (46.5, 20.75)
FIRE_CENTER = (ARM_CENTER[0] + 70.0, ARM_CENTER[1])
ENCODER_CENTER = (105.0, OLED_PCB_BOTTOM + 43.0 / 2.0)
LED_CENTER = ((ARM_CENTER[0] + FIRE_CENTER[0]) / 2.0, OLED_PCB_BOTTOM - 6.0)

BUTTON_DIAMETER = 16.5
ENCODER_DIAMETER = 6.0
LED_DIAMETER = 5.2  # Provisional lens clearance; replace from LED measurement.
CIRCLE_SEGMENTS = 64
ENGRAVING_DEPTH = 0.6

# Compact print-safe bitmap alphabets. A filled character pixel becomes one
# engraving stroke; this avoids relying on a locally installed font.
FONT_5X7 = {
    "A": ("01110", "10001", "10001", "11111", "10001", "10001", "10001"),
    "B": ("11110", "10001", "10001", "11110", "10001", "10001", "11110"),
    "C": ("01111", "10000", "10000", "10000", "10000", "10000", "01111"),
    "D": ("11110", "10001", "10001", "10001", "10001", "10001", "11110"),
    "I": ("11111", "00100", "00100", "00100", "00100", "00100", "11111"),
    "L": ("10000", "10000", "10000", "10000", "10000", "10000", "11111"),
    "N": ("10001", "11001", "11001", "10101", "10011", "10011", "10001"),
    "O": ("01110", "10001", "10001", "10001", "10001", "10001", "01110"),
    "R": ("11110", "10001", "10001", "11110", "10100", "10010", "10001"),
    "S": ("01111", "10000", "10000", "01110", "00001", "00001", "11110"),
    "T": ("11111", "00100", "00100", "00100", "00100", "00100", "00100"),
    "W": ("10001", "10001", "10001", "10101", "10101", "11011", "10001"),
}

FONT_3X5 = {
    "A": ("010", "101", "111", "101", "101"), "B": ("110", "101", "110", "101", "110"),
    "D": ("110", "101", "101", "101", "110"), "E": ("111", "100", "110", "100", "111"),
    "F": ("111", "100", "110", "100", "100"),
    "I": ("111", "010", "010", "010", "111"), "N": ("101", "111", "111", "111", "101"),
    "M": ("101", "111", "111", "101", "101"),
    "O": ("010", "101", "101", "101", "010"), "P": ("110", "101", "110", "100", "100"),
    "R": ("110", "101", "110", "101", "101"), "S": ("011", "100", "010", "001", "110"),
    "T": ("111", "010", "010", "010", "010"), "U": ("101", "101", "101", "101", "111"),
    "Y": ("101", "101", "010", "010", "010"), " ": ("000", "000", "000", "000", "000"),
}


def cutter(diameter: float, center: tuple[float, float]) -> m3d.Manifold:
    return m3d.Manifold.cylinder(
        PANEL_THICKNESS + 2.0, diameter / 2.0,
        circular_segments=CIRCLE_SEGMENTS,
    ).translate((center[0], center[1], -1.0))


def engraved_text(text: str, font: dict[str, tuple[str, ...]], pixel: float,
                  origin: tuple[float, float]) -> list[m3d.Manifold]:
    """Return top-face engraving cutters for a fixed-width bitmap wordmark."""
    glyph_width = len(next(iter(font.values()))[0])
    cutters: list[m3d.Manifold] = []
    for char_index, char in enumerate(text):
        glyph = font[char]
        for row, bits in enumerate(glyph):
            for column, bit in enumerate(bits):
                if bit == "1":
                    x = origin[0] + (char_index * (glyph_width + 1) + column) * pixel
                    y = origin[1] + (len(glyph) - 1 - row) * pixel
                    cutters.append(m3d.Manifold.cube((pixel, pixel, ENGRAVING_DEPTH + 0.1))
                                   .translate((x, y, PANEL_THICKNESS - ENGRAVING_DEPTH)))
    return cutters


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
    # Upper-right recessed product/master-brand lockup. Its 0.6 mm depth leaves
    # 2.4 mm of the panel intact and clears the corner fastener and controls.
    holes.extend(engraved_text("CROSSWIND", FONT_5X7, 1.0, (88.0, 84.0)))
    holes.extend(engraved_text("BY IRON PINE OUTDOORS", FONT_3X5, 0.55, (90.0, 79.0)))
    holes.extend(engraved_text("ARM", FONT_3X5, 0.7, (42.65, 31.0)))
    holes.extend(engraved_text("FIRE", FONT_3X5, 0.7, (111.25, 31.0)))
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

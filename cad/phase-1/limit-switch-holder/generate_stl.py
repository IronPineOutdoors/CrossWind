#!/usr/bin/env python3
"""Generate printable Crosswind YL-99 adjustable limit-switch holders.

No third-party CAD packages are required. Dimensions are millimetres.
"""

from pathlib import Path
from typing import Iterable

Vec3 = tuple[float, float, float]
Triangle = tuple[Vec3, Vec3, Vec3]


def box(x0: float, y0: float, z0: float, x1: float, y1: float, z1: float) -> list[Triangle]:
    p = [
        (x0, y0, z0), (x1, y0, z0), (x1, y1, z0), (x0, y1, z0),
        (x0, y0, z1), (x1, y0, z1), (x1, y1, z1), (x0, y1, z1),
    ]
    faces = [(0, 2, 1), (0, 3, 2), (4, 5, 6), (4, 6, 7),
             (0, 1, 5), (0, 5, 4), (1, 2, 6), (1, 6, 5),
             (2, 3, 7), (2, 7, 6), (3, 0, 4), (3, 4, 7)]
    return [(p[a], p[b], p[c]) for a, b, c in faces]


def triangular_prism(x0: float, x1: float, y0: float, y1: float,
                     z0: float, z1: float) -> list[Triangle]:
    """Right-triangle gusset, extruded along X."""
    p = [(x0, y0, z0), (x0, y1, z0), (x0, y1, z1),
         (x1, y0, z0), (x1, y1, z0), (x1, y1, z1)]
    faces = [(0, 2, 1), (3, 4, 5), (0, 1, 4), (0, 4, 3),
             (1, 2, 5), (1, 5, 4), (2, 0, 3), (2, 3, 5)]
    return [(p[a], p[b], p[c]) for a, b, c in faces]


def holder(offset_x: float = 0.0) -> list[Triangle]:
    tris: list[Triangle] = []
    add = lambda part: tris.extend(part)

    # Compact 38 x 38 x 4 base, assembled around two 4.5 x 20 mm slots.
    # The slots provide more than the module's approximately 2 mm click travel.
    add(box(-19, 0, 0, 19, 8, 4))
    add(box(-19, 28, 0, 19, 38, 4))
    add(box(-19, 8, 0, -13, 28, 4))
    add(box(-8.5, 8, 0, 8.5, 28, 4))
    add(box(13, 8, 0, 19, 28, 4))

    # The 26 mm wide upright clears a 14 mm PCB and its offset mounting hole.
    # A 3.6 x 16 mm slot fits the measured 3 mm hole and lets the lever height
    # be set without relying on nominal dimensions from other YL-99 variants.
    add(box(-13, 34, 4, -1.8, 38, 42))
    add(box(1.8, 34, 4, 13, 38, 42))
    add(box(-1.8, 34, 4, 1.8, 38, 9))
    add(box(-1.8, 34, 25, 1.8, 38, 42))

    # Two 4 mm gussets resist vibration and switch-actuation loads.
    add(triangular_prism(-12, -9, 23, 34, 4, 19))
    add(triangular_prism(9, 12, 23, 34, 4, 19))

    if offset_x:
        return [tuple((x + offset_x, y, z) for x, y, z in tri) for tri in tris]  # type: ignore[return-value]
    return tris


def normal(tri: Triangle) -> Vec3:
    a, b, c = tri
    u = tuple(b[i] - a[i] for i in range(3))
    v = tuple(c[i] - a[i] for i in range(3))
    n = (u[1]*v[2] - u[2]*v[1], u[2]*v[0] - u[0]*v[2], u[0]*v[1] - u[1]*v[0])
    length = sum(q*q for q in n) ** 0.5
    return tuple(q / length for q in n) if length else (0.0, 0.0, 0.0)  # type: ignore[return-value]


def write_ascii_stl(path: Path, name: str, triangles: Iterable[Triangle]) -> None:
    lines = [f"solid {name}"]
    for tri in triangles:
        nx, ny, nz = normal(tri)
        lines.append(f"  facet normal {nx:.6g} {ny:.6g} {nz:.6g}")
        lines.append("    outer loop")
        lines.extend(f"      vertex {x:.6g} {y:.6g} {z:.6g}" for x, y, z in tri)
        lines.extend(("    endloop", "  endfacet"))
    lines.append(f"endsolid {name}")
    path.write_text("\n".join(lines) + "\n", encoding="ascii")


if __name__ == "__main__":
    output = Path(__file__).resolve().parent
    write_ascii_stl(output / "crosswind_yl99_holder.stl", "crosswind_yl99_holder", holder())
    pair = holder(-21) + holder(21)
    write_ascii_stl(output / "crosswind_yl99_holder_pair.stl", "crosswind_yl99_holder_pair", pair)
    print("Generated single-holder and two-holder STL files.")

#!/usr/bin/env python3
"""Generate adjustable YL-99 striker flags for Crosswind Alpha."""

from pathlib import Path
from typing import Iterable

Vec3 = tuple[float, float, float]
Triangle = tuple[Vec3, Vec3, Vec3]


def box(x0: float, y0: float, z0: float, x1: float, y1: float, z1: float) -> list[Triangle]:
    p = [(x0, y0, z0), (x1, y0, z0), (x1, y1, z0), (x0, y1, z0),
         (x0, y0, z1), (x1, y0, z1), (x1, y1, z1), (x0, y1, z1)]
    faces = [(0, 2, 1), (0, 3, 2), (4, 5, 6), (4, 6, 7),
             (0, 1, 5), (0, 5, 4), (1, 2, 6), (1, 6, 5),
             (2, 3, 7), (2, 7, 6), (3, 0, 4), (3, 4, 7)]
    return [(p[a], p[b], p[c]) for a, b, c in faces]


def triangular_prism(x0: float, x1: float, y0: float, y1: float,
                     z0: float, z1: float) -> list[Triangle]:
    p = [(x0, y0, z0), (x0, y1, z0), (x0, y1, z1),
         (x1, y0, z0), (x1, y1, z0), (x1, y1, z1)]
    faces = [(0, 2, 1), (3, 4, 5), (0, 1, 4), (0, 4, 3),
             (1, 2, 5), (1, 5, 4), (2, 0, 3), (2, 3, 5)]
    return [(p[a], p[b], p[c]) for a, b, c in faces]


def flag(offset_x: float = 0.0) -> list[Triangle]:
    tris: list[Triangle] = []
    add = lambda part: tris.extend(part)

    # 34 x 34 x 4 mounting flange around two 4.5 x 21 mm slots.
    add(box(-17, 0, 0, 17, 6, 4))
    add(box(-17, 27, 0, 17, 34, 4))
    add(box(-17, 6, 0, -11.25, 27, 4))
    add(box(-6.75, 6, 0, 6.75, 27, 4))
    add(box(11.25, 6, 0, 17, 27, 4))

    # A 20 x 32 mm striker face presents a broad, flat target to the lever.
    add(box(-10, 30, 4, 10, 34, 36))
    add(triangular_prism(-10, -7, 20, 30, 4, 17))
    add(triangular_prism(7, 10, 20, 30, 4, 17))

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


def write_stl(path: Path, name: str, triangles: Iterable[Triangle]) -> None:
    lines = [f"solid {name}"]
    for tri in triangles:
        nx, ny, nz = normal(tri)
        lines.extend((f"  facet normal {nx:.6g} {ny:.6g} {nz:.6g}", "    outer loop"))
        lines.extend(f"      vertex {x:.6g} {y:.6g} {z:.6g}" for x, y, z in tri)
        lines.extend(("    endloop", "  endfacet"))
    lines.append(f"endsolid {name}")
    path.write_text("\n".join(lines) + "\n", encoding="ascii")


if __name__ == "__main__":
    output = Path(__file__).resolve().parent
    write_stl(output / "crosswind_limit_flag.stl", "crosswind_limit_flag", flag())
    write_stl(output / "crosswind_limit_flag_pair.stl", "crosswind_limit_flag_pair",
              flag(-19) + flag(19))
    print("Generated single-flag and two-flag STL files.")

from __future__ import annotations

import struct
from dataclasses import dataclass
from pathlib import Path


SECTION_GEOMETRY = 15
SECTION_GEOMETRY_LIST = 26
SECTION_FRAME_LIST = 14


@dataclass
class Chunk:
    offset: int
    code: int
    size: int
    lib: int
    body: int
    end: int


def version_from_lib(lib: int) -> int:
    if lib & 0xFFFF0000:
        return ((lib >> 14) & 0x3FF00) | ((lib >> 16) & 0x3F) | 0x30000
    return lib << 8


def iter_chunks(raw: bytes, start: int, end: int):
    off = start
    while off + 12 <= end:
        code, size, lib = struct.unpack_from("<III", raw, off)
        body = off + 12
        chunk_end = body + size
        yield Chunk(off, code, size, lib, body, chunk_end)
        off = chunk_end


def walk_chunks(raw: bytes, start: int, end: int):
    for chunk in iter_chunks(raw, start, end):
        yield chunk
        if chunk.code in (16, SECTION_GEOMETRY_LIST, 25):  # clump / geometry list / atomic
            yield from walk_chunks(raw, chunk.body, chunk.end)


def find_geometry_chunks(raw: bytes) -> list[Chunk]:
    geometries = [chunk for chunk in walk_chunks(raw, 0, len(raw)) if chunk.code == SECTION_GEOMETRY]
    if not geometries:
        raise ValueError("geometry chunk not found")
    return geometries


def find_frame_list_chunks(raw: bytes) -> list[Chunk]:
    return [chunk for chunk in walk_chunks(raw, 0, len(raw)) if chunk.code == SECTION_FRAME_LIST]


def scale_dff(path: Path, scale: float, z_floor: float = 0.15, output: Path | None = None) -> None:
    raw = bytearray(path.read_bytes())
    min_x = max_x = None
    min_y = max_y = None
    # Scale the frame hierarchy first so all sub-parts of the vehicle move outward
    # together. In a GTA SA vehicle DFF, each frame stores a local translation
    # vector for a component (wheel, door, bumper, etc.). Scaling those vectors
    # keeps the pieces aligned with the stretched geometry.
    for frame_list in find_frame_list_chunks(raw):
        struct_off = frame_list.body
        struct_code, struct_size, struct_lib = struct.unpack_from("<III", raw, struct_off)
        if struct_code != 1:
            continue

        body = struct_off + 12
        version = version_from_lib(struct_lib)
        if version < 0x34000:
            continue

        num_frames = struct.unpack_from("<I", raw, body)[0]
        frame_off = body + 4
        frame_size = 56
        needed = frame_off + num_frames * frame_size
        if needed > frame_list.end:
            continue

        for i in range(num_frames):
            base = frame_off + i * frame_size
            # Frame layout: 3x3 orientation matrix followed by local translation,
            # then parent index and flags. Only the translation is scaled.
            px, py, pz = struct.unpack_from("<fff", raw, base + 36)
            min_x = px if min_x is None else min(min_x, px)
            max_x = px if max_x is None else max(max_x, px)
            min_y = py if min_y is None else min(min_y, py)
            max_y = py if max_y is None else max(max_y, py)
            struct.pack_into("<fff", raw, base + 36, px * scale, py * scale, pz * scale)

    for geom in find_geometry_chunks(raw):
        # Geometry chunks in GTA SA are stored as a struct chunk inside the geometry
        # list. The actual geometry fields begin after that nested struct header.
        struct_off = geom.body
        struct_code, _, struct_lib = struct.unpack_from("<III", raw, struct_off)
        if struct_code != 1:
            continue

        body = struct_off + 12
        version = version_from_lib(struct_lib)
        fmt, num_triangles, num_vertices, num_morph_targets = struct.unpack_from("<IIII", raw, body)
        is_native = bool(fmt & 0x0100_0000)
        is_prelit = bool(fmt & 0x0000_0008)
        is_textured = bool(fmt & 0x0000_0004)
        is_textured2 = bool(fmt & 0x0000_0080)
        num_uv_layers_raw = (fmt >> 16) & 0xFF
        num_uv_layers = num_uv_layers_raw if num_uv_layers_raw else (2 if is_textured2 else 1 if is_textured else 0)

        off = body + 16
        if version < 0x34000:
            off += 12
        if not is_native:
            if is_prelit:
                off += 4 * num_vertices
            off += num_uv_layers * num_vertices * 8
            off += num_triangles * 8

        # Measure the lowest point before writing the new positions.
        min_z = None
        scan_off = off
        for _ in range(num_morph_targets):
            has_vertices = struct.unpack_from("<I", raw, scan_off + 16)[0]
            has_normals = struct.unpack_from("<I", raw, scan_off + 20)[0]
            vert_off = scan_off + 24
            if has_vertices:
                for i in range(num_vertices):
                    vx, vy, vz = struct.unpack_from("<fff", raw, vert_off + i * 12)
                    min_x = vx if min_x is None else min(min_x, vx)
                    max_x = vx if max_x is None else max(max_x, vx)
                    min_y = vy if min_y is None else min(min_y, vy)
                    max_y = vy if max_y is None else max(max_y, vy)
                    min_z = vz if min_z is None else min(min_z, vz)
                vert_off += num_vertices * 12
            if has_normals:
                vert_off += num_vertices * 12
            scan_off = vert_off

        center_x = 0.0 if min_x is None or max_x is None else (min_x + max_x) * 0.5
        center_y = 0.0 if min_y is None or max_y is None else (min_y + max_y) * 0.5
        z_shift = 0.0 if min_z is None else (z_floor - min_z)

        # Apply scale and lift in one pass.
        scan_off = off
        for _ in range(num_morph_targets):
            sphere_off = scan_off
            x, y, z, radius = struct.unpack_from("<ffff", raw, sphere_off)
            struct.pack_into(
                "<ffff",
                raw,
                sphere_off,
                center_x + (x - center_x) * scale,
                center_y + (y - center_y) * scale,
                (z * scale) + z_shift,
                radius * scale,
            )

            has_vertices = struct.unpack_from("<I", raw, sphere_off + 16)[0]
            has_normals = struct.unpack_from("<I", raw, sphere_off + 20)[0]
            vert_off = sphere_off + 24
            if has_vertices:
                for i in range(num_vertices):
                    vx, vy, vz = struct.unpack_from("<fff", raw, vert_off + i * 12)
                    struct.pack_into(
                        "<fff",
                        raw,
                        vert_off + i * 12,
                        center_x + (vx - center_x) * scale,
                        center_y + (vy - center_y) * scale,
                        (vz * scale) + z_shift,
                    )
                vert_off += num_vertices * 12
            if has_normals:
                vert_off += num_vertices * 12
            scan_off = vert_off

    target = output or path
    target.write_bytes(raw)


if __name__ == "__main__":
    import sys

    if len(sys.argv) < 3:
        raise SystemExit("usage: scale_renderware_dff.py <scale> <input_file> [<output_file>]")

    scale = float(sys.argv[1])
    input_path = Path(sys.argv[2])
    output_path = Path(sys.argv[3]) if len(sys.argv) >= 4 else None
    scale_dff(input_path, scale, output=output_path)

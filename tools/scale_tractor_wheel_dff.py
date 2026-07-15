from __future__ import annotations

import struct
from dataclasses import dataclass
from pathlib import Path


SECTION_GEOMETRY = 15
SECTION_GEOMETRY_LIST = 26


@dataclass
class Chunk:
    offset: int
    code: int
    size: int
    lib: int
    body: int
    end: int


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
        if chunk.code in (16, SECTION_GEOMETRY_LIST, 25):
            yield from walk_chunks(raw, chunk.body, chunk.end)


def find_geometry_chunks(raw: bytes) -> list[Chunk]:
    geometries = [chunk for chunk in walk_chunks(raw, 0, len(raw)) if chunk.code == SECTION_GEOMETRY]
    if not geometries:
        raise ValueError("geometry chunk not found")
    return geometries


def version_from_lib(lib: int) -> int:
    if lib & 0xFFFF0000:
        return ((lib >> 14) & 0x3FF00) | ((lib >> 16) & 0x3F) | 0x30000
    return lib << 8


def scale_tractor_wheel(path: Path, scale: float, output: Path | None = None) -> None:
    raw = bytearray(path.read_bytes())
    geometries = find_geometry_chunks(raw)

    # In the tractor DFF, the first geometry chunk is the wheel mesh.
    # We only scale that single chunk so the body and other parts stay unchanged.
    target = geometries[0]
    struct_off = target.body
    struct_code, _, struct_lib = struct.unpack_from("<III", raw, struct_off)
    if struct_code != 1:
        raise ValueError("unexpected geometry struct chunk")

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

    scan_off = off
    for _ in range(num_morph_targets):
        sphere_off = scan_off
        x, y, z, radius = struct.unpack_from("<ffff", raw, sphere_off)
        struct.pack_into("<ffff", raw, sphere_off, x * scale, y * scale, z * scale, radius * scale)

        has_vertices = struct.unpack_from("<I", raw, sphere_off + 16)[0]
        has_normals = struct.unpack_from("<I", raw, sphere_off + 20)[0]
        vert_off = sphere_off + 24
        if has_vertices:
            for i in range(num_vertices):
                vx, vy, vz = struct.unpack_from("<fff", raw, vert_off + i * 12)
                struct.pack_into("<fff", raw, vert_off + i * 12, vx * scale, vy * scale, vz * scale)
            vert_off += num_vertices * 12
        if has_normals:
            vert_off += num_vertices * 12
        scan_off = vert_off

    target_path = output or path
    target_path.write_bytes(raw)


if __name__ == "__main__":
    import sys

    if len(sys.argv) < 3:
        raise SystemExit("usage: scale_tractor_wheel_dff.py <scale> <input_file> [<output_file>]")

    scale = float(sys.argv[1])
    input_path = Path(sys.argv[2])
    output_path = Path(sys.argv[3]) if len(sys.argv) >= 4 else None
    scale_tractor_wheel(input_path, scale, output=output_path)

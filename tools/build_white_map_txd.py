from __future__ import annotations

import struct
from pathlib import Path


RW_VERSION = 0x1803FFFF
CHUNK_STRUCT = 0x01
CHUNK_EXTENSION = 0x03
CHUNK_TEXTURE_NATIVE = 0x15
CHUNK_TEXDICTIONARY = 0x16

PLATFORM_D3D9 = 0x09
RASTER_B8G8R8 = 0x0600
RASTER_B8G8R8A8 = 0x0500


def chunk(chunk_id: int, payload: bytes, version: int = RW_VERSION) -> bytes:
    return struct.pack("<III", chunk_id, len(payload), version) + payload


def c_string(value: str, size: int) -> bytes:
    data = value.encode("ascii", "ignore")[: size - 1]
    return data + b"\x00" + b"\x00" * (size - len(data) - 1)


def build_mipmap(width: int = 4, height: int = 4, alpha: bool = True) -> bytes:
    pixel = b"\xFF\xFF\xFF\xFF" if alpha else b"\xFF\xFF\xFF"
    return pixel * (width * height)


def build_texture_native(
    name: str,
    width: int = 4,
    height: int = 4,
    alpha: bool = True,
    platform: int = PLATFORM_D3D9,
) -> bytes:
    """
    Build a valid RenderWare texture-native chunk for GTA SA.
    This mirrors the layout used by libtxd in the txdedit project.
    """

    mipmap = build_mipmap(width, height, alpha=alpha)
    filter_flags = 0
    raster_format = RASTER_B8G8R8A8 if alpha else RASTER_B8G8R8
    depth = 32 if alpha else 24
    mipmap_count = 1
    compression_or_alpha = 0
    alpha_flag = 1 if alpha else 0

    struct_payload = b"".join(
        [
            struct.pack("<I", platform),
            struct.pack("<I", filter_flags),
            c_string(name, 32),
            c_string("", 32),
            struct.pack("<I", raster_format),
            struct.pack("<I", alpha_flag),
            struct.pack("<HH", width, height),
            struct.pack("<B", depth),
            struct.pack("<B", mipmap_count),
            struct.pack("<B", 4),  # raster type
            struct.pack("<B", compression_or_alpha),
            struct.pack("<I", len(mipmap)),
            mipmap,
        ]
    )

    texture_struct = chunk(CHUNK_STRUCT, struct_payload)
    return chunk(CHUNK_TEXTURE_NATIVE, texture_struct + chunk(CHUNK_EXTENSION, b""))


def build_txd(texture_names: list[str]) -> bytes:
    struct_payload = struct.pack("<HH", len(texture_names), 0)
    textures = b"".join(build_texture_native(name) for name in texture_names)
    return chunk(CHUNK_TEXDICTIONARY, chunk(CHUNK_STRUCT, struct_payload) + textures + chunk(CHUNK_EXTENSION, b""))


def main() -> None:
    import sys

    if len(sys.argv) < 3:
        raise SystemExit("usage: build_white_map_txd.py <out.txd> <texture_name1> [texture_name2 ...]")

    out = Path(sys.argv[1])
    out.parent.mkdir(parents=True, exist_ok=True)
    out.write_bytes(build_txd(sys.argv[2:]))


if __name__ == "__main__":
    main()

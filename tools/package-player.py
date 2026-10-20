#!/usr/bin/env python3
"""Create the minimal player ZIP from an existing Linux build."""

import argparse
import hashlib
import struct
import zipfile
from pathlib import Path


def main():
    root = Path(__file__).resolve().parents[1]
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--input", type=Path, default=root / "output")
    parser.add_argument("--output", type=Path, default=root / "dist" / "EU4-1.37-Linux-玩家版.zip")
    args = parser.parse_args()

    library = (args.input / "lib64Creamlinux.so").read_bytes()
    if len(library) < 64 or library[:6] != b"\x7fELF\x02\x01" or struct.unpack_from("<H", library, 18)[0] != 62:
        parser.error("The input library must be an x86_64 Linux ELF file.")

    license_text = (root / "LICENSE").read_text(encoding="utf-8")
    license_text = license_text.replace(
        "Copyright (c) 2022 20PercentRendered",
        "Copyright (c) 2022 20PercentRendered\n"
        "Copyright (c) 2018 Danijel Durakovic (mINI)\n"
        "Copyright (c) 2015-present Gabi Melman and spdlog contributors\n"
        "Copyright (c) 2012-present Victor Zverovich and fmt contributors",
    )
    license_text += (
        "\nCreamlinux: https://github.com/anticitizn/creamlinux\n"
        "Steamworks interface declarations: Copyright Valve Corporation. All rights reserved.\n"
    )
    files = {
        "cream.sh": (root / "package" / "cream.sh").read_bytes(),
        "cream_api.ini": (root / "package" / "cream_api.ini").read_bytes(),
        "lib64Creamlinux.so": library,
        "使用说明.txt": (root / "package" / "使用说明.txt").read_bytes(),
        "开源许可.txt": license_text.encode("utf-8"),
    }

    args.output.parent.mkdir(parents=True, exist_ok=True)
    with zipfile.ZipFile(args.output, "w", compression=zipfile.ZIP_DEFLATED, compresslevel=9) as archive:
        for name, data in files.items():
            entry = zipfile.ZipInfo(name)
            entry.create_system = 3
            entry.external_attr = (0o100755 if name == "cream.sh" else 0o100644) << 16
            entry.compress_type = zipfile.ZIP_DEFLATED
            archive.writestr(entry, data)

    with zipfile.ZipFile(args.output) as archive:
        assert set(archive.namelist()) == set(files)
        assert archive.testzip() is None
        for name, data in files.items():
            assert archive.read(name) == data, name
    print(args.output)
    print(f"Files: {len(files)}; size: {args.output.stat().st_size} bytes")
    print(f"SHA256: {hashlib.sha256(args.output.read_bytes()).hexdigest()}")


if __name__ == "__main__":
    main()

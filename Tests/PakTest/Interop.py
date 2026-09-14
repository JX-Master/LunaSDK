#!/usr/bin/env python3
"""Check Pak interoperability with Python's independent ZIP implementation."""
from pathlib import Path
import subprocess
import tempfile
import zipfile


def main():
    root = Path(__file__).resolve().parents[2]
    with tempfile.TemporaryDirectory(prefix="luna-pak-interop-") as temporary:
        directory = Path(temporary)
        source = directory / "python.zip"
        output = directory / "luna.pak"
        with zipfile.ZipFile(source, "w", allowZip64=True) as archive:
            archive.writestr("python/stored.txt", b"stored by Python", compress_type=zipfile.ZIP_STORED)
            archive.writestr("python/文件.txt", b"compressed by Python", compress_type=zipfile.ZIP_DEFLATED)
            archive.writestr("empty/", b"")
            with archive.open("small-zip64", "w", force_zip64=True) as entry:
                entry.write(b"ZIP64 local entry")
        original = source.read_bytes()
        subprocess.run(
            ["dotnet", "run", "--project", "LunaBuild.csproj", "--", "run", "--target", "PakTest",
             "--", str(output), str(source)],
            cwd=root, check=True,
        )
        assert source.read_bytes() == original
        with zipfile.ZipFile(output) as archive:
            assert archive.testzip() is None
            assert archive.read("other") == b"other PAK!"
            assert archive.read("copy") == b"changed"
            assert archive.read("assets/文件.bin") == b"abXYe"
            assert archive.getinfo("other").compress_type == zipfile.ZIP_DEFLATED
            assert archive.getinfo("copy").compress_type == zipfile.ZIP_STORED
            assert archive.getinfo("empty-again/").is_dir()
            assert not any(name.startswith("__luna_pak_move_") for name in archive.namelist())
        with zipfile.ZipFile(str(source) + ".edited.pak") as archive:
            assert archive.testzip() is None
            assert archive.read("imported/stored.txt") == b"stored by Python"
            assert archive.read("imported/文件.txt") == b"compressed by Python"
            assert archive.read("imported/added.txt") == b"written by Pak"
            assert archive.read("small-zip64") == b"ZIP64 local entry"
            assert "python/stored.txt" not in archive.namelist()
        print("Python/Pak ZIP interoperability passed.")


if __name__ == "__main__":
    main()

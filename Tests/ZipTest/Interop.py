"""Check ZIP interoperability in both directions using Python's standard library.

Run from any directory with `python3 Tests/ZipTest/Interop.py`.
The script launches ZipTest through LunaBuild; do not run another build alongside it.
"""
from pathlib import Path
import subprocess
import tempfile
import zipfile


def main():
    root = Path(__file__).resolve().parents[2]
    with tempfile.TemporaryDirectory(prefix="luna-zip-interop-") as directory:
        source = Path(directory) / "python.zip"
        output = Path(directory) / "luna.zip"
        contents = b"Created by Python zipfile."
        with zipfile.ZipFile(source, "w", allowZip64=True) as archive:
            archive.writestr("stored.txt", contents, compress_type=zipfile.ZIP_STORED)
            archive.writestr("deflated.txt", contents, compress_type=zipfile.ZIP_DEFLATED)
            archive.writestr("unsupported.txt", contents, compress_type=zipfile.ZIP_BZIP2)
            with archive.open("zip64.txt", "w", force_zip64=True) as entry:
                entry.write(contents)
        original = source.read_bytes()
        subprocess.run([
            "dotnet", "run", "--project", "LunaBuild.csproj", "--", "run",
            "--target", "ZipTest", "--", str(output), str(source),
        ], cwd=root, check=True)
        assert source.read_bytes() == original
        with zipfile.ZipFile(output) as archive:
            assert archive.testzip() is None
            expected = b"Luna ZIP stream test. Luna ZIP stream test."
            assert archive.read("stored.txt") == expected
            assert archive.read("nested/\u6587\u4ef6.txt") == expected
            assert archive.getinfo("stored.txt").compress_type == zipfile.ZIP_STORED
            assert archive.getinfo("nested/\u6587\u4ef6.txt").compress_type == zipfile.ZIP_DEFLATED
            assert archive.getinfo("empty/").is_dir()
            assert archive.read("zero") == b""
        print("Zip interoperability: Python to Luna and Luna to Python passed.")


if __name__ == "__main__":
    main()

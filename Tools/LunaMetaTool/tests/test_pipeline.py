"""Exercise the native Parse/Merge protocol using an already-built LunaMetaTool."""
import argparse
import concurrent.futures
import json
from pathlib import Path
import subprocess
import tempfile
import unittest

parser = argparse.ArgumentParser()
parser.add_argument("--tool", type=Path, required=True)
arguments = parser.parse_args()
TOOL = arguments.tool.resolve()


class MetadataPipelineTests(unittest.TestCase):
    def setUp(self):
        self.scope = tempfile.TemporaryDirectory(prefix="luna-meta-pipeline-")
        self.root = Path(self.scope.name)
        self.generated = self.root / "generated"
        self.generated.mkdir()
        self.headers = [self.root / "First.hpp", self.root / "Second.hpp"]
        self.headers[0].write_text('#pragma once\n#include "Second.hpp"\n#include "First.generated.hpp"\n')
        self.headers[1].write_text('#pragma once\n#include "Second.generated.hpp"\n')
        self.intermediates = [self.root / "First.json", self.root / "Second.json"]

    def tearDown(self):
        self.scope.cleanup()

    def invoke(self, stage, extra):
        return subprocess.run(
            [str(TOOL), "--stage", stage, "--target", "Fixture", "--output-dir", str(self.generated),
             "--include", str(self.root), "--include", str(self.generated), *extra],
            capture_output=True, text=True, timeout=60)

    def parse_header(self, index):
        extra = ["--header", str(self.headers[index]), "--intermediate", str(self.intermediates[index]),
                 "--depfile", str(self.intermediates[index].with_suffix(".d"))]
        for header in self.headers:
            extra += ["--placeholder-header", str(self.generated / (header.stem + ".generated.hpp"))]
        result = self.invoke("parse", extra)
        self.assertEqual(0, result.returncode, result.stderr)

    def parse_all(self):
        with concurrent.futures.ThreadPoolExecutor(max_workers=2) as executor:
            list(executor.map(self.parse_header, range(2)))

    def merge(self, inputs=None):
        extra = ["--stamp", str(self.generated / "Fixture.stamp"), "--depfile", str(self.generated / "Fixture.d")]
        for header, intermediate in zip(self.headers, inputs or self.intermediates):
            extra += ["--header", str(header), "--input", str(intermediate)]
        return self.invoke("merge", extra)

    def output_snapshot(self):
        return {path.name: (path.read_bytes(), path.stat().st_mtime_ns)
                for path in self.generated.iterdir() if path.is_file()}

    def test_virtual_headers_and_deterministic_merge(self):
        self.parse_all()
        self.assertEqual([], list(self.generated.iterdir()), "Parse must not publish shared generated headers")
        for index, intermediate in enumerate(self.intermediates):
            metadata = json.loads(intermediate.read_text())
            self.assertEqual(1, metadata["version"])
            self.assertEqual([], metadata["declarations"])
            depfile = intermediate.with_suffix(".d").read_text().lower()
            self.assertIn(self.headers[index].name.lower(), depfile)
            self.assertNotIn(".generated.hpp", depfile, "virtual placeholders must not create incremental cycles")
        self.assertIn("second.hpp", self.intermediates[0].with_suffix(".d").read_text().lower())
        result = self.merge()
        self.assertEqual(0, result.returncode, result.stderr)
        first = {path.name: path.read_bytes() for path in self.generated.iterdir()}
        result = self.merge()
        self.assertEqual(0, result.returncode, result.stderr)
        self.assertEqual(first, {path.name: path.read_bytes() for path in self.generated.iterdir()})

    def test_invalid_intermediates_leave_outputs_untouched(self):
        self.parse_all()
        result = self.merge()
        self.assertEqual(0, result.returncode, result.stderr)
        before = self.output_snapshot()
        valid = json.loads(self.intermediates[0].read_text())
        invalid_cases = ["{", json.dumps({}), json.dumps({**valid, "version": 99}),
                         json.dumps({**valid, "target": "WrongTarget"}),
                         json.dumps({**valid, "header": str(self.root / "Wrong.hpp")}),
                         json.dumps({**valid, "language": "objective-c++20"})]
        bad = self.root / "invalid.json"
        for data in invalid_cases:
            with self.subTest(data=data):
                bad.write_text(data)
                result = self.merge([bad, self.intermediates[1]])
                self.assertNotEqual(0, result.returncode)
                self.assertIn("error:", result.stderr)
                self.assertEqual(before, self.output_snapshot())

    def test_parse_failure_does_not_publish_an_intermediate(self):
        bad_header = self.root / "Bad.hpp"
        bad_header.write_text('#include "Bad.generated.hpp"\n#include <stddef.h>\n')
        intermediate = self.root / "Bad.json"
        extra = ["--header", str(bad_header), "--intermediate", str(intermediate),
                 "--depfile", str(self.root / "Bad.d"), "--placeholder-header", str(self.generated / "Bad.generated.hpp")]
        result = self.invoke("parse", extra)
        self.assertNotEqual(0, result.returncode)
        self.assertIn("error:", result.stderr)
        self.assertFalse(intermediate.exists())
        self.assertFalse((self.root / "Bad.d").exists())
        self.assertEqual([], list(self.generated.iterdir()))


if __name__ == "__main__":
    unittest.main(argv=[__file__], verbosity=2)

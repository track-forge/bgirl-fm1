# SPDX-License-Identifier: GPL-3.0-only
import importlib.util
from pathlib import Path
import tempfile
import unittest

ROOT = Path(__file__).resolve().parents[1]
spec = importlib.util.spec_from_file_location('logo', ROOT/'tools/package_logo.py')
logo = importlib.util.module_from_spec(spec)
spec.loader.exec_module(logo)

class LogoTest(unittest.TestCase):
    def test_deterministic_bounded_pack(self):
        with tempfile.TemporaryDirectory() as tmp:
            a, b = Path(tmp)/'a.h', Path(tmp)/'b.h'
            packed, colors = logo.package(a)
            logo.package(b)
            self.assertEqual(a.read_bytes(), b.read_bytes())
            self.assertEqual(len(packed), 224*145//2)
            self.assertEqual(len(colors), 16)
            self.assertLessEqual(max(colors), 65535)
            self.assertIn(logo.SOURCE_SHA, a.read_text())

    def test_unreviewed_source_change_rejected(self):
        with tempfile.TemporaryDirectory() as tmp:
            source = Path(tmp)/'changed.png'
            source.write_bytes(b'not the reviewed wordmark')
            old = logo.SOURCE
            try:
                logo.SOURCE = source
                with self.assertRaisesRegex(ValueError, 'wordmark changed'):
                    logo.package(Path(tmp)/'out.h')
            finally:
                logo.SOURCE = old

if __name__ == '__main__':
    unittest.main()

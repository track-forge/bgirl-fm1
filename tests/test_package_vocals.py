#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-only
"""Offline fixture and malformed-input acceptance; no private files needed."""
import copy
import importlib.util
import json
from pathlib import Path
import struct
import tempfile
import unittest
import wave

ROOT = Path(__file__).resolve().parents[1]
spec = importlib.util.spec_from_file_location('package_vocals', ROOT/'tools/package_vocals.py')
pv = importlib.util.module_from_spec(spec)
spec.loader.exec_module(pv)


class Packaging(unittest.TestCase):
    def setUp(self):
        self.temp = tempfile.TemporaryDirectory()
        self.addCleanup(self.temp.cleanup)
        self.base = Path(self.temp.name)
        self.input = self.base/'inputs'
        self.slices = self.input/'slices'
        self.slices.mkdir(parents=True)
        meta = json.loads((ROOT/'assets/speech/manifest.json').read_text())
        source = ROOT/'assets/speech'/meta['phrases'][0]['dry_source']
        with wave.open(str(source)) as w:
            params = w.getparams()
            self.raw = w.readframes(params.nframes)
        self.params = params
        self.name = 'test_candidate_float.wav'
        self.write(self.input/self.name, self.raw)
        n = params.nframes
        self.manifest = dict(source_count=1, slice_count=2, slices=[])
        for j, (a, b) in enumerate(((0, n//2), (n//2, n))):
            name = f'{j+1}.wav'
            self.write(self.slices/name, self.raw[a*2:b*2])
            self.manifest['slices'].append(dict(source=self.name, slice=name, slice_index=j+1,
                start_seconds=a/params.framerate, end_seconds=b/params.framerate,
                duration_seconds=(b-a)/params.framerate, sample_rate=params.framerate,
                channels=1, edge_fade_ms=3.0))
        self.path = self.slices/'manifest.json'

    def write(self, path, raw):
        with wave.open(str(path), 'wb') as w:
            w.setparams(self.params)
            w.writeframes(raw)

    def package(self, manifest=None, dest='out'):
        self.path.write_text(json.dumps(manifest or self.manifest))
        return pv.package(self.input, self.path, self.base/dest)

    def test_reproducible_and_sorted(self):
        a = self.package(dest='a')
        self.manifest['slices'].reverse()
        b = self.package(dest='b')
        self.assertEqual((self.base/'a/bank.ima').read_bytes(), (self.base/'b/bank.ima').read_bytes())
        self.assertEqual(a['entries'], b['entries'])
        # Manifest bytes are provenance: row reordering changes identity, not mapping/audio.
        for f in ('bank.ima', 'bank.json', 'bgirl_vocals.h'):
            self.package(dest='c')
            self.assertEqual((self.base/'b'/f).read_bytes(), (self.base/'c'/f).read_bytes())
        self.assertEqual(len(a['entries']), 3)
        self.assertEqual(a['groups'][1]['first'], 1)
        self.assertEqual(a['bytes'], (a['samples']+1)//2)
        self.assertEqual(a['entries'][0]['source_sha256'], pv.sha((self.input/self.name).read_bytes()))

    def test_versioned_pack_and_shared_slice(self):
        path = self.input/'pack.json'
        shot = dict(id='shot', name='Shot', wav=self.name, frames=[0, 512])
        loop = dict(id='loop', name='Loop', wav=self.name, frames=[512, 1024], bpm=128)
        cut = dict(id='cut', name='Cut', parent='loop', frames=[640, 896])
        manifest = dict(version=1, id='test-pack', name='Test Pack', max_audio_bytes=2048,
                        groups=[dict(id=kind, kind=kind, name=kind.upper(), entries=[entry])
                                for kind, entry in [('shots', shot), ('loops', loop), ('slices', cut)]])
        path.write_text(json.dumps(manifest))
        a = pv.package_pack(path, self.base/'a')
        b = pv.package_pack(path, self.base/'b')
        self.assertEqual(a, b)
        self.assertEqual((self.base/'a/bank.ima').read_bytes(), (self.base/'b/bank.ima').read_bytes())
        parent, child = a['entries'][1:]
        self.assertEqual(a['samples'], parent['end'])
        self.assertLess(parent['start'], child['start'])
        self.assertLess(child['end'], parent['end'])
        for record, key, invalid in [(manifest, 'version', 2), (manifest, 'max_audio_bytes', 1),
                                     (shot, 'wav', '../missing.wav'), (shot, 'id', 'loop'),
                                     (cut, 'frames', [128, 384]), (loop, 'bpm', -1)]:
            with self.subTest(key=key):
                old = record[key]
                record[key] = invalid
                path.write_text(json.dumps(manifest))
                with self.assertRaises(ValueError):
                    pv.package_pack(path, self.base/'bad')
                record[key] = old

    def test_fixture_reproduces(self):
        a = pv.fixture(self.base/'fixture1')
        b = pv.fixture(self.base/'fixture2')
        self.assertEqual(a, b)
        for f in ('bank.ima', 'bank.json', 'bgirl_vocals.h'):
            self.assertEqual((self.base/'fixture1'/f).read_bytes(), (self.base/'fixture2'/f).read_bytes())

    def test_single_source_keeps_g6_label(self):
        self.path.write_text(json.dumps(self.manifest))
        a = pv.package(self.input, self.path, self.base/'g6', group_ids=[6])
        b = self.package(dest='g1')
        self.assertEqual(a['groups'][1]['name'], 'G6')
        self.assertEqual(a['entries'], b['entries'])
        self.assertEqual((self.base/'g6/bank.ima').read_bytes(), (self.base/'g1/bank.ima').read_bytes())
        self.assertNotEqual(a['bank_sha256'], b['bank_sha256'])
        for ids in ([], [0], [7], [1, 2], ['6']):
            with self.subTest(ids=ids), self.assertRaises(ValueError):
                pv.package(self.input, self.path, self.base/'bad', group_ids=ids)

    def test_missing_inputs_never_substitute(self):
        (self.input/self.name).unlink()
        with self.assertRaisesRegex(ValueError, 'inventory'): self.package()

    def test_missing_extra_duplicate_chops(self):
        self.write(self.slices/'extra.wav', self.raw)
        with self.assertRaisesRegex(ValueError, 'inventory'): self.package()
        (self.slices/'extra.wav').unlink()
        m = copy.deepcopy(self.manifest)
        m['slices'][1]['slice'] = m['slices'][0]['slice']
        with self.assertRaisesRegex(ValueError, 'inventory'): self.package(m)
        (self.slices/'2.wav').unlink()
        with self.assertRaisesRegex(ValueError, 'inventory'): self.package()

    def test_malformed_manifests(self):
        for field, value in [('start_seconds', -1), ('end_seconds', 999), ('duration_seconds', 99),
                             ('sample_rate', 123), ('channels', 2), ('edge_fade_ms', 0), ('slice_index', 3)]:
            with self.subTest(field=field):
                m = copy.deepcopy(self.manifest)
                m['slices'][0][field] = value
                with self.assertRaises(ValueError): self.package(m)
        for field in ('source_count', 'slice_count'):
            m = copy.deepcopy(self.manifest)
            m[field] = 99
            with self.assertRaises(ValueError): self.package(m)

    def test_missing_tail_and_wrong_extent(self):
        m = copy.deepcopy(self.manifest)
        m['slices'][1]['end_seconds'] -= .01
        with self.assertRaises(ValueError): self.package(m)
        self.write(self.slices/'2.wav', self.raw[:100])
        with self.assertRaisesRegex(ValueError, 'extent'): self.package()

    def test_unsafe_paths(self):
        with self.assertRaises(ValueError): pv.safe_file(self.input, '../escape.wav')
        with self.assertRaises(ValueError): pv.safe_file(self.input, str(ROOT/'README.md'))

    def test_float_stereo_downmix_and_invalid_pcm(self):
        def floating(values, rate=44100):
            raw = struct.pack('<%df'%len(values), *values)
            fmt = struct.pack('<HHIIHH', 3, 2, rate, rate*8, 8, 32)
            return b'RIFF'+struct.pack('<I', 36+len(raw))+b'WAVEfmt '+struct.pack('<I',16)+fmt+b'data'+struct.pack('<I',len(raw))+raw
        file = self.base/'float.wav'
        file.write_bytes(floating([.5, -.25, 1.008, -1.0]))
        _, sr, ch, mono = pv.read_wav(file)
        self.assertEqual((sr,ch), (44100,2))
        self.assertAlmostEqual(mono[0], .125)
        for values in ([float('nan'),0], [float('inf'),0], [2,0]):
            file.write_bytes(floating(values))
            with self.assertRaises(ValueError): pv.read_wav(file)
        file.write_bytes(floating([0,0], 12345))
        with self.assertRaises(ValueError): pv.read_wav(file)
        file.write_bytes(b'not audio')
        with self.assertRaises(ValueError): pv.read_wav(file)

if __name__ == '__main__': unittest.main()

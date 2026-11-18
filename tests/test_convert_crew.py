# SPDX-License-Identifier: GPL-3.0-only
import hashlib,json,re,sys,unittest
from pathlib import Path
root=Path(__file__).resolve().parents[1]
sys.path.insert(0,str(root/'tools'))
from convert_dance import convert
class CrewPack(unittest.TestCase):
    def test_four_integral_banks(self):
        sources=json.loads((root/'assets/crew/sources.json').read_text())
        self.assertEqual(len(sources),4)
        for source in sources:
            name=source['clip'];base=root/'assets/crew'/name
            m=json.loads((base/'manifest.json').read_text())
            blob=(base/'poses-96.bin').read_bytes()
            self.assertEqual(len(blob),13824)
            self.assertEqual(hashlib.sha256(blob).hexdigest(),m['sizes']['96']['sha256'])
            self.assertEqual(m['source_sha256'],source['source_sha256'])
            self.assertEqual(sorted(set(m['indices'])),m['indices'])
            self.assertEqual(len(m['indices']),12)
            self.assertEqual(m['playback_indices'],list(range(12))+list(range(10,0,-1)))
            self.assertEqual(m['playback'],'ping-pong')
            self.assertEqual(m['phrase_beats'],4)
            self.assertLess(max(m['indices']),m['frame_count'])
            self.assertGreaterEqual(len(set(blob[k*1152:(k+1)*1152] for k in range(12))),10)
            header=(base/('bgirl_'+name.replace('-','_')+'.h')).read_text()
            self.assertEqual(bytes(int(x,16) for x in re.findall(r'0x([0-9a-f]{2})',header)),blob)
            self.assertIn('GPL-3.0-only',m['rights'])
            # All masks retain padding; no limb touches the crop edge.
            for k in range(12):
                rows=blob[k*1152:(k+1)*1152]
                self.assertFalse(any(rows[:12]+rows[-12:]))
                self.assertFalse(any(rows[y*12]&128 or rows[y*12+11]&1 for y in range(96)))
    def test_art_direction_recipe(self):
        base=root/'assets/crew'
        w=json.loads((base/'windmill/manifest.json').read_text())
        self.assertEqual(w['framing'],'follow-x')
        self.assertGreater(470/w['stage'][2],1.25) # prior stage470: visibly larger at constant scale
        _,top,side=w['stage']
        for left,(x0,y0,x1,y1) in zip(w['crop_lefts'],w['pose_bounds']):
            self.assertGreater(x0,left)
            self.assertLess(x1,left+side)
            self.assertGreater(y0,top)
            self.assertLess(y1,top+side)
        worm=json.loads((base/'worm/manifest.json').read_text())
        self.assertEqual(worm['source_window_seconds'],[1,4])
        self.assertEqual(worm['indices'],[24,31,37,44,50,57,63,70,76,83,89,96])
        self.assertTrue(all(1<=i/24<=4 for i in worm['indices']))
        # Neighboring source masks do not change merely to add reverse playback.
        for name,digest in [('reference-02','3a4a274f0245e4ba34b9f1447a97a95e2f4838a319465a3d4e14f378d1664064'),('wave-ish','fbf7d9d5f6308e3aa28f413d9e7e20973499dfaa7dcfafcc0f5e3323456d4b93')]:
            self.assertEqual(hashlib.sha256((base/name/'poses-96.bin').read_bytes()).hexdigest(),digest)
    def test_invalid_recipe(self):
        for kw in ({'clip':'../bad'},{'sizes':(64,)},{'framing':'crop-limbs'},{'bounds_scope':'bad'},{'padding':3}):
            with self.assertRaises(ValueError):convert(Path('/not-read'),Path('/not-written'),[], 'x',**kw)
if __name__=='__main__':unittest.main()

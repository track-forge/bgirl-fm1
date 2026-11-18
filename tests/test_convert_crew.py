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
    def test_invalid_recipe(self):
        for kw in ({'clip':'../bad'},{'sizes':(64,)}):
            with self.assertRaises(ValueError):convert(Path('/not-read'),Path('/not-written'),[], 'x',**kw)
if __name__=='__main__':unittest.main()

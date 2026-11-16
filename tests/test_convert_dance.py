# SPDX-License-Identifier: GPL-3.0-only
"""Pack layout integrity; full MP4 determinism uses the documented local source."""
import hashlib
import json
import re
import unittest
from pathlib import Path
root=Path(__file__).resolve().parents[1]
class Pack(unittest.TestCase):
    def test_masks(self):
        m=json.loads((root/'assets/dance/manifest.json').read_text())
        self.assertEqual(len(set(m['indices'])),12)
        for size in (64,96):
            blob=(root/f'assets/dance/poses-{size}.bin').read_bytes()
            stride=size*size//8
            self.assertEqual(len(blob),12*stride)
            self.assertEqual(hashlib.sha256(blob).hexdigest(),m['sizes'][str(size)]['sha256'])
            self.assertEqual(len({blob[i*stride:(i+1)*stride] for i in range(12)}),12)
        values=bytes(int(x,16) for x in re.findall(r'0x([0-9a-f]{2})',(root/'assets/dance/bgirl_dance.h').read_text()))
        self.assertEqual(values,(root/'assets/dance/poses-96.bin').read_bytes())
    def test_packing(self):
        import sys
        sys.path.insert(0,str(root/'tools'))
        from convert_dance import pack
        from PIL import Image
        im=Image.new('L',(9,2));im.putpixel((0,0),255);im.putpixel((8,1),255)
        self.assertEqual(pack(im),bytes([128,0,0,128]))
if __name__=='__main__':unittest.main()

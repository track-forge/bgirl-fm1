#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-only
"""Stable inherited icon IDs/pixels and original SVG -> compressed-cell parity."""
import hashlib
import json
from pathlib import Path
import re
import sys
sys.path.insert(0, str(Path(__file__).resolve().parents[1] / 'felucca/tools'))
import bgirl_sections
from gen_aa_font import huff_unpack


def parse(path):
    text = Path(path).read_text()
    def arr(name):
        return [int(v, 0) for v in re.search(r'\b'+name+r'\[\d+\] = \{([^}]*)\}', text).group(1).replace('\n',' ').split(',') if v.strip()]
    names=re.findall(r'^    ICON_(\w+),',text,re.M)
    result={}
    for size in [12,16,24]:
        data=bytes(arr(f'AI{size}_DATA'));offs=arr(f'AI{size}_OFF');idx=arr(f'AI{size}_IDX')
        result[size]={n:bytes(huff_unpack(arr('AI_HC'),data[offs[idx[k]]:],size*size)) for k,n in enumerate(names) if idx[k]!=255}
    return names,result


if __name__=='__main__':
    manifest=Path('felucca/assets/bgirl-icons/inherited-alpha-sha256.json')
    if len(sys.argv)>1 and sys.argv[1]=='--baseline':
        names,cells=parse(sys.argv[2])
        manifest.write_text(json.dumps({'names':names,'sizes':{s:{n:hashlib.sha256(v).hexdigest() for n,v in cs.items()} for s,cs in cells.items()}},indent=2)+'\n')
    else:
        old=json.loads(manifest.read_text());names,cells=parse('felucca/build/gen/ui_icons.h')
        assert names[:len(old['names'])]==old['names']
        inherited=0
        for size,cs in cells.items():
            for n,h in old['sizes'][str(size)].items():
                assert hashlib.sha256(cs[n]).hexdigest()==h,(size,n)
                inherited+=1
            masks=[]
            for name in bgirl_sections.NAMES:
                b,_=bgirl_sections.raster(name,size)
                alpha=bytes(v for i in range(size*size) for v in [(b[i//2]>>(0 if i&1 else 4))&15])
                assert cs[name.upper()]==alpha
                assert any(alpha) and len(set(alpha))>2
                assert not any(alpha[:size]) and not any(alpha[-size:]),(name,size,'padding')
                masks.append(alpha)
            assert len(set(masks))==3
        print(f'STUDIO icons: inherited IDs +{inherited} exact alpha cells unchanged; 9 original size-specific SVG cells/decoder parity, padding, distinct silhouettes PASS')

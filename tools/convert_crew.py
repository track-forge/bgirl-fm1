#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-only
"""Offline conversion of the four supplied, authorized clips. No network access."""
import argparse
import json
from pathlib import Path
from convert_dance import convert

def main():
    p=argparse.ArgumentParser(description=__doc__)
    p.add_argument('originals',type=Path)
    p.add_argument('dest',type=Path)
    a=p.parse_args()
    sources=json.loads((Path(__file__).resolve().parents[1]/'assets/crew/sources.json').read_text())
    for item in sources:
        name=item['clip']
        m=convert(a.originals/(name+'.mp4'),a.dest/name,item['indices'],item['source_sha256'],item['source_url'],item['rights'],clip=name,sizes=(96,))
        m.update({k:item[k] for k in ('creator','caption','provenance')})
        (a.dest/name/'manifest.json').write_text(json.dumps(m,indent=2)+'\n')
        print(name,m['source_sha256'],m['sizes']['96']['sha256'])
if __name__=='__main__':main()

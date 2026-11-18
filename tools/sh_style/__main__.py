# SPDX-License-Identifier: GPL-3.0-only
import argparse
import json
from pathlib import Path
from .core import artifact_bytes, translate_bundle
from .bank import generate

def main():
    p=argparse.ArgumentParser(description='Offline deterministic approximate SH-STYLE recipes (no MIDI)')
    sub=p.add_subparsers(dest='command',required=True)
    t=sub.add_parser('translate');t.add_argument('bundle',type=Path);t.add_argument('--name',required=True);t.add_argument('--preset',type=int,required=True)
    t.add_argument('--output',type=Path,required=True);t.add_argument('--overrides',type=Path);t.add_argument('--strict',action='store_true')
    b=sub.add_parser('build-bank');b.add_argument('--check',action='store_true')
    a=p.parse_args()
    if a.command=='build-bank': generate(a.check)
    else:
        overrides=json.loads(a.overrides.read_bytes()) if a.overrides else None
        r,report=translate_bundle(a.bundle,a.name,a.preset,overrides=overrides,strict=a.strict)
        a.output.mkdir(parents=True,exist_ok=False)
        (a.output/'recipe.json').write_bytes(artifact_bytes(r));(a.output/'report.json').write_bytes(artifact_bytes(report))
if __name__=='__main__': main()

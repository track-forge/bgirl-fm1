# SPDX-License-Identifier: GPL-3.0-only
"""One additive ANALOG wrapper generator, shared with the carrier."""
import json
from .core import ROOT, ENGINE, PARAMS, POLICY, IDS, canonical, sha, validate_values

MANIFEST = ROOT/'assets/sh-style/manifest.json'

def read_bank():
    m=json.loads(MANIFEST.read_bytes())
    if m['version'] != 1 or m['collection'] != 'SH-ISH' or len(m['sounds']) != 2:
        raise ValueError('invalid collection manifest')
    if len({s['name'] for s in m['sounds']}) != len(m['sounds']):
        raise ValueError('duplicate preset names; choose distinct curated names')
    result=[]
    for i,s in enumerate(m['sounds']):
        r=json.loads((MANIFEST.parent/s['recipe']).read_bytes())
        report=json.loads((MANIFEST.parent/s['report']).read_bytes())
        if r['engine'] != 0 or r['preset'] != 13+i or r['name'] != s['name'] or r['version'] != 1:
            raise ValueError('bank IDs/names must append after BG CARR SAW')
        if len(r['name'])>12 or any(c not in 'ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789 -' for c in r['name']):
            raise ValueError('invalid preset name')
        if sha(canonical(r)) != report['recipe_sha256'] or sha(POLICY.read_bytes()) != report['policy_sha256']:
            raise ValueError('recipe/policy hash mismatch')
        if report['translator_sha256'] != sha((ROOT/'tools/sh_style/core.py').read_bytes()):
            raise ValueError('translator drift; reproduce recipes/reports from original private sources')
        if report['engine_source_sha256'] != sha(ENGINE.read_bytes()) or report['common_descriptors_sha256'] != sha(PARAMS.read_bytes()):
            raise ValueError('target descriptor drift; review mappings before regeneration')
        if report['source_snapshot_sha256'] != s['snapshot_sha256'] or report['source_raw_sha256'] != s['raw_sha256']:
            raise ValueError('source reference mismatch')
        validate_values(r['values'],IDS)
        result.append(r)
    return result

def generate(check=False):
    recipes=read_bank()
    source=ENGINE.read_text()
    start=source.index('static const preset_t ANALOG_PRESETS[]')
    array=source[start:source.index('\n};',start)+3].replace('ANALOG_PRESETS','BG_ANALOG_PRESETS')
    rows=['    {"BG CARR SAW", {0,0,0,0,127,0,0,0}, {0,0,127,20}, 0,1,FX(0,0,0,0),PAT(4)},']
    for r in recipes:
        v=r['values']
        rows.append('    {"%s", {%s}, {%s}, %d,1,FX(0,0,0,0),PAT(4)},' %
                    (r['name'],','.join(str(v['P_E'+str(k)]) for k in range(8)),
                     ','.join(str(v[k]) for k in ('P_ATK','P_DEC','P_SUS','P_REL')),v['P_ED_FLT']))
    array=array.replace('\n};','\n'+'\n'.join(rows)+'\n};')
    start=source.index('static const engine_t ENG_ANALOG =')
    engine=source[start:source.index('\n};',start)+3].replace('ENG_ANALOG','BG_ANALOG').replace('ANALOG_PRESETS','BG_ANALOG_PRESETS')
    hook=['/* Normal factory loading applies extra common parameters, inside the existing load transaction. */',
          'static void sh_style_factory_loaded(track_t *t) {', '    if (t->eng_req != 0) return;', '    switch(t->preset) {']
    for r in recipes:
        hook.append('    case %d:' % r['preset'])
        for k,v in sorted(r['values'].items()): hook.append('        t->p[%s] = %d;' % (k,v))
        hook.append('        break;')
    hook+=['    default: break;','    }','}']
    content='/* SPDX-License-Identifier: GPL-3.0-only; generated local wrapper around MIT engine presets. */\n'+array+'\n'+engine+'\n'+'\n'.join(hook)+'\n'
    out=ROOT/'felucca/firmware/src/bgirl_carrier.h'
    if check:
        if out.read_text()!=content: raise ValueError('stale ANALOG wrapper: python3 tools/carrier_bank.py')
    else: out.write_text(content)

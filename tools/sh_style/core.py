# SPDX-License-Identifier: GPL-3.0-only
"""Pure deterministic translation and sanitized evidence; no transport imports."""
import hashlib
import json
import re
from pathlib import Path
from tools.sh01a.core import canonical, replay
from tools.sh01a.definitions import CONTROLS, PATCH_IDS

ROOT = Path(__file__).resolve().parents[2]
POLICY = ROOT/'assets/sh-style/basics-v1.json'
SCHEMA = ROOT/'tools/sh01a/snapshot.schema.json'
ENGINE = ROOT/'felucca/firmware/engines/src/eng_analog.c'
PARAMS = ROOT/'felucca/firmware/src/params.c'
KEPT = {'P_LEVEL','P_PAN','P_MUTE','P_CHRD','P_VOIC','P_SQNT'}
IDS = re.findall(r'\bP_[A-Z0-9_]+\b', (ROOT/'felucca/firmware/src/core.h').read_text().split('enum {')[1].split('P_COUNT')[0])
# Ranges retained by the normal factory sound loader, not part of the sound.
for first,last in [('P_AMODE','P_SGATE'),('P_SLCR','P_SLDEPTH')]:
    KEPT.update(IDS[IDS.index(first):IDS.index(last)+1])

def artifact_bytes(value):
    """Readable, stable public artifacts; semantic recipe hashes use canonical()."""
    return (json.dumps(value,sort_keys=True,indent=2,allow_nan=False)+'\n').encode()

def sha(data):
    return hashlib.sha256(data).hexdigest()

def scaled(raw, maximum=127):
    return max(0, min(maximum, (raw*maximum+63)//127))

def load_policy(path=POLICY):
    p = json.loads(Path(path).read_bytes())
    if p.get('version') != 'basics-v1' or p.get('translator_version') != 1 or p != json.loads(POLICY.read_bytes()):
        raise ValueError('unknown or modified policy; add a versioned implementation first')
    return p

def bounds(key):
    if key == 'P_E0': return 0,6
    if key in ('P_ED_FLT','P_ED_PIT','P_LD_PIT','P_LD_FLT'): return -64,63
    if key == 'P_VOICE': return 0,3
    if key == 'P_LWAVE': return 0,4
    return 0,127

def validate_values(values, allowed):
    if set(values)-set(allowed): raise ValueError('unknown output parameter')
    for k,v in values.items():
        lo,hi = bounds(k)
        if type(v) is not int or not lo <= v <= hi: raise ValueError('out-of-range output '+k)

def translate(snapshot_bytes, raw_bytes, name, preset, overrides=None, strict=False, policy_path=POLICY):
    import jsonschema
    p = load_policy(policy_path)
    s = json.loads(snapshot_bytes)
    jsonschema.Draft202012Validator(json.loads(SCHEMA.read_bytes())).validate(s)
    if type(s['schema_version']) is not int or s['schema_version'] != 1 or s['definition_version'] != p['definition_version']:
        raise ValueError('unsupported schema/definition version')
    if s['source']['evidence'] != 'hardware': raise ValueError('source must be real hardware, not synthetic')
    if sha(raw_bytes) != s['raw_log']['sha256'] or len(raw_bytes) != s['raw_log']['bytes']:
        raise ValueError('raw log hash/length mismatch')
    if s['modified_since_load'] or s['confidence']['disconnect'] or s['confidence']['state'] not in ('partial','ready'):
        raise ValueError('require connected saved-patch baseline, not edits/stale capture')
    if not re.fullmatch(r'[A-Za-z0-9 -]{1,12}',name): raise ValueError('name must be 1..12 ASCII letters, digits, spaces or hyphens')
    if type(preset) is not int or not 13 <= preset <= 127: raise ValueError('preset must be appended 13..127')
    for k,v in s['parameters'].items():
        d=CONTROLS.get(v['cc'])
        if d is None or d['parameter'] != k or d['category'] != v['category']:
            raise ValueError('unknown/inconsistent parameter '+k)
    missing = sorted(set(PATCH_IDS)-set(s['parameters']))
    if missing and strict: raise ValueError('missing fields: '+', '.join(missing))
    raw = {k:v['raw'] for k,v in s['parameters'].items()}
    def get(k): return raw.get(k,0)
    values = dict(p['constants'])
    provenance = {k:dict(sources=[],transform='policy constant; '+p['laws']['selectors']) for k in values}
    inputs = {k:dict(disposition='performance-only/ignored' if k.startswith(('performance.','system.')) else 'unsupported',
                    reason='No verified selector bins/routing or target equivalent; see policy limitations') for k in raw}
    for k in missing: inputs[k]=dict(disposition='missing/defaulted',reason='raw0 assumption; strict mode rejects')
    for out,(source,maximum) in p['mapped'].items():
        values[out]=scaled(get(source),maximum)
        provenance[out]=dict(sources=[source],transform='normalized half-up 0..'+str(maximum)+'; clamp; heuristic not physical equivalence')
        if source in raw: inputs[source]=dict(disposition='approximated',reason=provenance[out]['transform'])
    saw,pulse,sub = (get('mixer.'+k+'_level') for k in ('saw','pulse','sub'))
    losses = list(p['limitations'])
    if saw >= pulse and saw > 0:
        wave=6 if sub else 0
        mix=(127*sub+(saw+sub)//2)//(saw+sub) if sub else 0
        if pulse: losses.append('Pulse component dropped: dominant saw with target SUB/SAW, not independent saw+pulse+sub.')
    elif pulse:
        wave,mix=4,0
        if sub: losses.append('Sub component dropped: dominant pulse uses target PWM pair; cannot combine pulse and sub in this single-track mode.')
    elif sub:
        wave,mix=6,127
    else:
        wave,mix=0,0
        losses.append('No pitched source mixer gain: target SAW remains present; noise-only/silent source cannot be isolated by this policy.')
    values.update(P_E0=wave,P_E2=mix)
    for k in ('P_E0','P_E2'):
        provenance[k]=dict(sources=['mixer.saw_level','mixer.pulse_level','mixer.sub_level'],transform=p['laws']['wave']+' '+p['laws']['sub_mix'])
    for k in ('mixer.saw_level','mixer.pulse_level','mixer.sub_level'):
        if k in raw: inputs[k]=dict(disposition='approximated',reason='dominant single-track waveform; dropped components in losses')
    overrides = overrides or {}
    if 'P_E0' in overrides:
        # The default oscillator-selection losses no longer describe the curated wave.
        losses = losses[:len(p['limitations'])]
    for k,entry in overrides.items():
        if not isinstance(entry,dict) or set(entry) != {'value','reason'} or not isinstance(entry['reason'],str) or not entry['reason'].strip():
            raise ValueError('override requires value and nonempty reason')
        validate_values({k:entry['value']},set(values) | {'P_ED_PIT'})
        values[k]=entry['value'];provenance[k]=dict(sources=provenance.get(k, {}).get('sources', []),transform='explicit override: '+entry['reason'])
    validate_values(values,values)
    recipe = dict(version=1,name=name,engine=0,preset=preset,values=values)
    outputs = {k:dict(value=values[k],disposition='approximated',**provenance[k]) if k in values else
               dict(disposition='retained-track-state' if k in KEPT else 'target-descriptor-default',
                    sources=[],transform='normal apply_preset_to / TP default; no source inference') for k in IDS}
    report = dict(version=1,translator_version=1,source_snapshot_sha256=sha(snapshot_bytes),source_raw_sha256=sha(raw_bytes),
                  source_schema_version=1,source_definition_version=s['definition_version'],
                  capture_confidence=dict(state=s['confidence']['state'],observed=s['confidence']['observed'],expected=s['confidence']['expected']),
                  translation_fidelity='heuristic; uncalibrated; not acoustic proof',policy_sha256=sha(Path(policy_path).read_bytes()),
                  translator_sha256=sha(Path(__file__).read_bytes()),engine_source_sha256=sha(ENGINE.read_bytes()),
                  common_descriptors_sha256=sha(PARAMS.read_bytes()),recipe_sha256=sha(canonical(recipe)),overrides=overrides,
                  inputs=inputs,outputs=outputs,missing=missing,unknown_message_count=s['unknown_count'],losses=losses,
                  metadata_disposition={k:('hash-reference-only' if k=='raw_log' else 'sanitized-confidence-counts' if k=='confidence' else
                      'field-dispositions-only' if k=='parameters' else 'validated-private-not-published') for k in sorted(s)})
    return recipe,report

def translate_bundle(bundle, name, preset, **kw):
    bundle=Path(bundle)
    snapshot=(bundle/'patch.json').read_bytes();raw=(bundle/'events.jsonl').read_bytes()
    s=json.loads(snapshot)
    # Replay authoritative bytes with original private identity; do not publish it.
    t=replay(bundle/'events.jsonl',s['source']['channel'],s['session_id'],s['source']['port'],'hardware').export()
    if any(t[k] != s[k] for k in ('parameters','baseline','changes')):
        raise ValueError('source snapshot does not match raw replay')
    return translate(snapshot,raw,name,preset,**kw)

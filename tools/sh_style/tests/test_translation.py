# SPDX-License-Identifier: GPL-3.0-only
"""Synthetic boundary tests plus optional exact private-bundle reproduction.
Synthetic test state is never used as collection source material.
"""
import copy
import json
import os
import tempfile
import unittest
from pathlib import Path
from tools.sh01a.core import Tracker, canonical
from tools.sh01a.definitions import CONTROLS, PATCH_IDS
from tools.sh_style.core import ROOT, IDS, POLICY, artifact_bytes, sha, scaled, translate, translate_bundle
from tools.sh_style.bank import read_bank, generate


def fixture():
    t=Tracker(3,'SYNTHETIC-UNIT-TEST','PRIVATE-TEST-PORT','hardware')
    t.apply(dict(seq=1,time='2026-01-01T00:00:00+00:00',kind='arm',label='SYNTHETIC TEST ONLY'))
    for cc,d in CONTROLS.items():
        if d['parameter'] in PATCH_IDS:
            t.apply(dict(seq=t.seq+1,time='2026-01-01T00:00:00+00:00',kind='midi',bytes=[178,cc,64]))
    t.apply(dict(seq=t.seq+1,time='2026-01-01T00:00:00+00:00',kind='baseline'))
    s=t.export();s['updated_at']='2026-01-01';s['raw_log']=dict(file='events.jsonl',sha256=sha(b'test'),bytes=4)
    return s

class Translation(unittest.TestCase):
    def run_translation(self,s=None,**kw): return translate(canonical(s or fixture()),b'test','SH TEST',13,**kw)
    def test_deterministic_and_private(self):
        a=self.run_translation();b=self.run_translation();self.assertEqual(canonical(a),canonical(b))
        public=canonical(a).decode()
        for secret in ['PRIVATE-TEST-PORT','SYNTHETIC-UNIT-TEST','2026-01-01','SYNTHETIC TEST ONLY']:
            self.assertNotIn(secret,public)
        self.assertEqual(set(a[1]['outputs']),set(IDS));self.assertEqual(len(a[1]['inputs']),36)
    def test_policy_version_and_schema(self):
        for field,value in [('schema_version',2),('schema_version',True),('definition_version','unknown')]:
            s=fixture();s[field]=value
            with self.assertRaises(Exception): self.run_translation(s)
        with tempfile.TemporaryDirectory() as d:
            p=Path(d)/'policy.json';p.write_text('{"version":"future"}')
            with self.assertRaises(ValueError):self.run_translation(policy_path=p)
    def test_input_ranges_and_identity(self):
        for value in [-1,128,True,1.5]:
            s=fixture();s['parameters']['vcf.cutoff']['raw']=value
            with self.assertRaises(Exception):self.run_translation(s)
        s=fixture();s['parameters']['vcf.cutoff']['cc']=71
        with self.assertRaises(ValueError):self.run_translation(s)
        s=fixture();s['parameters']['invented']=s['parameters'].pop('vcf.cutoff')
        with self.assertRaises(ValueError):self.run_translation(s)
    def test_missing_and_strict(self):
        s=fixture();del s['parameters']['vcf.cutoff']
        r,report=self.run_translation(s);self.assertEqual(r['values']['P_E4'],0)
        self.assertEqual(report['inputs']['vcf.cutoff']['disposition'],'missing/defaulted')
        with self.assertRaises(ValueError): self.run_translation(s,strict=True)
    def test_endpoints_rounding_and_clamps(self):
        self.assertEqual([scaled(x,63) for x in [-9,0,64,127,200]],[0,0,32,63,63])
        for x in [0,127]:
            s=fixture()
            for v in s['parameters'].values():v['raw']=x
            r,_=self.run_translation(s)
            self.assertEqual(r['values']['P_ED_FLT'],0 if x==0 else 63)
            self.assertEqual(r['values']['P_ATK'],x)
    def test_selector_unknown_is_not_decoded(self):
        a=fixture();b=copy.deepcopy(a)
        for key in ['vco.range','voice.assign_mode','vco.pwm_source','lfo.waveform','vca.envelope_switch','envelope.mode']:
            b['parameters'][key]['raw']=127
        self.assertEqual(self.run_translation(a)[0],self.run_translation(b)[0])
        for key in ['vco.range','lfo.waveform']:
            self.assertEqual(self.run_translation(b)[1]['inputs'][key]['disposition'],'unsupported')
    def test_mixed_wave_fallback_losses(self):
        for saw,pulse,sub,wave,loss in [(108,25,54,6,'Pulse component dropped'),(0,124,84,4,'Sub component dropped'),(0,0,0,0,'No pitched source mixer gain'),(0,0,40,6,None)]:
            s=fixture()
            for k,v in [('saw',saw),('pulse',pulse),('sub',sub)]:s['parameters']['mixer.'+k+'_level']['raw']=v
            r,report=self.run_translation(s);self.assertEqual(r['values']['P_E0'],wave)
            if loss:self.assertTrue(any(loss in l for l in report['losses']))
    def test_overrides_bounded_and_recorded(self):
        r,report=self.run_translation(overrides={'P_E4':{'value':90,'reason':'explicit design choice'}})
        self.assertEqual(r['values']['P_E4'],90);self.assertIn('explicit design choice',report['outputs']['P_E4']['transform'])
        for o in [{'P_E4':{'value':128,'reason':'bad'}},{'P_LEVEL':{'value':10,'reason':'bad'}},{'P_E4':{'value':4,'reason':''}},{'P_VOICE':{'value':4,'reason':'bad'}}]:
            with self.assertRaises(ValueError):self.run_translation(overrides=o)
    def test_hash_state_and_synthetic_rejection(self):
        for key,value in [('modified_since_load',True),('raw_log',dict(file='events.jsonl',bytes=5,sha256=sha(b'test')))]:
            s=fixture();s[key]=value
            with self.assertRaises(ValueError):self.run_translation(s)
        s=fixture();s['source']['evidence']='synthetic'
        with self.assertRaises(ValueError):self.run_translation(s)
    def test_bank_wrapper_and_old_recipes(self):
        generate(True);bank=read_bank();self.assertEqual([r['preset'] for r in bank],[13,14])
        source=(ROOT/'felucca/firmware/engines/src/eng_analog.c').read_text()
        old=source.split('static const preset_t ANALOG_PRESETS[] = {')[1].split('\n};')[0]
        wrapper=(ROOT/'felucca/firmware/src/bgirl_carrier.h').read_text()
        self.assertIn(old,wrapper)
        self.assertIn('{"BG CARR SAW", {0,0,0,0,127,0,0,0}, {0,0,127,20}, 0,1,FX(0,0,0,0),PAT(4)},',wrapper)
    @unittest.skipUnless(os.environ.get('SH_STYLE_SOURCE_ROOT'),'private sources intentionally not in public repository')
    def test_real_sources_reproduce_exact_bytes(self):
        base=Path(os.environ['SH_STYLE_SOURCE_ROOT'])
        for bundle,ident,name,preset in [('proving-patch-1-1-reloaded','proving-1-1','SH 1-1',13),('second-saved-patch-wire1-repeat','wire1','SH WIRE1',14)]:
            paths=[base/bundle/f for f in ['patch.json','events.jsonl']];before=[p.read_bytes() for p in paths]
            r,report=translate_bundle(base/bundle,name,preset,strict=True)
            for value,suffix in [(r,'recipe'),(report,'report')]:self.assertEqual(artifact_bytes(value),(ROOT/f'assets/sh-style/{ident}.{suffix}.json').read_bytes())
            self.assertEqual(before,[p.read_bytes() for p in paths])
if __name__=='__main__':unittest.main()

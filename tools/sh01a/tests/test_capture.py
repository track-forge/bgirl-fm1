import ast
import contextlib
import hashlib
import io
import json
from pathlib import Path
import signal
import tempfile
import threading
import time
import unittest
from unittest.mock import patch
from types import SimpleNamespace
from tools.sh01a.core import Tracker, atomic, canonical, diff, read_log, replay, stale_status
from tools.sh01a.definitions import CONTROLS, PATCH_IDS
from tools.sh01a.__main__ import monitor, snapshot, queue

class Tests(unittest.TestCase):
 def setUp(self):
  self.t=Tracker(1,'test','synthetic','synthetic'); self.n=0
 def event(self,kind='midi',**data):
  self.n+=1; e=dict(kind=kind,seq=self.n,time='2026-11-18T12:00:00+00:00',**data)
  self.t.apply(e); return e
 def arm(self): return self.event('arm',label='synthetic, NOT hardware')
 def cc(self,cc,value,channel=1): return self.event(bytes=[175+channel,cc,value])
 def test_partial_startup(self):
  self.cc(74,10); self.assertEqual(self.t.values,{}); self.assertEqual(self.t.state,'uninitialized')
 def test_channel_filter(self):
  self.arm(); self.cc(74,10,2); self.assertFalse(self.t.values); self.assertEqual(self.t.filtered,1)
 def test_unknown_and_selector(self):
  self.arm(); self.cc(99,127); self.cc(12,65)
  self.assertEqual(self.t.unknown[0]['raw'],127); self.assertIsNone(self.t.values['lfo.waveform']['decoded'])
 def test_performance_not_coverage(self):
  self.arm(); self.cc(1,64); self.cc(76,64); self.event(bytes=[144,60,100]); self.event(bytes=[248])
  self.assertEqual(self.t.export()['confidence']['observed'],0)
 def test_duplicate_and_diff_revert(self):
  self.arm(); self.cc(74,10); self.event('baseline'); self.cc(74,11)
  self.assertTrue(self.t.export()['modified_since_load']); self.cc(74,10); self.cc(74,10)
  self.assertEqual(self.t.export()['changes'],[])
 def test_manual_transition_no_carry(self):
  self.arm(); self.cc(74,10); self.cc(75,20); self.event('baseline'); self.arm(); self.cc(74,30)
  self.assertNotIn('envelope.decay',self.t.values); self.assertIsNone(self.t.baseline_id)
 def test_program_before_and_after_burst(self):
  self.arm(); self.cc(75,20); self.event(bytes=[192,8]); self.cc(74,30)
  self.assertNotIn('envelope.decay',self.t.values); self.assertIn('vcf.cutoff',self.t.values)
  self.event(bytes=[192,9]); self.assertFalse(self.t.values); self.assertFalse(self.t.export()['confidence']['ready'])
 def test_disconnect_reconnect_requires_arm(self):
  self.arm(); self.cc(74,1); self.event('disconnect'); self.event('reconnect'); self.cc(74,2)
  self.assertEqual(self.t.state,'stale'); self.assertEqual(self.t.values['vcf.cutoff']['raw'],1)
  self.arm(); self.assertFalse(self.t.values)
 def test_missing_even_with_all_candidates_never_ready(self):
  self.arm()
  for cc,d in CONTROLS.items(): self.cc(cc,64)
  state=self.t.export(); self.assertEqual(state['confidence']['missing'],[])
  self.assertFalse(state['confidence']['ready']); self.assertEqual(state['confidence']['state'],'partial')
 def test_stop_stale(self):
  self.arm(); self.event('stop'); self.assertEqual(self.t.state,'stale')
 def test_stale_reader(self):
  self.assertTrue(stale_status(dict(heartbeat_epoch=0,running=True))['stale'])
  self.assertFalse(stale_status(dict(heartbeat_epoch=time.time(),running=True))['stale'])
  with self.assertRaises(ValueError): stale_status({},float('nan'))
 def test_reject_bad_messages(self):
  for b in ([176,74,128],[176,74],[1,2],[240,128,247],[249]):
   with self.assertRaises(ValueError): self.event(bytes=b)
 def test_reject_order(self):
  e=self.arm()
  with self.assertRaises(ValueError): self.t.apply(e)
 def test_replay_deterministic(self):
  events=[self.arm(),self.cc(74,44),self.event('baseline'),self.cc(74,45)]
  with tempfile.TemporaryDirectory() as d:
   p=Path(d)/'events'; p.write_bytes(b''.join(map(canonical,events)))
   a=replay(p,1,'test','synthetic').export(); b=replay(p,1,'test','synthetic').export()
   self.assertEqual(a,b); self.assertEqual(a,self.t.export())
   p.write_text('{bad}\n')
   with self.assertRaises(ValueError): read_log(p)
 def test_atomic_readers(self):
  with tempfile.TemporaryDirectory() as d:
   p=Path(d)/'state'; atomic(p,dict(i=0)); errors=[]
   def writer():
    for i in range(100): atomic(p,dict(i=i))
   thread=threading.Thread(target=writer); thread.start()
   while thread.is_alive():
    try: self.assertIn('i',json.loads(p.read_text()))
    except Exception as exc: errors.append(exc)
   thread.join(); self.assertFalse(errors)
 def test_full_inventory(self):
  self.assertEqual(len(CONTROLS),42)
  self.assertFalse(CONTROLS[11]['transmitted']); self.assertNotIn('system.tune',PATCH_IDS)
 def test_no_output_code(self):
  for p in Path('tools/sh01a').glob('*.py'):
   tree=ast.parse(p.read_text())
   for node in ast.walk(tree):
    if isinstance(node,ast.Attribute):
     self.assertNotIn(node.attr,('open_output','get_output_names','open_ioport','send','MidiOut'))
 def test_live_input_spy_and_snapshot(self):
  class Message:
   def bytes(self): return [176,74,55]
  class Port:
   calls=0
   def iter_pending(self):
    self.calls+=1
    if self.calls==1: return [] # drain before arm
    if self.calls==2: return [Message()]
    signal.raise_signal(signal.SIGTERM); return []
   def close(self): pass
  class Backend:
   opened=[]
   def get_input_names(self): return ['synthetic']
   def open_input(self,name): self.opened.append(name); return Port()
   def __getattr__(self,name): raise AssertionError('forbidden backend access: '+name)
  with tempfile.TemporaryDirectory() as d:
   session=Path(d)/'session'; session.mkdir(); (session/'commands').mkdir()
   queue(session,'arm','synthetic')
   b=Backend()
   with contextlib.redirect_stdout(io.StringIO()): monitor(SimpleNamespace(session=str(session),port='synthetic',channel=1),b)
   self.assertEqual(b.opened,['synthetic']); state=json.loads((session/'current.json').read_text())
   self.assertEqual(state['parameters']['vcf.cutoff']['raw'],55)
   status=json.loads((session/'status.json').read_text()); self.assertFalse(status['running'])
   out=Path(d)/'export'
   args=SimpleNamespace(session=str(session),output=str(out),source_revision='test',stale_after=5)
   snapshot(args); saved=json.loads((out/'patch.json').read_text())
   self.assertEqual(hashlib.sha256((out/'events.jsonl').read_bytes()).hexdigest(),saved['raw_log']['sha256'])
   self.assertEqual(saved['confidence']['state'],'stale')
   with self.assertRaises(FileExistsError): snapshot(args)
   with self.assertRaises(ValueError): monitor(SimpleNamespace(session=str(session),port='synthetic',channel=1),b)
 def test_schema(self):
  import jsonschema
  self.arm(); self.cc(74,55)
  state=self.t.export(); state.update(updated_at='2026-11-18T00:00:00+00:00',raw_log=dict(file='events.jsonl',sha256='0'*64,bytes=0))
  schema=json.loads(Path('tools/sh01a/snapshot.schema.json').read_text())
  jsonschema.validate(state,schema)
  state['parameters']['vcf.cutoff']['raw']=128
  with self.assertRaises(jsonschema.ValidationError): jsonschema.validate(state,schema)

if __name__=='__main__': unittest.main()

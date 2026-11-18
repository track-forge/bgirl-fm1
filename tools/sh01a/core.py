"""Dependency-free state, strict raw log replay, and atomic storage. GPL-3.0-only."""
import copy
import datetime
import hashlib
import json
import math
import os
from pathlib import Path
import time
import uuid
from .definitions import CONTROLS, PATCH_IDS, VERSION

LIMITATIONS = [
 'Input only; no dump request, SysEx, program select, or hardware write.',
 'Manual arm BEFORE patch load required: no verified automatic delimiter/order.',
 'Quiet time is not completeness. No hardware-verified transmitted profile yet.',
 'Physical units, selector thresholds, dropped messages and MIDI echo unverified.',
 'Program changes invalidate state; automatic attribution intentionally unsupported.',
 'System/performance CCs excluded from patch coverage; internal state may be unobservable.',
]

def utc():
 return datetime.datetime.now(datetime.timezone.utc).isoformat()

def canonical(value):
 return (json.dumps(value,sort_keys=True,separators=(',',':'),allow_nan=False)+'\n').encode()

def atomic(path,value):
 path=Path(path)
 tmp=path.with_name('.'+path.name+'.'+uuid.uuid4().hex+'.tmp')
 try:
  with tmp.open('xb') as f:
   f.write(canonical(value)); f.flush(); os.fsync(f.fileno())
  os.replace(tmp,path)
 finally:
  if tmp.exists(): tmp.unlink()

def diff(baseline,current):
 return [dict(parameter=k,before=baseline.get(k,{}).get('raw'),after=current.get(k,{}).get('raw'),
              kind='added' if k not in baseline else 'removed' if k not in current else 'changed')
         for k in sorted(set(baseline)|set(current))
         if baseline.get(k,{}).get('raw')!=current.get(k,{}).get('raw')]

def validate_event(e):
 if not isinstance(e,dict) or e.get('kind') not in ('midi','arm','baseline','disconnect','reconnect','stop'):
  raise ValueError('invalid event kind')
 if type(e.get('seq')) is not int or e['seq']<1: raise ValueError('invalid sequence')
 if not isinstance(e.get('time'),str): raise ValueError('missing timestamp')
 datetime.datetime.fromisoformat(e['time'])
 if e['kind']=='arm' and (not isinstance(e.get('label'),str) or not e['label'].strip()):
  raise ValueError('arm requires patch label')
 if e['kind']=='midi':
  b=e.get('bytes')
  if not isinstance(b,list) or not b or any(type(x) is not int or not 0<=x<=255 for x in b):
   raise ValueError('invalid MIDI bytes')
  s=b[0]
  if s<128: raise ValueError('missing MIDI status')
  if s<240:
   size=2 if s>>4 in (12,13) else 3
   if len(b)!=size or any(x>127 for x in b[1:]): raise ValueError('invalid channel message')
  elif s==240:
   if len(b)<2 or b[-1]!=247 or any(x>127 for x in b[1:-1]): raise ValueError('invalid SysEx')
  else:
   size={241:2,242:3,243:2,246:1,248:1,250:1,251:1,252:1,254:1,255:1}.get(s)
   if size is None or len(b)!=size or any(x>127 for x in b[1:]): raise ValueError('invalid system message')
 return e

class Tracker:
 def __init__(self,channel,session_id,port,evidence='hardware'):
  if type(channel) is not int or not 1<=channel<=16: raise ValueError('channel must be 1..16')
  self.channel=channel; self.session_id=session_id; self.port=port; self.evidence=evidence
  self.seq=0; self.state='uninitialized'; self.values={}; self.baseline={}; self.baseline_id=None
  self.capture_id=None; self.label=None; self.unknown=[]; self.filtered=0; self.unknown_count=0
  self.counts={}; self.programs=[]; self.reason='arm capture before loading a saved patch'
  self.disconnected=False; self.armed=False; self.ambiguities=[]
 def apply(self,e):
  validate_event(e)
  if e['seq']<=self.seq: raise ValueError('non-monotonic sequence; out-of-order log rejected')
  kind=e['kind']
  if kind=='baseline' and (not self.armed or self.disconnected or not self.values):
   raise ValueError('no active capture values')
  self.seq=e['seq']
  if kind=='arm':
   self.values={}; self.baseline={}; self.baseline_id=None; self.unknown=[]; self.unknown_count=0; self.programs=[]
   self.capture_id=e.get('capture_id',str(uuid.uuid5(uuid.NAMESPACE_URL,self.session_id+':'+str(self.seq))))
   self.label=e['label']; self.armed=True; self.ambiguities=[]
   self.state='capturing' if not self.disconnected else 'stale'
   self.reason='manual window; waiting for received patch values'
  elif kind=='baseline':
   if not self.armed or self.disconnected or not self.values: raise ValueError('no active capture values')
   self.baseline=copy.deepcopy(self.values); self.baseline_id=self.capture_id+':'+str(self.seq)
   self.state='partial'; self.reason='manual baseline; transmitted profile not hardware-verified'
  elif kind in ('disconnect','reconnect'):
   self.disconnected=kind=='disconnect'; self.armed=False; self.state='stale'
   self.reason=kind+'; arm and reload required'
  elif kind=='stop':
   self.state='stale'; self.armed=False; self.reason='monitor stopped'
  elif kind=='midi':
   b=e['bytes']; status=b[0]; typ=status>>4
   if status<240 and (status&15)+1!=self.channel:
    self.filtered+=1; return
   self.counts[str(status if status>=240 else typ)]=self.counts.get(str(status if status>=240 else typ),0)+1
   if typ==12:
    self.ambiguities.append('Program Change seen: order/delimiter unverified; compare raw log before acceptance')
    self.ambiguities=self.ambiguities[-16:]
    self.programs.append(dict(wire_program=b[1],seq=self.seq,time=e['time']))
    self.programs=self.programs[-64:]
    self.values={}; self.baseline={}; self.baseline_id=None
    self.state='partial'; self.reason='program change ordering unverified; values cleared, raw retained; manual recapture may be required'
   elif typ==11:
    cc,value=b[1:]; definition=CONTROLS.get(cc)
    if definition is None:
     self.unknown_count+=1
     self.unknown.append(dict(cc=cc,raw=value,seq=self.seq,time=e['time']))
     self.unknown=self.unknown[-128:]
    elif self.armed and not self.disconnected:
     self.values[definition['parameter']]=dict(cc=cc,raw=value,decoded=None,
       decoding=definition['decoding'],seq=self.seq,time=e['time'],category=definition['category'])
     self.state='partial'; self.reason='received values; manual boundary and unverified coverage'
 def export(self):
  patch={k:v for k,v in self.values.items() if v['category']=='patch'}
  missing=sorted(set(PATCH_IDS)-set(patch))
  return dict(schema_version=1,device_model='SH-01A',tracker_version='1',definition_version=VERSION,
   session_id=self.session_id,capture_id=self.capture_id,patch_identity=dict(label=self.label,provenance='manual'),
   source=dict(port=self.port,channel=self.channel,evidence=self.evidence),sequence=self.seq,
   confidence=dict(state=self.state,reasons=[self.reason]+self.ambiguities,profile='documented-transmit-candidates/unverified',
     missing=missing,observed=len(set(PATCH_IDS)&set(patch)),expected=len(PATCH_IDS),
     disconnect=self.disconnected,overflow='not_detectable',ready=False),
   parameters=copy.deepcopy(self.values),baseline=copy.deepcopy(self.baseline),baseline_id=self.baseline_id,
   modified_since_load=bool(self.baseline_id and diff(self.baseline,self.values)),
   changes=diff(self.baseline,self.values) if self.baseline_id else [],
   unknown_messages=copy.deepcopy(self.unknown),unknown_count=self.unknown_count,
   wire_program_observations=copy.deepcopy(self.programs),filtered_messages=self.filtered,
   message_counts=dict(self.counts),limitations=LIMITATIONS)

def read_log(path):
 events=[]; last=0
 with Path(path).open() as f:
  for line in f:
   e=validate_event(json.loads(line))
   if e['seq']<=last: raise ValueError('non-monotonic log')
   events.append(e); last=e['seq']
 return events

def replay(path,channel,session_id='offline',port='offline',evidence='synthetic'):
 t=Tracker(channel,session_id,port,evidence)
 for e in read_log(path): t.apply(e)
 return t

def stale_status(status,threshold=5):
 if not math.isfinite(threshold) or threshold<=0: raise ValueError('positive finite stale threshold required')
 result=copy.deepcopy(status)
 age=time.time()-result['heartbeat_epoch']
 result['age_seconds']=age
 result['stale']=not result.get('running',False) or age>threshold or age<0
 return result

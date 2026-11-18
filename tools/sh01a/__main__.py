import argparse
import contextlib
import fcntl
import hashlib
import json
import os
from pathlib import Path
import signal
import sys
import time
import uuid
from .core import Tracker, atomic, canonical, diff, replay, stale_status, utc
from .definitions import CONTROLS


def midi_backend():
 # Imported only for live input. Never instantiate MidiOut or call output APIs.
 import mido
 return mido


def queue(session,kind,label=None):
 d=Path(session)/'commands'
 if not d.is_dir(): raise ValueError('start monitor first')
 command=dict(kind=kind,id=str(uuid.uuid4()),requested_at=utc())
 if label is not None: command['label']=label
 atomic(d/(str(time.time_ns())+'-'+command['id']+'.json'),command)
 return command


def monitor(args,backend=None):
 backend=backend or midi_backend()
 d=Path(args.session); d.mkdir(parents=True,exist_ok=True)
 with (d/'writer.lock').open('a') as lock:
  try: fcntl.flock(lock,fcntl.LOCK_EX|fcntl.LOCK_NB)
  except BlockingIOError: raise ValueError('session already has a writer')
  if (d/'events.jsonl').exists(): raise ValueError('session exists: use a fresh session directory (no silent restart)')
  if args.port not in backend.get_input_names(): raise ValueError('exact input port not found')
  (d/'commands').mkdir(exist_ok=True)
  run=str(uuid.uuid4()); tracker=Tracker(args.channel,run,args.port,'hardware')
  running=True; port=None; last_command=None; digest=hashlib.sha256(); raw_size=0
  def stop(*unused):
   nonlocal running
   running=False
  previous={s:signal.signal(s,stop) for s in (signal.SIGINT,signal.SIGTERM)}
  with (d/'events.jsonl').open('xb') as raw:
   def emit(kind,**data):
    nonlocal raw_size
    event=dict(kind=kind,seq=tracker.seq+1,time=utc(),**data)
    tracker.apply(event)
    line=canonical(event); raw.write(line); raw.flush(); os.fsync(raw.fileno())
    digest.update(line); raw_size+=len(line)
   def publish():
    state=tracker.export()
    state.update(updated_at=utc(),raw_log=dict(file='events.jsonl',sha256=digest.hexdigest(),bytes=raw_size))
    atomic(d/'current.json',state)
    atomic(d/'status.json',dict(session_id=run,sequence=tracker.seq,running=running,
      heartbeat_epoch=time.time(),confidence=state['confidence'],last_command=last_command,
      source=state['source'],raw_log=state['raw_log']))
   try:
    port=backend.open_input(args.port)
    publish(); print('INPUT ONLY: listening; arm before manual patch load',flush=True)
    next_publish=0; next_ports=0
    while running:
     # Commands first: old queued MIDI is drained to raw log before new arm.
     for request in sorted((d/'commands').glob('*.json')):
      command=json.loads(request.read_text())
      if command['kind'] not in ('arm','baseline'): raise ValueError('unsupported command')
      if port:
       for msg in port.iter_pending(): emit('midi',bytes=msg.bytes())
      try:
       emit(command['kind'],**({'label':command['label']} if command['kind']=='arm' else {}))
       last_command=dict(id=command['id'],result='accepted',sequence=tracker.seq)
      except ValueError as exc:
       last_command=dict(id=command['id'],result='rejected',error=str(exc))
      request.unlink()
      publish()
     if port:
      try:
       for msg in port.iter_pending(): emit('midi',bytes=msg.bytes())
      except (OSError,RuntimeError):
       port.close(); port=None; emit('disconnect')
     now=time.monotonic()
     if now>=next_ports:
      names=backend.get_input_names(); next_ports=now+2
      if port and args.port not in names:
       port.close(); port=None; emit('disconnect')
      elif port is None and args.port in names:
       try: port=backend.open_input(args.port)
       except (OSError,RuntimeError): pass
       else: emit('reconnect')
     if now>=next_publish: publish(); next_publish=now+.5
     time.sleep(.01)
   finally:
    running=False
    if port: port.close()
    emit('stop'); publish()
    for sig,handler in previous.items(): signal.signal(sig,handler)


def snapshot(args):
 d=Path(args.session); state=json.loads((d/'current.json').read_text())
 status=stale_status(json.loads((d/'status.json').read_text()),args.stale_after)
 if state['session_id']!=status['session_id']: raise ValueError('session mismatch')
 if not state['capture_id'] or not state['parameters']: raise ValueError('no armed capture values')
 if status['stale']:
  state['confidence']['state']='stale'; state['confidence']['reasons'].append('reader: monitor heartbeat stale/stopped')
 raw_meta=state['raw_log']
 with (d/'events.jsonl').open('rb') as f: raw=f.read(raw_meta['bytes'])
 if hashlib.sha256(raw).hexdigest()!=raw_meta['sha256']: raise ValueError('raw prefix hash mismatch')
 # A unique directory publishes immutable JSON + exact raw prefix, never overwrite.
 out=Path(args.output); out.mkdir(parents=False,exist_ok=False)
 state['exported_at']=utc(); state['export_id']=str(uuid.uuid4())
 state['provenance']=dict(license='not granted for redistribution; private operator capture',
   author='operator supplied',source_revision=args.source_revision)
 (out/'events.jsonl').write_bytes(raw)
 atomic(out/'patch.json',state)
 return dict(output=str(out),capture_id=state['capture_id'],confidence=state['confidence'],raw_log=raw_meta)


def main():
 p=argparse.ArgumentParser(description='Read-only SH-01A capture; no output MIDI APIs.')
 sub=p.add_subparsers(dest='command',required=True)
 sub.add_parser('list-ports'); sub.add_parser('inventory')
 m=sub.add_parser('monitor'); m.add_argument('--port',required=True); m.add_argument('--channel',type=int,required=True); m.add_argument('--session',required=True)
 for name in ('status','arm','baseline','snapshot'):
  q=sub.add_parser(name); q.add_argument('--session',required=True)
  if name=='arm': q.add_argument('--label',required=True)
  if name in ('status','snapshot'): q.add_argument('--stale-after',type=float,default=5)
  if name=='snapshot':
   q.add_argument('--output',required=True); q.add_argument('--source-revision',required=True)
 r=sub.add_parser('replay'); r.add_argument('events'); r.add_argument('--channel',required=True,type=int)
 r.add_argument('--session-id',default='offline'); r.add_argument('--port',default='offline')
 r.add_argument('--evidence',choices=['synthetic','hardware'],default='synthetic'); r.add_argument('--output',required=True)
 df=sub.add_parser('diff'); df.add_argument('--baseline',required=True); df.add_argument('--current',required=True)
 args=p.parse_args()
 try:
  if args.command=='monitor': monitor(args); return
  if args.command=='list-ports': result=midi_backend().get_input_names()
  elif args.command=='inventory': result=CONTROLS
  elif args.command in ('arm','baseline'): result=queue(args.session,args.command,getattr(args,'label',None))
  elif args.command=='status':
   result=stale_status(json.loads((Path(args.session)/'status.json').read_text()),args.stale_after)
   if result['stale']: result['confidence']['state']='stale'
  elif args.command=='snapshot': result=snapshot(args)
  elif args.command=='replay':
   result=replay(args.events,args.channel,args.session_id,args.port,args.evidence).export()
   # Exclusive output, no accidental replacement of a capture.
   with Path(args.output).open('xb') as f: f.write(canonical(result))
  elif args.command=='diff':
   result=diff(json.loads(Path(args.baseline).read_text())['parameters'],json.loads(Path(args.current).read_text())['parameters'])
  print(json.dumps(result,indent=2,allow_nan=False))
 except (ValueError,OSError,KeyError,ImportError) as exc:
  p.exit(1,'sh01a: '+str(exc)+'\n')

if __name__=='__main__': main()

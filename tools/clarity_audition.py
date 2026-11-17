#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-only
"""Reproduce bounded real-DSP comparisons, offline RMS match and six-track timings.
Run from repository root after ./build.sh --web. All artifacts stay ignored/local.
"""
import argparse, cmath, hashlib, html, json, math, platform, shutil, subprocess, sys, wave
from pathlib import Path
import numpy as np
sys.path.insert(0,str(Path(__file__).resolve().parent))
from vocoder_coeff import BANKS,coefficients
from clarity_source_audit import audit
ROOT=Path(__file__).resolve().parents[1]
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def run(args):return subprocess.check_output(args,cwd=ROOT,text=True)
def compile_(dest,*flags):
 run(['cc','-O2','-w','-ffp-contract=off',*flags,'-Ifelucca/build/gen','-Ifelucca/firmware/src','-Ifelucca/firmware/hal','-Ifelucca/firmware/bsp/chip','-Ifelucca/firmware/bsp/boards/fm1','tools/render_clarity.c','-lm','-o',str(dest)])
def read(p):
 with wave.open(str(p)) as f:return np.frombuffer(f.readframes(f.getnframes()),dtype='<i2').astype(float)/32768
CASES=[('dry',4,2,150,0,127),('carrier-saw',3,2,150,0,127),('carrier-grain',3,2,150,1,127),
 ('whole-12',0,1,150,0,127),('whole-24',0,2,150,0,127),('release-50',0,2,50,0,127),('release-100',0,2,100,0,127),
 ('grain-12',0,1,150,1,127),('grain-24',0,2,150,1,127),('blend-24',0,2,150,0,117),('chops-24',1,2,150,0,127),('arrangement',2,2,150,0,127)]
def coverage():
 result=[]
 for name,n,lo,hi,q in BANKS:
  rows=coefficients(n,lo,hi,q);freq=np.geomspace(lo,hi,2000)
  def response(row,f):
   z=cmath.exp(-2j*math.pi*f/44100);b0,b1,b2,a1,a2=row
   return (b0+b1*z+b2*z*z)/(1+a1*z+a2*z*z)
  power=np.array([sum(abs(response(row,f))**2 for row in rows) for f in freq])
  # The two cascaded bank edges, not a claim of brick-wall coverage.
  def edges(center):
   w=2*math.pi*center/44100;alpha=math.sin(w)/(2*q)
   poles=np.roots([1,-2*math.cos(w)/(1+alpha),(1-alpha)/(1+alpha)])
   return max(abs(poles))
  edges_f=[]
  for row,center in [(rows[0],lo),(rows[-1],hi)]:
   sweep=np.geomspace(max(1,center/3),min(22049,center*3),20000)
   selected=[f for f in sweep if abs(response(row,f))>=2**-.5]
   edges_f.append([min(selected),max(selected)])
  result.append(dict(name=name,bands=n,centers_hz=[lo,hi],q=q,power_sum_db=[10*math.log10(min(power)),10*math.log10(max(power))],outer_3db_hz=[edges_f[0][0],edges_f[1][1]],max_pole_radius=max(edges(lo),edges(hi))))
 return result

def main():
 ap=argparse.ArgumentParser();ap.add_argument('--output',default='local/clarity');args=ap.parse_args();out=(ROOT/args.output).resolve();out.mkdir(parents=True,exist_ok=True)
 compile_(out/'render');compile_(out/'render-narrow','-DBGIRL_VOCODER_NARROW24=1')
 reports=[]
 for name,mode,bands,ms,carrier,wet in CASES+[('narrow-24',0,2,150,0,127)]:
  p=out/(name+'.wav');binary=out/('render-narrow' if name=='narrow-24' else 'render')
  r=json.loads(run([str(binary),str(p),str(mode),str(bands),str(ms),str(carrier),str(wet)]));r.update(name=name,sha256=sha(p));reports.append(r)
 # Whole-file RMS incl deliberate silence, all12s files; choose common target
 # no louder than -24dBFS or any file's gain-to-.9peak limit. No limiter/compression.
 target=min(10**(-24/20),*[r['rms']*.9/r['peak'] for r in reports if r['peak']])
 for r in reports:
  a=read(out/(r['name']+'.wav'));gain=target/(float(np.sqrt(np.mean(a*a))) or 1);p=out/(r['name']+'-matched.wav')
  with wave.open(str(p),'wb') as f:f.setparams((2,2,44100,0,'NONE',''));f.writeframes(np.rint(a*gain*32767).astype('<i2').tobytes())
  r.update(match_gain_db=20*math.log10(gain),matched_sha256=sha(p))
 timings=[]
 for repeat in range(5):
  for bands in ([1,2] if repeat%2==0 else [2,1]):
   r=json.loads(run([str(out/'render'),'-','5',str(bands),'150','0','127']));r['repeat']=repeat;timings.append(r)
 report=dict(revision=run(['git','rev-parse','HEAD']).strip(),source_dirty=bool(run(['git','status','--porcelain'])),compiler=run(['cc','--version']).splitlines()[0],python=platform.python_version(),numpy=np.__version__,machine=platform.platform(),cpu=Path('/proc/cpuinfo').read_text().split('model name')[1].splitlines()[0].lstrip('\t: '),block=128,rate=44100,matching=dict(method='whole-file RMS, silence retained, fixed scalar per file, no compression/limiting',target_dbfs=20*math.log10(target),peak_ceiling=.9),cases=reports,timings=timings,coverage=coverage(),source_audit=audit(),source={'path':'assets/vocals/g6/T75_S008_candidate_float.wav','sha256':sha(ROOT/'assets/vocals/g6/T75_S008_candidate_float.wav')},wasm_sha256=sha(ROOT/'felucca/build/emu/felucca.wasm'),human_approval='pending',hardware_feasibility='unverified')
 (out/'manifest.json').write_text(json.dumps(report,indent=2)+'\n')
 body='''<!doctype html><meta charset="utf-8"><meta name="viewport" content="width=device-width"><title>BGIRL clarity comparisons</title><style>body{background:#101b17;color:#e0ffe9;font:16px system-ui;max-width:900px;margin:2em auto;padding:1em}audio{width:min(100%,350px)}section{border-top:1px solid;padding:1em 0}a{color:#a4ffc2}</style><h1>BGIRL · vocoder clarity</h1><p>Human listening pending; host timing is not hardware proof. Play the complete phrase, then compare. No autoplay. Raw and matched copies retained.</p>'''
 body+=f'<p>Revision {html.escape(report["revision"])}; dirty={report["source_dirty"]}. Source: public-authorized G6/W01, original speed. 44.1kHz stereo, 12s. Matching: whole-file RMS incl silence to {20*math.log10(target):.2f} dBFS, fixed gain, no limiter.</p><p>Compare whole-12 vs whole-24 first. Then release-50/100, grain, blend, and narrow (180–6500/Q6) vs whole-24 (100–9000/Q5). All isolated tests use MIDI46 carrier, velocity94, release151.107ms unless labeled; gain8 and unity trims/color. Chops/arrangement change phrasing, not isolated DSP comparisons.</p><a href="manifest.json">Exact settings, hashes and measurements</a>'
 for r in reports:
  name=r['name'];body+=f'<section><h2>{name}</h2><p>{[8,12,24][r["bands"]]} bands · {r["release_ms"]:.3f}ms · wet{r["wet"]}/127 · peak{r["peak"]:.4f} · matching{r["match_gain_db"]:+.2f}dB</p><label>Matched <audio controls preload="none" src="{name}-matched.wav"></audio></label><label>Raw <audio controls preload="none" src="{name}.wav"></audio></label></section>'
 (out/'index.html').write_text(body)
 dest=ROOT/'felucca/build/emu/listening/clarity';dest.mkdir(parents=True,exist_ok=True)
 for p in out.iterdir():
  if p.suffix in ('.wav','.json','.html'):shutil.copyfile(p,dest/p.name)
 print(json.dumps({'files':len(reports),'target_dbfs':20*math.log10(target),'report':str(out/'manifest.json'),'timing_runs':len(timings)}))
if __name__=='__main__':main()

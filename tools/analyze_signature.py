#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-only
"""Measure renders without normalization; write loudness-matched copies separately.
Requires only Python stdlib. Usage: python3 tools/analyze_signature.py local/signature
"""
import array, hashlib, json, math, sys, wave
from pathlib import Path
root=Path(sys.argv[1]);out=root/'matched';out.mkdir(exist_ok=True)
results=[]
for path in sorted(root.glob('*.wav')):
    with wave.open(str(path)) as w:
        assert w.getnchannels()==2 and w.getsampwidth()==2 and w.getframerate()==44100
        raw=w.readframes(w.getnframes());samples=array.array('h',raw)
    if sys.byteorder!='little':samples.byteswap()
    peak=max(abs(v) for v in samples)/32768
    rms=math.sqrt(sum(v*v for v in samples)/len(samples))/32768
    threshold=peak*.001*32768
    end=max((i//2+1 for i,v in enumerate(samples) if abs(v)>threshold),default=0)/44100
    mono=[(samples[i]+samples[i+1])/65536 for i in range(0,len(samples),2)]
    mono_rms=math.sqrt(sum(v*v for v in mono)/len(mono))
    row=dict(file=path.name,sha256=hashlib.sha256(path.read_bytes()).hexdigest(),frames=len(mono),
             peak=peak,rms=rms,mono_rms=mono_rms,dc=sum(mono)/len(mono),last_above_relative_minus60dB_seconds=end)
    if 'sub-' in path.name and '-dry' in path.name:
        x=mono[44100:88200];cross=[]
        for i in range(1,len(x)):
            if x[i-1]<0<=x[i]:cross.append(i-1-x[i-1]/(x[i]-x[i-1]))
        hz=44100*(len(cross)-1)/(cross[-1]-cross[0]);row['fundamental_hz']=hz
        def magnitude(freq):
            a=b=0
            for i,v in enumerate(x):
                v*=.5-.5*math.cos(2*math.pi*i/(len(x)-1))
                a+=v*math.cos(2*math.pi*freq*i/44100);b+=v*math.sin(2*math.pi*freq*i/44100)
            return math.hypot(a,b)
        row['second_harmonic_ratio']=magnitude(2*hz)/magnitude(hz)
        assert abs(hz-65.4064)<.1
    # RMS-match whole files to -26dBFS, capped at -6dBFS peak. Retain original.
    gain=min(10**(-26/20)/(rms or 1),.5/(peak or 1))
    matched=array.array('h',(round(v*gain) for v in samples))
    if sys.byteorder!='little':matched.byteswap()
    with wave.open(str(out/path.name),'wb') as w:w.setparams((2,2,44100,0,'NONE','not compressed'));w.writeframes(matched.tobytes())
    row['comparison_gain_db']=20*math.log10(gain);results.append(row)
(root/'measurements.json').write_text(json.dumps(results,indent=2)+'\n')
print(json.dumps(results,indent=2))

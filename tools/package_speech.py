#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-only
"""Offline deterministic PCM trim/peak normalize/linear resample -> SLICE IMA."""
import hashlib, json, struct, sys, wave
from pathlib import Path
ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT/'felucca/tools'))
import sampleio
RATE = 22050

def package():
    base=ROOT/'assets/speech'
    manifest=json.loads((base/'manifest.json').read_text())
    pcm=[]; pos=[]; blob=bytearray(); assets=[]
    for a in manifest['phrases']:
        f=base/a['dry_source']
        assert hashlib.sha256(f.read_bytes()).hexdigest()==a['source_sha256']
        with wave.open(str(f)) as w:
            assert w.getnchannels()==1 and w.getsampwidth()==2
            sr=w.getframerate(); raw=struct.unpack('<%dh'%w.getnframes(),w.readframes(w.getnframes()))
        peak=max(map(abs,raw)); threshold=max(32,peak//100)
        first=next(i for i,x in enumerate(raw) if abs(x)>threshold)
        last=next(i for i in range(len(raw)-1,-1,-1) if abs(raw[i])>threshold)
        first=max(0,first-sr//40); last=min(len(raw),last+sr//20)
        raw=raw[first:last]
        n=len(raw)*RATE//sr
        x=[]
        for i in range(n):
            q,r=divmod(i*sr,RATE); v=(raw[q]*(RATE-r)+raw[min(q+1,len(raw)-1)]*r)//RATE
            v=v*23000//peak
            fade=min(i,n-1-i,RATE//200)
            x.append(v*fade//(RATE//200))
        if len(x)%2: x.append(0) # each phrase starts at a byte boundary
        pos.append(len(pcm)); pcm.extend(x)
        assets.append(dict(id=a['id'],samples=len(x),rate=RATE,trim_source_frames=[first,last]))
    # One stream ensures decoder state continues between phrase boundaries.
    encoded,_=sampleio.ima_encode(pcm,None); blob.extend(encoded)
    grid=[k*len(pcm)//128 for k in range(128)]
    checkpoints=set(pos+grid)
    states={}; pred=idx=0
    for n in range(len(pcm)):
        if n in checkpoints: states[n]=(pred&65535)|(idx<<16)
        code=(blob[n//2]>>(4*(n%2)))&15; step=sampleio.IMA_STEP[idx]
        vd=(step>>3)+(step if code&4 else 0)+(step>>1 if code&2 else 0)+(step>>2 if code&1 else 0)
        pred=max(-32768,min(32767,pred-vd if code&8 else pred+vd)); idx=max(0,min(88,idx+sampleio.IMA_IDX[code&7]))
    dest=ROOT/'felucca/build/gen'; dest.mkdir(parents=True,exist_ok=True)
    rows=[','.join(str(b) for b in blob[i:i+32])+',' for i in range(0,len(blob),32)]
    h='/* Generated offline by tools/package_speech.py; see assets/speech/manifest.json. */\n'
    h+='static const uint8_t BG_SPEECH_DATA[] = {\n'+'\n'.join(rows)+'\n};\n'
    h+='#define BG_SPEECH_INIT { .len=%du, .rate=%du, .nseg=1, .nauto=%d, .seg={{0,0,%du}}, .apos={%s}, .ast={%s} }\n'%(len(pcm),round(RATE/44100*65536),len(pos),len(pcm),','.join(map(str,pos)),','.join(str(states[p]) for p in pos))
    h=h.replace(".apos={", ".grid={"+",".join(str(states[p]) for p in grid)+"}, .apos={")
    (dest/'bgirl_speech.h').write_text(h)
    report={'version':1,'bank_id':'bgirl-speech-v1','codec':'IMA4 low-nibble-first','bytes':len(blob),'samples':len(pcm),'sha256':hashlib.sha256(blob).hexdigest(),'header_sha256':hashlib.sha256(h.encode()).hexdigest(),'phrases':assets}
    (dest/'bgirl-speech.bin').write_bytes(blob)
    (base/'packaged.json').write_text(json.dumps(report,indent=2)+'\n')
    print('speech:',len(blob),'bytes;',len(pcm)/RATE,'seconds; sha256',report['sha256'])
if __name__=='__main__': package()

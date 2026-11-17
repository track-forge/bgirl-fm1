#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-only
"""Measured public G6 pipeline audit, same box downsample/gain/fade/IMA as packager."""
import json,math,sys
from pathlib import Path
import numpy as np
sys.path.insert(0,str(Path(__file__).resolve().parent))
from package_vocals import read_wav, GAIN
import sampleio
ROOT=Path(__file__).resolve().parents[1]
def audit():
 data,rate,channels,mono=read_wav(ROOT/'assets/vocals/g6/T75_S008_candidate_float.wav')
 raw=np.array(mono);m=np.array([(mono[i]+mono[min(i+1,len(mono)-1)])/2 for i in range(0,len(mono),2)])
 fade=22050*3//1000;pcm=np.array([round(v*GAIN*32767*min(i,len(m)-1-i,fade)/fade) for i,v in enumerate(m)])
 blob,_=sampleio.ima_encode(pcm,None);pred=idx=0;decoded=[]
 for i in range(len(pcm)):
  code=(blob[i//2]>>(4*(i%2)))&15;step=sampleio.IMA_STEP[idx]
  delta=(step>>3)+(step if code&4 else 0)+(step>>1 if code&2 else 0)+(step>>2 if code&1 else 0)
  pred=max(-32768,min(32767,pred+(-delta if code&8 else delta)));idx=max(0,min(88,idx+sampleio.IMA_IDX[code&7]));decoded.append(pred)
 decoded=np.array(decoded)
 def spectrum(a,sr):
  power=abs(np.fft.rfft(a*np.hanning(len(a))))**2;f=np.fft.rfftfreq(len(a),1/sr);total=sum(power)
  return {f'{lo}-{hi}':float(sum(power[(f>=lo)&(f<hi)])/total*100) for lo,hi in [(0,100),(100,180),(180,6500),(6500,9000),(9000,11025),(11025,22051)]}
 return dict(source_rate=rate,source_channels=channels,packaged_rate=22050,source_seconds=len(raw)/rate,source_mono_energy_pct=spectrum(raw,rate),box_downsample_energy_pct=spectrum(m,22050),ima_decoded_energy_pct=spectrum(decoded,22050),ima_snr_db=10*math.log10(float(sum(pcm.astype(float)**2)/sum((pcm-decoded).astype(float)**2))),box_response_db={str(h):20*math.log10(abs(math.cos(math.pi*h/44100))) for h in [6500,9000,11025]},method='full phrase Hann FFT energy; IMA SNR against post-gain/fade PCM; no subjective articulation verdict',limitations='Two-point box is a weak anti-alias filter. Mono22.05kHz has no original detail above11.025kHz;44.1kHz output does not restore it. Pipeline redesign deferred.')
if __name__=='__main__':print(json.dumps(audit(),indent=2))

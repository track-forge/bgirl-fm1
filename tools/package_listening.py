#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-only
"""Lossless packaging of tests/render_vocoder.sh output; no loudness alteration."""
import hashlib, json, math, struct, subprocess, wave
from pathlib import Path
root=Path(__file__).resolve().parents[1]
rows=[]
for f in sorted((root/'tmp/listening').glob('*.wav')):
    with wave.open(str(f)) as w:
        n=w.getnframes(); rate=w.getframerate(); assert w.getnchannels()==2 and w.getsampwidth()==2
        x=struct.unpack('<%dh'%(n*2),w.readframes(n))
    dest=root/'assets/listening'/f.with_suffix('.flac').name
    subprocess.run(['ffmpeg','-v','error','-y','-i',str(f),'-map_metadata','-1','-c:a','flac',str(dest)],check=True)
    rows.append(dict(file=dest.name,seconds=n/rate,rate=rate,channels=2,sha256=hashlib.sha256(dest.read_bytes()).hexdigest(),bytes=dest.stat().st_size,peak=max(map(abs,x))/32768,rms=math.sqrt(sum(a*a for a in x)/len(x))/32768))
assert len(rows)==5
(root/'assets/listening/manifest.json').write_text(json.dumps(dict(human_listening='pending',source='tests/vocoder_test.c, same existing sequencer patterns and carrier, no post normalization',examples=rows),indent=2)+'\n')
print('Packaged five lossless9s comparisons; human intelligibility pending.')

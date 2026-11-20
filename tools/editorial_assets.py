#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-only
"""Build reproducible, bounded host-DSP listening excerpts. No private inputs."""
import array, json, math, subprocess, wave
from pathlib import Path
from PIL import Image
root=Path(__file__).resolve().parents[1]
out=root/'felucca/build/emu/editorial'
out.mkdir(parents=True,exist_ok=True)
binary=out/'render'
subprocess.run(['cc','-O2','-w','-ffp-contract=off',*[f'-Ifelucca/{p}' for p in ['build/gen','firmware/src','firmware/hal','firmware/bsp/chip','firmware/bsp/boards/fm1']],'tools/render_editorial.c','-lm','-o',str(binary)],cwd=root,check=True)
names=['dry','saw','trio','sub-clean','sub-drive','hit','pocket']
clips={}
for mode,name in enumerate(names):
    dest=out/name
    subprocess.run([str(binary),str(mode),str(dest)],cwd=root,check=True)
    samples=array.array('f');samples.frombytes(dest.with_suffix('.raw').read_bytes())
    rms=math.sqrt(sum(x*x for x in samples)/len(samples))
    assert rms>0 and all(math.isfinite(x) and abs(x)<=1 for x in samples)
    clips[name]=(samples,rms,max(map(abs,samples)))
    Image.open(dest.with_suffix('.ppm')).save(dest.with_suffix('.png'))
    dest.with_suffix('.raw').unlink();dest.with_suffix('.ppm').unlink()
# Fixed whole-excerpt RMS matching within comparison groups, no compressor.
manifest={}
for group in [['dry','saw','trio'],['sub-clean','sub-drive'],['hit','pocket']]:
    target=min(10**(-26/20),*[clips[n][1]*.85/clips[n][2] for n in group])
    for name in group:
        samples,rms,peak=clips[name];gain=target/rms
        pcm=array.array('h',(round(x*gain*32767) for x in samples))
        with wave.open(str(out/(name+'.wav')),'wb') as f:
            f.setparams((1,2,44100,0,'NONE',''));f.writeframes(pcm.tobytes())
        manifest[name]=dict(raw_rms=rms,gain=gain,matched_rms=target,peak=peak*gain)
Image.open(root/'docs/img/arp-dance/crisp-panel.png').save(out/'floorwork.png')
(out/'manifest.json').write_text(json.dumps(manifest,indent=2)+'\n')
binary.unlink()
print('Editorial: seven finite real-DSP excerpts; pairwise RMS matching; real screen crops.')

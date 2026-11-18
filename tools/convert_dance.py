#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-only
"""Offline, deterministic MP4 -> packed masks. No audio, network or generation.
Requires FFmpeg 7.1.5 and Pillow 12.1.1. See assets/dance/README.md.
"""
import argparse
import base64
import hashlib
import json
import re
import subprocess
from pathlib import Path
import PIL
from PIL import Image, ImageFilter, ImageDraw

SOURCE_SHA = 'acb274cd20f88d92a624453eb4b2eb4d1f3d7e2f3c687988e39475b207fd3d51'
SOURCE_URL = 'https://www.moltazine.com/post/a559b395-af6b-49c0-b6e9-08b11a202269'

def sha(b):
    return hashlib.sha256(b).hexdigest()

def pack(im):
    # Pillow mode 1 exports row-major MSB first, padded to whole bytes per row.
    return im.convert('1', dither=Image.Dither.NONE).tobytes()

def convert(source, dest, indices, expected_sha, source_url=SOURCE_URL, rights=None, clip=None, sizes=(64,96), framing='union', bounds_scope='all', padding=24, ping_pong=False):
    if clip and not re.fullmatch(r'[a-z][a-z0-9-]*',clip):
        raise ValueError('clip must be a lowercase named slug')
    if 96 not in sizes or any(s not in (64,96) for s in sizes):
        raise ValueError('sizes must include 96; supported sizes:64,96')
    if framing not in ('union','follow-x') or bounds_scope not in ('all','selected') or not isinstance(padding,int) or padding<8 or padding>96 or padding%2:
        raise ValueError('invalid framing, bounds scope or even source padding (8..96)')
    if expected_sha != SOURCE_SHA and (source_url == SOURCE_URL or not rights):
        raise ValueError("replacement art requires its own --source-url and --rights")
    rights = rights or "Doug Smith authorized public inclusion of derived masks under GPL-3.0-only on 2026-11-16"
    if PIL.__version__ != '12.1.1':
        raise ValueError('requires Pillow 12.1.1')
    version = subprocess.check_output(['ffmpeg', '-version'], text=True).splitlines()[0]
    if not version.startswith('ffmpeg version 7.1.5 '):
        raise ValueError('requires FFmpeg 7.1.5')
    data = source.read_bytes()
    if sha(data) != expected_sha:
        raise ValueError('source checksum mismatch; art replacement needs an explicit --sha256')
    probe = json.loads(subprocess.check_output(['ffprobe', '-v', 'error', '-select_streams', 'v:0', '-show_streams', '-of', 'json', str(source)]))['streams'][0]
    w, h = probe['width'], probe['height']
    raw = subprocess.check_output(['ffmpeg', '-v', 'error', '-threads', '1', '-i', str(source), '-map', '0:v:0', '-an', '-vsync', '0', '-f', 'rawvideo', '-pix_fmt', 'gray', '-threads', '1', '-'])
    count = len(raw) // (w*h)
    if len(indices) != 12 or len(set(indices)) != 12 or min(indices)<0 or max(indices)>=count:
        raise ValueError('exactly twelve distinct in-range frame indices required')
    frames = [Image.frombytes('L', (w,h), raw[i*w*h:(i+1)*w*h]).point(lambda p: 255 if p<96 else 0).filter(ImageFilter.MedianFilter(3)) for i in range(count)]
    framing_indices = range(count) if bounds_scope=='all' else indices
    bounds = [frames[i].getbbox() for i in framing_indices if frames[i].getbbox()]
    if not bounds or any(not frames[i].getbbox() for i in indices):
        raise ValueError('framing and selected poses must contain a silhouette')
    x0,y0,x1,y1 = min(b[0] for b in bounds),min(b[1] for b in bounds),max(b[2] for b in bounds),max(b[3] for b in bounds)
    width = x1-x0 if framing=='union' else max(b[2]-b[0] for b in bounds)
    side = max(width,y1-y0)+padding
    left, top = (x0+x1-side)//2, y1+padding//2-side  # common scale and floor anchor
    crop_lefts = [left if framing=='union' else (frames[i].getbbox()[0]+frames[i].getbbox()[2]-side)//2 for i in indices]
    dest.mkdir(parents=True,exist_ok=True)
    manifest = dict(source_url=source_url,source_sha256=sha(data),source_bytes=len(data),source_size=[w,h],fps=probe['r_frame_rate'],frame_count=count,indices=indices,union_bounds=[x0,y0,x1,y1],stage=[left,top,side],threshold=96,cleanup='3x3 median before resampling',resize='LANCZOS then >=128',bit_order='MSB first, row-major',toolchain={'ffmpeg':version,'pillow':PIL.__version__},rights=rights,sizes={})
    if framing!='union' or bounds_scope!='all' or padding!=24:
        manifest.update(framing=framing,bounds_scope=bounds_scope,source_padding=padding,crop_lefts=crop_lefts,pose_bounds=[frames[i].getbbox() for i in indices])
    for size in sizes:
        poses=[]
        for i,crop_left in zip(indices,crop_lefts):
            stage=Image.new('L',(side,side)); stage.paste(frames[i],(-crop_left,-top))
            poses.append(stage.resize((size,size),Image.Resampling.LANCZOS).point(lambda p:255 if p>=128 else 0))
        blob=b''.join(pack(p) for p in poses)
        (dest/f'poses-{size}.bin').write_bytes(blob)
        sheet=Image.new('RGB',(4*size*2,3*(size*2+20)),'#202830'); draw=ImageDraw.Draw(sheet)
        for k,p in enumerate(poses):
            x,y=(k%4)*size*2,(k//4)*(size*2+20)
            sheet.paste(p.resize((size*2,size*2),Image.Resampling.NEAREST).convert('RGB'),(x,y)); draw.text((x+4,y+size*2+2),f'{k}: frame {indices[k]}',fill='white')
        sheet.save(dest/f'contact-{size}.png')
        # Ghost the turnaround for ping-pong; retain the old hard-cut review otherwise.
        seam=Image.new('RGB',(size*6,size*2),'#202830')
        for k,(prev,cur) in enumerate([(10,11),(11,10),(1,0)] if ping_pong else [(10,11),(11,0),(0,1)]):
            cell=Image.new('RGB',(size,size),'#202830'); cell.paste('#586877',(0,0,size,size),poses[prev]); cell.paste('#eaf4f0',(0,0,size,size),poses[cur])
            seam.paste(cell.resize((size*2,size*2),Image.Resampling.NEAREST),(k*size*2,0))
        seam.save(dest/f'seam-ghost-{size}.png')
        manifest['sizes'][str(size)]={'bytes':len(blob),'sha256':sha(blob),'stride':size//8,'anchor':[size//2,round((y1-top)*size/side)]}
        if size==96:
            rows=[','.join(f'0x{v:02x}' for v in blob[i:i+24]) for i in range(0,len(blob),24)]
            (dest/('bgirl_'+clip.replace('-','_')+'.h' if clip else 'bgirl_dance.h')).write_text('/* SPDX-License-Identifier: GPL-3.0-only\n * Generated by tools/convert_dance.py */\n#define BG_DANCE_SIZE 96u\n#define BG_DANCE_COUNT 12u\n#define BG_DANCE_STRIDE 12u\nstatic const uint8_t bg_dance_masks[12][1152] = {\n'+''.join('{\n'+',\n'.join(rows[i*48:(i+1)*48])+'\n},\n' for i in range(12))+'};\n')
    if clip:
        header=dest/('bgirl_'+clip.replace('-','_')+'.h')
        prefix=clip.replace('-','_')
        header.write_text(header.read_text().replace('BG_DANCE_', 'BG_'+prefix.upper()+'_').replace('bg_dance_masks','bg_'+prefix+'_masks'))
        manifest.update(clip=clip, duration=probe.get('duration'), time_base=probe['time_base'], phrase_beats=4, seam='ping-pong 0..11..1; endpoints not duplicated' if ping_pong else 'hard cut last to first; no seamless-loop claim')
        if ping_pong: manifest.update(playback='ping-pong',playback_indices=list(range(12))+list(range(10,0,-1)))
    preview = Path(__file__).with_name('dance_preview.html').read_text().replace('@MASK_BASE64@',base64.b64encode((dest/'poses-96.bin').read_bytes()).decode())
    if clip:
        preview=Path(__file__).with_name('crew_preview.html').read_text().replace('@CLIP@',clip.upper()).replace('@PLAYBACK@','ping-pong' if ping_pong else 'forward').replace('@PLAYBACK_DESCRIPTION@','Forward then back, without repeated endpoints.' if ping_pong else 'Source-order playback with hard cut 11→0.').replace('@MASK_BASE64@',base64.b64encode((dest/'poses-96.bin').read_bytes()).decode())
    (dest/'preview.html').write_text(preview)
    (dest/'manifest.json').write_text(json.dumps(manifest,indent=2)+'\n')
    return manifest

if __name__=='__main__':
    p=argparse.ArgumentParser(description=__doc__);p.add_argument('source',type=Path);p.add_argument('dest',type=Path);p.add_argument('--indices',default=','.join(str(i) for i in range(0,96,8)));p.add_argument('--sha256',default=SOURCE_SHA)
    p.add_argument('--source-url',default=SOURCE_URL);p.add_argument('--rights');p.add_argument('--clip');p.add_argument('--sizes',default='64,96');a=p.parse_args();print(json.dumps(convert(a.source,a.dest,[int(i) for i in a.indices.split(',')],a.sha256,a.source_url,a.rights,a.clip,tuple(int(s) for s in a.sizes.split(','))),indent=2))

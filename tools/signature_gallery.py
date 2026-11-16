#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-only
"""Write a local-only listening gallery alongside rendered WAVs."""
import html,json,sys
from pathlib import Path
root=Path(sys.argv[1]);rows=json.loads((root/'measurements.json').read_text())
parts=['<!doctype html><meta charset="utf-8"><meta name="viewport" content="width=device-width"><title>Signature Electro listening</title>',
'<style>body{font:16px system-ui;max-width:950px;margin:30px auto;padding:16px;background:#f4f0e4;color:#161b18}audio{width:100%}article{border-top:1px solid;padding:14px 0}a{color:#194c36}</style>',
'<h1>BGIRL Signature Electro · listening review</h1><p>Musical approval pending Doug. These are original native firmware renders, not recordings from physical hardware.</p>',
'<p>Players use whole-file RMS-matched comparisons (−26 dBFS target, −6 dBFS peak ceiling). Original gain renders remain linked. Match comparisons by ear too: long sub notes and short transients have different crest factors.</p>',
'<p>Sound references: MIDI36 for subs, MIDI60 for others, velocity100. Subs hold8seconds then release; other gates125ms, drum note-offs do not truncate tails. Wet adds DLY14/REV12; default factory sounds are dry. Demo A: retained TIGHT + drive sub + iron stab + minor chord. B: kick + layered clap + closed hat + clean sub. C: closed/open group choke exercise. Each demo is four beats × four bars at90/120/140BPM.</p>',
'<p><a href="measurements.json">Measurements and hashes</a> · <a href="manifest.json">Versioned recipes</a> · <a href="context.json">Tested revision and render context</a> · <a href="../../index.html">Play the synth</a></p>']
for row in rows:
    name=html.escape(row['file']);parts+=['<article><h2>'+name.removesuffix('.wav')+'</h2>',
      '<audio controls preload="none" src="matched/'+name+'"></audio>',
      '<p><a href="'+name+'">Unnormalized original WAV</a> · peak '+f'{row["peak"]:.4f}'+
      ' · RMS '+f'{row["rms"]:.4f}'+' · comparison gain '+f'{row["comparison_gain_db"]:+.1f}'+'dB</p></article>']
(root/'index.html').write_text('\n'.join(parts)+'\n')

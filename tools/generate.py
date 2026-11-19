#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-only
"""Browser-only entry into upstream's generators; no device toolchain/package needed."""
import sys
from pathlib import Path
sys.path.insert(0, str(Path(__file__).resolve().parents[1] / 'felucca/tools'))
import build
build.generate()

import subprocess
for gen in ("vocoder_time.py", "vocoder_coeff.py", "carrier_bank.py"):
    subprocess.run([sys.executable, str(Path(__file__).resolve().parent/gen)], check=True)

import package_speech
package_speech.package()

import shutil
import os
import json
import package_vocals
vocal_dest = Path(__file__).resolve().parents[1] / 'felucca/build/gen'
vocal_bank = os.environ.get('BGIRL_VOCAL_BANK')
if vocal_bank:
    bank = Path(vocal_bank).resolve()
    metadata = json.loads((bank/'bank.json').read_text())
    if package_vocals.sha((bank/'bank.ima').read_bytes()) != metadata['audio_sha256']:
        raise ValueError('optional bank audio checksum mismatch')
    if package_vocals.sha((bank/'bgirl_vocals.h').read_bytes()) != metadata['header_sha256']:
        raise ValueError('optional bank header checksum mismatch')
    shutil.copyfile(bank/'bgirl_vocals.h', vocal_dest/'bgirl_vocals.h')
    print('EXPLICIT LOCAL VOCAL BANK:', metadata['bank_sha256'])
else:
    public = Path(__file__).resolve().parents[1] / 'assets/vocals/g6'
    package_vocals.package(public, public/'slices/manifest.json', vocal_dest, group_ids=[6])
listening = Path(__file__).resolve().parents[1] / 'assets/listening'
shutil.copytree(listening, Path(__file__).resolve().parents[1] / 'felucca/build/emu/listening', dirs_exist_ok=True)

import package_logo
package_logo.package()

# Interchange descriptors must match this engine/layout without regenerating by surprise.
subprocess.run([sys.executable, str(Path(__file__).resolve().parent/'song-snapshots/generate_layout.py'), '--check'], check=True)
subprocess.run(['node', str(Path(__file__).resolve().parent/'song-snapshots/schema.mjs'), '--check'], check=True)

#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-only
"""Append one carrier in a local wrapper; inherited source/pin is untouched."""
from pathlib import Path
root=Path(__file__).resolve().parents[1]
s=(root/'felucca/firmware/engines/src/eng_analog.c').read_text()
start=s.index('static const preset_t ANALOG_PRESETS[]')
array=s[start:s.index('\n};',start)+3]
array=array.replace('ANALOG_PRESETS','BG_ANALOG_PRESETS').replace('\n};','\n    {"BG CARR SAW", {0,0,0,0,127,0,0,0}, {0,0,127,20}, 0,1,FX(0,0,0,0),PAT(4)},\n};')
start=s.index('static const engine_t ENG_ANALOG =')
engine=s[start:s.index('\n};',start)+3].replace('ENG_ANALOG','BG_ANALOG').replace('ANALOG_PRESETS','BG_ANALOG_PRESETS')
(root/'felucca/firmware/src/bgirl_carrier.h').write_text('/* SPDX-License-Identifier: GPL-3.0-only; generated local wrapper around MIT engine presets. */\n'+array+'\n'+engine+'\n')

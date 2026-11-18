"""SH-01A MIDI chart v1.00 (2017-08-08), p1; manual eng04 p1.

Chart gives CC names/directions, NOT selector bins or physical response laws.
All raw values remain authoritative; deliberately no guessed selector decoding.
"""
VERSION = 'roland-chart-1.00/raw-v1'
MIDI_URL = 'https://static.roland.com/assets/media/pdf/SH-01A_MIDI_Imple_Chart_eng01_W.pdf'
MANUAL_URL = 'https://static.roland.com/assets/media/pdf/SH-01A_eng04_W.pdf'
_NAMES = {
 1: ('performance.modulation','MODULATION'),
 3: ('lfo.rate','LFO RATE'), 5: ('portamento.time','PORTAMENTO TIME'),
 11: ('performance.expression','EXPRESSION PEDAL'),
 12: ('lfo.waveform','LFO WAVE FORM'),13: ('vco.mod_depth','VCO MOD DEPTH'),
 14: ('vco.range','VCO RANGE'),15: ('vco.pulse_width','VCO PULSE WIDTH'),
 16: ('vco.pwm_source','VCO PWM SOURCE'),17: ('vco.mod_sensitivity','VCO MOD SENS'),
 18: ('vco.bend_depth','VCO BEND DEPTH'),19: ('mixer.pulse_level','VCO PWM LEVEL'),
 20: ('mixer.saw_level','VCO SAW LEVEL'),21: ('mixer.sub_level','VCO SUB LEVEL'),
 22: ('vco.sub_type','VCO SUB TYPE'),23: ('mixer.noise_level','VCO NOISE LEVEL'),
 24: ('vcf.envelope_depth','VCF ENV DEPTH'),25: ('vcf.mod_depth','VCF MOD DEPTH'),
 26: ('vcf.key_follow','VCF KEY FOLLOW'),27: ('vcf.bend_depth','VCF BEND DEPTH'),
 28: ('vca.envelope_switch','VCA ENV SW'),29: ('envelope.mode','VCA ENV MODE'),
 30: ('envelope.sustain','ENV SUSTAIN'),31: ('portamento.mode','PORTAMENT MODE'),
 64: ('performance.hold','HOLD'),65: ('performance.portamento','PORTAMENTO'),
 71: ('vcf.resonance','VCF RESONANCE'),72: ('envelope.release','ENV RELEASE'),
 73: ('envelope.attack','ENV ATTACK'),74: ('vcf.cutoff','VCF CUTOFF'),
 75: ('envelope.decay','ENV DECAY'),76: ('system.tune','TUNE'),
 77: ('system.transpose_switch','TRANSPOSE SW'),78: ('noise.mode','NOISE MODE'),
 79: ('lfo.mode','LFO MODE'),80: ('voice.assign_mode','ASSIGN MODE'),
 81: ('chord.voice2.enabled','CHORD VOICE 2 SW'),
 82: ('chord.voice3.enabled','CHORD VOICE 3 SW'),
 83: ('chord.voice4.enabled','CHORD VOICE 4 SW'),
 85: ('chord.voice2.key_shift','CHORD VOICE 2 KEY SHIFT'),
 86: ('chord.voice3.key_shift','CHORD VOICE 3 KEY SHIFT'),
 87: ('chord.voice4.key_shift','CHORD VOICE 4 KEY SHIFT'),
}
CONTROLS = {cc: dict(parameter=pid, label=label,
 category=pid.split('.')[0] if pid.startswith(('performance.','system.')) else 'patch',
 transmitted=cc not in (11,64,65), recognized=True,
 source=dict(url=MIDI_URL,version='1.00',date='2017-08-08',page=1),
 decoding='raw-only; physical law and selector bins unverified')
 for cc,(pid,label) in _NAMES.items()}
PATCH_IDS = sorted(v['parameter'] for v in CONTROLS.values() if v['category']=='patch' and v['transmitted'])

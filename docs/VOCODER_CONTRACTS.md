# BGIRL vocoder v1 bounded contracts

Implementation base: planning PR #5, `7aa442c281293698b3f8b5c0fa47286a6105d20d`.
Pins remain Felucca v1.8.1 / engines d0260074 / BSP 5942e68b.
Written before implementation; host/browser milestone, not target feasibility.

- One instance: lowest numbered SLICE track with CARR != OFF wins. Other requests
  remain visible but inactive. No self-route; invalid/unsupported/muted source yields
  zero carrier and no consonant leakage. An ineligible owner yields no instance.
- Eligible carrier: ANALOG, PHASE, LOFI, VOICE, TRIO, WHEEL, GRAIN, PHYS, NOISE,
  FM6, SAMPLE, DRUM, SLICE (not self). Any valid enabled engine. Stereo SPREAD is
  deliberately disabled on the two participating tracks while routing is active:
  their mono shaped signal is the explicit tap contract, not an undocumented sum.
- Carrier tap follows DIST, INSERT and SLICER, before level/pan/sends. MONITOR OFF
  suppresses all direct carrier dry/sends/throw, not rendering or its feed.
- Ordinary P_MUTE prevents new notes (inherited) and silences the selected feed;
  performance mute also silences feed. Speech mute silences processed output.
  No new solo system: inherited track-mute combinations retain these semantics.
- Bundled SPEECH is append-only SLICE source 5. Four phrases at C4..D#4, wrapping
  chromatically across octaves with START/ROOT. AUTO phrase boundaries irrespective
  of DIV. ONE mode, forward, original pitch (PTCH and pitch modulation ignored),
  no DCAY attenuation. TONE/ADSR remain available. One voice, newest trigger wins,
  no note-off fallback to earlier held phrases. A 3ms transition from the previous
  output sample to a newly faded-in phrase bounds retrigger discontinuity.
- Speech uses existing sequencer/SLICE decoder. No network at build/runtime.
- Routing/source/engine/band changes reset filter/envelope/Freeze state at block
  boundaries and clear capture validity. Missing sources never reuse old captures.
  Disable returns to ordinary dry track processing. Transport STOP and panic end
  bundled speech and reset live Freeze; ordinary downstream effect tails decay.
- Freeze latches envelopes, even zero before any speech; analysis keeps running.
  Unfreeze slews to current envelopes. No held audio loops. Carrier remains playable.
- Three normal four-knob FX pages: routing (CARR OFF/T1..T4, BANDS 8/12, WET
  0..127 default127, MONITOR OFF/ON defaultON), input (HOLD live OFF/ON,
  speech/carrier trim 0..127 default64 = unity, consonants 0..127 default0),
  character (LOW/MID/HIGH 0..127 default64 = unity, RELEASE 1..127 default32).
  Release is saved Freeze configuration; HOLD is transient and loads OFF.
- Parameters added before P_E0 use inherited count-mapped project/preset layouts.
  Old saves default route OFF. Sound presets never restore project-specific CARR
  references or live HOLD. Projects restore routing and bank SRC, never latched state.
- Two int32[CTL] captures, one writer each, published by existing join. Snapshot
  routing before fork; defer only speech's post-DIST processing, preserving its
  mod_begin values until post-join mix/insert/slicer/sends/mod_end. No double render.
- Submit rejected before execution runs locally. Failed join is assumed to return
  only after worker is stopped (BSP contract). Discard all worker sums/captures;
  reset affected tracks before the next block, never replay partially advanced work.
  This is silence/recovery, not seamless fault tolerance. Target cache/hold behavior
  remains hardware-unverified. No BSP pin changes.
- DSP: float biquad pairs, 8/12 logarithmic bands 180..6500Hz; rectified envelopes,
  bounded gains/output; coefficients prepared outside per-sample loops. Target cost
  unknown. Band default provisional12; human intelligibility approval outstanding.
- Compile-time BGIRL_VOCODER=0 retains old DSP; source bank remains selectable.
  Route OFF preserves ordinary audio/goldens. No render allocation/I/O/logging.

Asset rights: original text/described synthetic voice, no real-person reference.
Model Apache-2.0 metadata is not a blanket rights warranty. Preserve source
provenance; custom-node license caveat and stochastic generation are documented.

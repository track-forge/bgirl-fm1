# BGIRL vocoder v1 bounded contracts

Implementation base: planning PR #5, `7aa442c281293698b3f8b5c0fa47286a6105d20d`.
Pins remain Felucca v1.8.1 / engines d0260074 / BSP 5942e68b.
Written before implementation; host/browser milestone, not target feasibility.

- Routed SLICE sources also use one voice/newest trigger; ordinary bank-off SLICE
  behavior stays inherited. ONE note-off never enters release, including a tap
  before the first render and an articulation queued during an engine transition.
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

## Storage fit correction (found during first save regression)

FUN10 has only one spare byte after its 3-byte MIDI record; twelve count-mapped
parameters cannot fit. FUN11 (`FUNB`) therefore keeps the same 3840-byte object,
addresses, step/chain/motion/FM6 formats, but omits the twenty retired DIGITAL
operator fields (IDs61..80). Their defaults are restored. They have no function
in this pinned build (`FELUCCA_FM4=0`); DIGITAL old saves still import/convert before
writing. New saves cannot represent active DIGITAL/fm4 and explicitly refuse it.
All FUN1..FUNA readers remain. Old firmware rejects FUNB by magic, not misinterprets
it. Runtime parameter count126 remains <128; compact wire count106 fits with33
spare bytes before MIDI. Preset records have capacity and need no format change.

Source switching uses normal SLICE lifetime: an already sounding slice retains its
captured source/bounds until retrigger/end; routing/filter/Freeze state resets at
the next block. A former carrier resumes ordinary direct monitoring when it is
no longer selected. Bundled SPEECH is factory/read-only on SLICES; import a user
slot to edit slice boundaries. Two participating tracks use mono shaping even if
their saved SPREAD is nonzero; unrelated tracks retain SPREAD bit-identically.

The failed-worker path resets voices/insert state and drops that block's sums;
LFO/other engine state is not rolled back. It is bounded recovery with possible
silence/discontinuity, not transaction rollback or fault-transparent rendering.


## Listening follow-up: bell carrier / front-loaded pages

Doug reports the overall experiment is a win and expects the vocoder to work.
At his request, FX now cycles FX → VOCODER → VOC INPUT → VOC COLOR → SLICER →
INSERT → INSERT2 → DLY → REVERB → CHORUS. No parameter IDs changed. Wet preview
buttons preserve the electro demo's original T1 drums, T2 bass and T3 FM6 BELL
patches/patterns, replacing only T4 VOX with speech and choosing CARR=T3, MON=ON.
Dry speech remains isolated. Diagnostic tests and existing listening references
retain the old isolated ANALOG setup; they are labeled separately from the live
bell groove. Bell decay/sparse notes naturally provide a different, less sustained
carrier; no patch brightening or envelope changes are silently added.

## Listening follow-up: sustained pad carrier

After the bell audition sounded like vocal-gated bells, Doug requested a richer
pad instead. Live wet8/12 setups now replace T3 with ANALOG dual saws, 10-cent
detune / equal mix, open low-pass / zero resonance, no noise/drive/filter envelope,
fast attack, full sustain and release40. C3/G3 alternate every16 steps across64,
with15 inherited TIE steps per note to maintain the feed through speech. T1/T2
remain electro drums/bass. CARR=T3, MON=OFF by default to avoid dry-pad masking;
MON ON remains available. T4 speech, DSP, startup electro, saves and existing
isolated reference assets are unchanged. Human pad intelligibility still pending.

## Main-feature demo promotion (2026-11-15)

Doug accepted the pad-carrier audition and merged the implementation stack.
The default electro demo now features that12-band T3-pad→T4-speech arrangement,
with dry carrier MON OFF. Shared initialization owns the arrangement for both
fresh startup and explicit browser demo load, not separate copies. It runs only
before autosave restore or an explicit reset, never over a restored session.
Blank electro template retains its prior palette with empty steps/no routing.
BGIRL_VOCODER=0 retains the original bell/synth demo. Legacy migration TRK_DEF,
saves, speech assets and DSP stay unchanged. Separate vocal-overlay issue #8
tracks dry speech alongside vocoded output, independent of carrier MON.

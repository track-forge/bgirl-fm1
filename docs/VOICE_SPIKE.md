> **Historical prototype only.** Source/assets/tests and old relative paths refer to
> checkpoint `4cc1e11`, not the active Felucca edition. See [current status](BUILD_STATUS.md)
> and [provenance](UPSTREAM.md). Vocoder integration is deferred.

# Playable voice feasibility spike — host/browser experiment

2026-11-15. **Implemented experimental live carrier-controlled vocoding;
intelligibility/listening approval and hardware feasibility NOT established.**
Not pre-rendered robot playback. The dry modulator is pre-generated speech;
analysis, carrier pitch and resynthesis run live. No automatic voice reply in
the groove until a human listening pass approves it.

## Method / decision

12 logarithmically spaced constant-peak band-pass pairs, Q=3, 180–6500 Hz;
rectified envelopes with approximately 0.9 ms attack / 15 ms release;
sawtooth mono carrier. Same dry phrase for 110, 164.8138 and 220 Hz renders.
20 ms output RMS ranges from silence to 0.077/0.107/0.109 at the three pitches
(with WAV scaling). Autocorrelation estimates 109.98/164.55/220.50 Hz versus
110/164.81/220 Hz requested (correlations 0.976/0.984/0.989). These are pitch and
time-envelope checks, **not proof someone understood the words**.
Initial gain 12 then 3 exceeded unity (measured peak 1.2843 at gain 3); gain 1.5
reduces that peak to 0.6422 before conservative mix gain/limiting.

12 bands is an inexpensive host starting point, not an FM-1 budget decision.
Next experiment: Doug listens blind to dry and each processed pitch; record
which words survive, then compare 16 bands or add high-passed unvoiced energy
if consonants disappear. Do not replace this with a rendered hook silently.

## Reproduce / compare

```sh
./tests/render_voice.sh
# dry speech: app/assets/phrase.wav
# live DSP renders: app/build/voice/vocoder-{110,164.8138,220}.wav
```

Standalone renders contain 8 seconds (phrase plus silence) and use 24000/32768
output scaling. Host/browser keys in VOICE mode restart the phrase on each new
articulation; legato changes carrier pitch without restarting. Note-off releases;
stop and panic silence it. No sample-rate-dependent speech pitch shifting.
See assets/README.md for source/rights/conversion. No network during build.

## Resource evidence (host only)

- PCM: 125990 read-only bytes, 62995 samples / 2.857 seconds.
- `sizeof(vocoder)`: 828 bytes on this host; no heap, no decode buffer.
- 12 analysis + 12 synthesis biquads per voiced frame, bounded loops.
- Standalone host render + stdio of 8 seconds: roughly 10–23 ms in the initial
  measurements (only 2.857 seconds of active speech). Not a target deadline test.
- Exact final sizes/timing and browser evidence: BUILD_STATUS.md.
- Nominal device block: 256/44100 = 5.805 ms. No device build, linker RAM/map,
  XIP/storage audit, xrun measurement or physical test. Target fit is unknown.

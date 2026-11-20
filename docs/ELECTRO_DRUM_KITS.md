# BGIRL electro drum kits

In **DRUM**, select a factory sound with PRESETS, or change only **EDIT → KIT**
to keep the current knobs, mix and pattern. All three retain the same key/MIDI
map and grid: kick 36, snare 38, clap 39, closed hat 42, open hat 46,
tom/conga 45, rim 37, bell/zap 56. Closed hats choke open hats on the same track.

- **BG BREAK** — short, swept punch; woody offbeat rim; the bright clap sits over a lower, dry snare. Listen to step 5 (snare), 10 (clap) and 13 (the stack).
- **808 RAW** — rounded low kick with a longer ring, tuned shell, wider clap bursts, sizzling hats and a two-square bell. TOM plays a hollow conga without moving the lane.
- **COLD WIRE** — hard knock, inharmonic shell, clipped clap and thin hats. The two-operator tom answers the bass; accents increase the phase-modulation index on the glassy bell rather than just turning it up.

Fresh power-on and Blank Project select BREAK on T1. Fresh power-on retains the
editable startup groove and split kick; the explicitly loaded **Mega Jam** still
uses T1 TIGHT and T6 BG KICK. Existing autosave, named project, user preset,
and v1 portable song loads take precedence, just as before. New kit IDs
are 19–21, factory presets 9–11 (zero based), and MIDI Program Change values
16–18 (zero based). Old IDs are unchanged.

## Listen: one beat, six clips

| KIT | Drums solo | Same beat with Crunchy Bass |
| --- | --- | --- |
| BG BREAK | [Solo](audio/electro-kits/bg-break-solo.mp3?raw=true) | [With bass](audio/electro-kits/bg-break-bass.mp3?raw=true) |
| 808 RAW | [Solo](audio/electro-kits/808-raw-solo.mp3?raw=true) | [With bass](audio/electro-kits/808-raw-bass.mp3?raw=true) |
| COLD WIRE | [Solo](audio/electro-kits/cold-wire-solo.mp3?raw=true) | [With bass](audio/electro-kits/cold-wire-bass.mp3?raw=true) |

120 BPM, two bars, dry, 44.1 kHz. The normal firmware sequencer uses the
same 32 steps, velocities and accents, with the same mix controls and (where
present) bass notes. One constant gain per finished clip matches full-phrase
RMS to −24 dBFS before MP3 encoding. No EQ, compression or limiting. This
matches playback levels, not individual drum hits or LUFS.

Reproduce after the web build with `./tools/render_electro_kits.sh` (requires
FFmpeg with libmp3lame). The [fixture](../tests/electro_kits.c) is the complete
beat and bassline. The script also renders Felucca 80 and TIGHT reference takes
to ignored `tmp/electro-kits/`.

## Reference and implementation

Felucca's existing fixed-point kit voices are the starting point (Leo Kuroshita /
Hügelton Instruments, GPL-3.0-only). The reference comparison rendered the same
hit mask through Felucca **80**, **TIGHT**, and the standalone modeled **808**
in [fm1-x0x at 512c0da](https://github.com/charlesvestal/fm1-x0x/blob/512c0daaf25f82c5b70e9eceee195864e7da378c/firmware/src/dsp/drum808.c).
The x0x comparison used its direct trigger/dry-bus path and native velocity law,
not the BGIRL master or its accent response. In the opening kick/hat window
(79–120 ms versus 0–50 ms), relative RMS fell 15.3 dB in TIGHT, 5.6 dB in
80, 6.2 dB in x0x, and 2.2 dB in RAW. RAW deliberately extends the sub
beyond the circuit reference; its sharper distinctions are not just decay.

RAW adapts the x0x/8W8 clap's approx. 875 Hz band, Q and 10 ms teeth spacing
(`cp_trigger`), and the metal oscillator frequencies (`k_bank_inc`, converted
to Felucca's pitch-relative Q12 ratios). It retains Felucca's sine kick, snare,
rim and bell runs and its envelope/filter/choke architecture rather than
importing x0x's state, circuit solvers, PolyBLEP, Optimist code or samples.
Source credits and GPL notices: [fm1-x0x LICENSING](https://github.com/charlesvestal/fm1-x0x/blob/512c0daaf25f82c5b70e9eceee195864e7da378c/LICENSING.md);
Charles Vestal (fm1-x0x, GPL-3.0-only) and athousanddetails/Gustavo Lima
([8W8](https://github.com/athousanddetails/schwung-8W8), GPL-3.0).
[Hortator's FM percussion](https://github.com/DeadActive/hortator/blob/44c9ad4aa57bebf63f412bd552e57143c30b4065/firmware/src/dm_fm.c)
is a design reference only; no model code, coefficients or sequencer were copied.

## Verification

- Native focused run: all 24 MIDI lane hits audible and released; per-kit preset
  and autosave restore passed; same-track open/closed choke passed. The six
  takes reached at most six active voices with no steals.
- Dense comparison: 240 full eight-lane velocity-127 stacks per kit, at maximum
  ACC. New kits peaked at 0.328–0.377, versus 0.334 TIGHT and 0.340 80; no steals,
  stuck voices or nonfinite output. Whole host render cost was 8.06–8.78 ms per
  audio second, versus 8.06 TIGHT / 8.17 80 on that run. Not a device benchmark.
- Headless Chromium/AudioWorklet: real live keys and MIDI hits, same-pattern
  Program Changes through all three kits, transport playback, autosave refresh
  and blank default passed; zero page errors. Portable song browser suite
  also passed, including download/upload, refresh and saved-slot isolation.
- No device build or flash. These are rendered and measured takes; final
  voicing and balance are for human listening.

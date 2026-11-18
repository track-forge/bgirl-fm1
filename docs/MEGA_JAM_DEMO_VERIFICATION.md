# Mega Jam default — mapping and verification

2026-11-17 · existing draft [PR28](https://github.com/track-forge/bgirl-fm1/pull/28).
Base `c51c53885fd18c1e84be676d1644143803bc2e0b`; plan commit `4c78c894`.
Implementation/preview exact head and final checks are recorded in the PR receipt.
Technical validation is not Doug's listening approval. Hardware remains untested.

## Arrangement and deliberate translation

Normal six-track sequencer,64 sixteenth-note steps,128BPM:7.5s/four bars.
T6 is deliberately empty for jamming. The last bar's softer rim-only ending,
four-bar pad contour and whole-hook/answer shape make this more than four
copies of one bar. No new engine, voice pool, wire format, sample, DSP or effect.

- **T1 DRUM/TIGHT KIT:** all11 reference kick onsets, four snare+clap backbeats,
 14 closed hats, three open hats and three rims per two bars, repeated into
 four bars except final step63 is a soft rim-only breath. Snare and clap are
 intentionally separate layers, not a combined-body sound double-layered.
 GM49 cymbals at beats4.5/5.5 deliberately omitted, not remapped to TOM/BELL.
 Drum grid gate lengths cannot reproduce individual open/cymbal MIDI durations;
 existing one-shot kit decays/choke apply. Step velocity104, kick/backbeat
 accent127 where source kick120 or snare/clap120; source kick100/104 and rim100
 approximate shared104, final rim velocities88/78. Thus shared hat/backbeat
 loudness is a deliberate approximation, not per-note-velocity import.
 Signature drum presets are **single-identity** instruments (every key maps to
 one piece), so they cannot form this multi-lane kit without splitting tracks
 or changing the engine. Keep TIGHT KIT and preserve the BG Signature bank.
- **T2 ANALOG/SQR BASS:**16 mono events per two bars; A/G/F pitch classes,
 source velocities and rhythm preserved, transposed **−24 semitones** per Doug's
 2026-11-18 listening request (MIDI21/29/31/33). Repeat at step32. Simultaneous55 at beat4.5
 (0.012798beat) and45 at5.75 (0.008936beat) omitted in favor of longer57/55.
 Global GATE100/128 gives0.1953125beat; source0.25 and short gates are quantized
 to this common value. SGATE is not motion-lockable, so no unsupported locks
 or independent timing engine. Mono, short release20, level79; no claim of
 recreating the unknown external SH-01A patch or external tuning.
- **T3 TRIO/FAT BASS:**factory preset unchanged, mono, level90, A2 then E3,
 each tied for two bars. MON OFF, normal T3→T4 carrier route. Oscillator settings
 remain factory; this is a compositional carrier line, not the old C2/G2 loop.
- **T4 SLICE/VOCALS:**find public G6 by view label. WHOLE one-shots at
 steps0/32 only (beat1 of bars1/3), using the matching parent. No chops or VIEW
 automation, per Doug's 2026-11-18 listening revision. SLC_ONE means note-off
 does not gate the phrase. WET117/127 (92%),24bands, LOG REL nearest120ms;
 MON OFF, consonants0. Whole phrase is1.918s, with7.5s/four bars. No new speech
 or bank rebuild. Bundled public G6 fingerprint/provenance unchanged; see
 `assets/vocals/` and existing VOCAL_BANK documentation. A custom bank lacking
 G6 intentionally leaves the vocal pattern silent instead of substituting an
 unrelated phrase. The parent/view relationship comes from generated bank metadata,
 not a full-bank hard-coded G6 index.
- **T5 GRAIN/CLOUD PAD:**single mono voice, level55, reverb43/127, release35.
 Raw50/60/64 (D3/C4/E4) at steps0/28/33; ties through61, breath62/63.
 Source onsets0/6.951958/8.203610beats quantize to0/7/8.25, mono transitions
 collapse slight source overlaps. Final gate reaches15.5beats vs15.482824.
 Low Ableton rack velocities34/33/31 become72/70/68 at reduced track level;
 no claim of ear-matched loudness or recreating rack macro percentages.
 Existing bundled GRAIN sources/effects suggest texture, not a rack/patch dump.

All engine/preset IDs and frozen `project_defaults.h` remain unchanged. The old
blank palette stays blank. Standalone speech8/12, vocal, Signature, six-track
and clarity auditions retain the prior setup via a clearly named legacy helper;
they do not inherit the new pad, tempo or VIEW automation. Autosave restores
after initialization and wins. Explicit replacement still confirms/cancels and
preserves saved slots. Dormant FM6 patches are initialized on demo replacement
so fresh and explicit project captures are exactly identical, including hidden
saved state; this is not a migration of existing user projects.

## Reproduction and evidence

```
source /home/dev/.local/share/bgirl-emsdk/emsdk_env.sh
./build.sh --web
./tests/run_tests.sh
python3 -m http.server 8796 --bind 127.0.0.1 --directory felucca/build/emu
```

Browser environment:
`PLAYWRIGHT_MODULE=/tmp/moltazine-351-deps/node_modules/playwright`,
`CHROMIUM_PATH=/home/dev/.cache/ms-playwright/chromium-1228/chrome-linux64/chrome`,
`BGIRL_URL=http://127.0.0.1:8796` (or exact deployed HTTPS8448).
Run `node tests/mega-browser.cjs` plus existing affected browser suites.
Logs `/tmp/bgirl-mega-{baseline-build,baseline-tests,web,tests}.log`;
ignored screenshots/results under `tmp/mega-browser/`.

Baseline root browser build and full host suite PASS at plan-only head.
New focused native test: fresh/explicit full-project equality; exact packed
roundtrip;1447/3840bytes,2393bytes margin;30s/four loops, max8 admitted voices,
zero steals, peak0.23104858 and RMS0.031035784 at test master; nonzero vocoder
modulation input. STOP/restart release and forced-overload/panic recovery,
manual edited project save/load, autosave priority and blank preservation are
asserted. Measurements are host observations, not subjective audio approval.

Full root browser build and updated complete host suite PASS. All109 existing
audio goldens unchanged (0changed,0gone);88200 inherited heavy-song native/WASM
frames and176400 clarity12/24 frames bit-identical. Serial/threaded8/12/24
worker comparisons and failure recovery, six-track codec/editor/storage,
vocoder-disabled and dance-disabled preservation all pass. No engine/BSP pins,
frozen defaults or golden files changed. Additional browser/rollout receipts
are recorded below and on the PR.

## Audition / rollback

Refresh https://openclaw-fedora.tail549b77.ts.net:8448/ → **Switch on** →
**Load electro demo** → confirm replacement → **PLAY**. Start MASTER low.
An old autosave intentionally wins until explicitly loading the new demo.
Listen at least two loops: groove/riff identity, full G6 hook articulation,
pad balance/change and final-bar breath. Use ALGORITHM T1–T6 to edit; T6 is free.
STOP, save with normal PROJECT controls. Band/release/wet/carrier preference
remains Doug's listening decision, separate from technical tests.

Deployment uses an immutable exact-head artifact and existing loopback8786
service. Prior artifact and effective service configuration are retained;
rollback directory `local/mega/rollback/` contains `clarity-preview.conf` and
before-service/build/Serve receipts. Restore that single runtime drop-in,
`systemctl --user daemon-reload`, restart `bgirl-signature-local.service`, verify
old `build.json` and WASM. No Serve/Funnel settings should change.
No merge/public deployment/Ableton change/device build/flash performed.

### Additional observed checks

- New actual Chromium Mega Jam suite PASS: gesture audio, two loops, bounded
  signal (peak0.216896), STOP/restart, confirm/cancel, six distinct track screens,
  normal pad sound/mixer editing, manual project save, autosave reload preserving
  edited pad level and flash, desktop/mobile screenshots, no overflow/page errors.
  Both screenshots visually inspected. `tests/mega-browser.cjs` joins hosted CI.
- New native/WASM test:661500frames (two complete loops) **bit-identical**.
  This is arrangement evidence, not a replacement for any sound golden.
- AddressSanitizer new Mega test PASS including project codec and overload/panic;
  leak detection disabled for static firmware harness, no target-memory claim.
- Old assertions comparing wet8/12 lab backing to the fresh default now compare
  those auditions to their retained legacy backing; their concrete carrier,
 32-step/chop and8/12 routing assertions are unchanged. New default is covered
  separately, including strict full-project fresh/explicit equality.

Optional local listening file `local/mega/mega-jam-two-loops.wav` is the15s
native parity capture converted from `felucca/build/host/mega-native.f32` to
stereo44.1kHz PCM16 without normalization. The preview exposes a copy under
`listening/mega-jam.wav`; it is generated review evidence, not a source sample.
The existing historical clarity gallery is copied unchanged into the frozen
preview and retains its old render revision/labels (including archived GRAIN).

The local clarity browser run initially reached its final gallery check and
returned404 because root builds do not bundle the optional archived gallery.
Copied the existing preview's30 gallery files byte-for-byte (all hashes matched),
without regeneration or relabeling, then reran the suite against the complete
preview fixture. This gallery is not made a source/build dependency.

Final local actual Chromium result: **10/10 suites PASS** — Mega, splash,
smoke, vocoder, demo, vocal, dance, six-track, Signature and clarity. Zero
reported page errors. Clarity preserves all five signal loaders, normal8/12/24
selection,5/500ms controls, Freeze/STOP, persistence and mobile behavior with
the unchanged archived gallery fixture. Final HTTPS rollout checks and hosted
CI are independently recorded in the PR handoff at the exact deployed head.


### Listening revision — SQR BASS down two octaves (2026-11-18)

Doug found T2 too beepy and requested two octaves, not one. Only demo T2 note
pitches change:21/29/31/33 instead of45/53/55/57. Rhythm, velocities, sound
parameters and all other tracks (including the TRIO carrier) are unchanged.
Existing autosaves are not rewritten; explicitly Load electro demo after refresh.
Original measurement/CI/deployment receipts above describe the first version;
updated exact-head verification and preview are recorded in the PR follow-up.

### Listening revision — whole vocal one-shots (2026-11-18)

Doug also requested the vocal track play only the whole sample at beat1 of
bars1/3. T4 now has exactly two SLC_ONE notes at steps0/32, WHOLE view throughout,
no chopped answers or VIEW automation. Carrier, vocoder settings and all other
tracks are unchanged. This is combined with the T2−24-semitone revision above.


### Listening revision — saw carrier and vocal FX gate (2026-11-18)

Supersedes the initial T3 TRIO recipe above, only for the default Mega Jam.
T3 now uses the existing ANALOG / BG CARR SAW preset12, mono and MON OFF.
A0(MIDI21) and the lower G0(MIDI19) each last four beats/16 sixteenth steps,
repeating at steps0/16/32/48. This interprets two octaves down relative to A2/G2.
T4 retains WHOLE G6 / SLC_ONE on steps0/32, but its normal FX SLICER is now
GATE, displayed pattern3, existing rate1(1/16) and full depth. Normal FX page1
DLY is25/127, displayed20%. Gate is an output effect, not chopped source notes.
Vocoder24bands/LOG~120ms/wet117, lowered T2 riff, drums, pad and empty T6 remain
unchanged. Standalone vocoder/clarity labs retain their existing TRIO recipes.
Existing autosaves still win; refresh and explicitly Load electro demo to audition.

Regression coverage checks the complete fresh/explicit project match, exact
carrier schedule/ties, gate parameters and displayed delay, codec equality, and
edited gate/delay persistence through manual save and autosave. Native/WASM
comparison renders the actual scheduled arrangement for two complete loops;
voice-budget/STOP and browser checks remain required. Exact-head results and
private-preview receipts are recorded in the PR follow-up, not listening approval.


### Listening revision — carrier one octave higher (2026-11-18)

The saw carrier audition above was too low. Only T3 pitches rise12semitones:
A1(MIDI33), then lower G1(MIDI31), four beats each, repeated. All other demo
parameters/tracks remain unchanged, including T2−24semitones and T4 FX gate3
plus20% delay. Per Doug, a focused local build/arrangement/parity check precedes
preview rollout; full hosted CI runs asynchronously, not as a deployment gate.


### Listening revision — separate BG KICK, wet66%, A2/G1 (2026-11-18)

T6 is no longer empty: existing DRUM / BG KICK preset4 plays T1's former kick
grid with the same onsets, shared velocity104 and accent bits, at level92.
The final step63 kick omission remains. T1 has no kick notes/hits; its snare,
clap, hats and rim remain unchanged. BG KICK's preset timbre is unmodified.
T4 vocoder wet is84/127 (display66%); WHOLE one-shots, GATE3 and delay20%
remain. T3 carrier is now A2(MIDI45) then lower G1(MIDI31), four beats each
repeated. The G stays in the octave below A, not G2. T2/pad and labs unchanged.
Focused local arrangement/audio/persistence/parity and live preview checks
precede the quick rollout; hosted full CI is asynchronous per Doug.


### Listening revision — FM6 PAD down an octave with gate (2026-11-18)

Only T5 changes: existing FM6 / PAD preset4 replaces CLOUD PAD, still mono
at level55 with the existing reverb43/release35/gate127 demo settings.
Notes38/48/52(D2/C3/E3) are12semitones below the prior50/60/64, equivalent
in pitch to one OCT-; sequence onsets0/28/33, ties and velocities72/70/68
remain unchanged. OCT- is not globally changed (that UI control affects the
live keyboard, not stored sequence notes). Normal FX page6 SLICER: GATE,
PAT1, rate1(1/16), depth95/127(display75%). Dormant FM6 state is canonicalized
and the active T5 factory PAD slot is restored immediately. All other tracks,
including separate BG KICK, A2/G1 carrier and T4 wet66/gate3/delay20, remain.
Focused checks and live quick preview precede rollout; full CI asynchronous.

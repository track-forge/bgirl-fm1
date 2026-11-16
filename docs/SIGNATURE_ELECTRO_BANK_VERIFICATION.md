# Signature Electro — verification and listening handoff

Musical approval: **pending Doug for every family**. No approval inferred from
signals, waveforms, deterministic tests or this implementation.

## Exact revision and baseline

Runtime/test source: `5e2d699a7021ab20b25bd13610b5f22c37a4f823`.
Final documentation-only evidence commit follows this revision; no runtime edits.
Baseline: planning head `8e3ad05c79842318cb9c1769aa0078829a8cf361`, main
`5b2587014376e26d64d97b7082d67be65fdabef3`, no main drift at implementation start.
Baseline `./build.sh --web` and complete `./tests/run_tests.sh`: PASS before edits.
Logs: `/tmp/bgirl-signature-baseline-{build,tests}.log`.

Pins unchanged: engine `d0260074ce100a47675814b7f38d61f11176c926`, BSP
`5942e68b89ab9ccd23f2c541d712ca38fe635f11`. Emscripten6.0.10
(`~/.local/share/bgirl-emsdk`), GCC15.3.1, Node24.22.1. No target toolchain.

## Technical acceptance

Final web build and complete root suite: **PASS (exit0)**. All three actual
Chromium commands above passed (exit0): Signature controls/selection, general
browser regression, and bundled G6 startup/explicit vocal demo. The combined
108goldens had0changed/0gone,0health/voice/routing failures;88,200native/WASM frames
were bit-identical. New Signature focused and28threaded comparisons passed.

| Gate | Evidence |
| --- | --- |
| Baseline | Full build/root suite passed before implementation |
| Ten identities | Normal factory selection, SIGNATURE category, actual browser sound loading/audio |
| Standalone drums | Every MIDI0..127 × five identities; same fixed identity; velocity32/64/96/127; short note-offs, rolls, tuning/decay extremes |
| Input paths | Real queued USB MIDI low/middle/high notes, panel keyboard, sequencer; switched-track note-off ownership retained |
| Clap | One note starts clap+snare states, one shared allocation; balance endpoints both audible |
| Choke | Earlier open tail, both event orders and track orders, same control block closed wins, FREE isolation, asymmetric open, panic |
| Threading |28 bit-identical serial/fork-join grouped-hat comparisons; no cross-track renderer writes |
| Bass |8second held notes at every MIDI24..52; rapid/overlapping mono retriggers; release ends; no space/detune/pitch EG |
| Character |Every MIDI48..72 rendered/released; live chord exactly4tones, stacking capped6; actual chirp pitch envelope monotonic9st→root in139.320ms |
| Persistence |Ten edited project/user sounds, edited OWN patches, FUND+vocal fingerprints; autosave boot precedence; templates preserve user slot; legacy readers/fixtures retained |
| Robustness |Dry/wet29 reference/context renders finite/bounded; budget overload exercises stealing without stuck voices; STOP/panic and replacement cleanup |
| Browser |Actual Chromium all10 sounds, normal PRESETS, stopped template/demos, cancel/confirm, save/reload, vocal demo coexistence, desktop/mobile, zero page errors |

Root suite combines inherited98 goldens with **ten separate new hashes** in
`tests/signature-golden.txt`; inherited `felucca/tests/golden.txt` is unchanged.
Upstream assertions were updated only for appended category/version/program
ranges and an explicitly legacy KIT fixture (its synthetic KIT118 is no longer
an ordinary-kit value). No golden rewriting hides an inherited sound change.
The FM6 PC regression specifically caught and fixed bank-slot displacement.

Reproduce:

```sh
source ~/.local/share/bgirl-emsdk/emsdk_env.sh
./build.sh --web
./tests/run_tests.sh
export PLAYWRIGHT_MODULE=/home/dev/.openclaw/npm/projects/askjo-camofox-browser-2f55486f67/node_modules/playwright-core
export CHROMIUM_PATH=/home/dev/.cache/ms-playwright/chromium-1228/chrome-linux64/chrome
export BGIRL_URL=http://127.0.0.1:8786/
node tests/signature-browser.cjs
node tests/browser-smoke.cjs
node tests/demo-browser.cjs
git diff --check
```

Logs: `/tmp/bgirl-signature-{build,tests,browser,browser-regression,browser-vocals}.log`.
Screenshots: ignored `local/signature/browser/{desktop,mobile}.png` (visually
inspected). These are browser/host tests, not physical MIDI or hardware evidence.

## Original recipes / architecture / compatibility

`tools/signature_bank.py` authors five original155byte FM6 patches and the
versioned manifest; `--check` refuses stale generated data. No purchased presets,
wavetables, private audio, network generation or engine/BSP modifications.

- FM6 factory presets8..12 use existing **OWN**, preserving F1..F8 and B1..B32.
  Only explicit factory selection loads recipes; project/user/editor restores
  preserve edited patch bytes. OP ENV is amplitude; common ADSR is ineffective
  for FM6. MLVL is modulation brightness, not a post-FM resonant filter.
- DRUM presets4..8 append fixed identities. KIT0..11 remains legacy; KIT12..16
  selects kick/snare/clap/closed/open;17/18 selects ungrouped closed/open.
  Group1 is explicit in CH G1/OH G1; FREE opts out. Selected drums ignore key pitch,
  retain velocity and decay tails, reuse their one lane on retrigger.
- TUNE maps0..64..127 to −range..0..+range in1/16semitone increments; range5 for
  kick/hats,3 for snare/clap. Existing TONE/DECY/SNAP/ACC/DRV and track LVL work.
  CLAP uses existing LANES SNARE level as0..127 body blend, factory32, not a second
  sequenced trigger. Both internal states render under one shared voice/envelope.
- Same-block group choke is resolved after serial event dispatch, before per-track
  samples. Open uses existing1.5ms choke release; open never kills closed.
  Renderer reads the group decision and writes only its own state. A new open in
  a later block may ring. Legacy kits and FREE hats never join the cross-track group.
- STOP/panic/replacement clear standalone state; runtime origin survives a sound's
  parameter replacement until panic so solo→kit does not leak the old tail.
- FUND is the existing3840byte compact layout, including FUNC's32 fingerprint
  bytes; no common parameters/flash addresses/extents changed. Only standalone
  selections or DRUM KIT motion require FUND. Older firmware rejects its magic.
  Other projects keep FUNB/FUNC; all old readers stay. New drum presets are7/8
  (notes/grid), older firmware rejects; ordinary presets keep their old versions.
- Signature-category user presets restore saved chord configuration; old preset
  behavior stays unchanged. Project carrier references/live Freeze still clear on
  sound load, and missing vocal fingerprints are not rebound or substituted.
- FM6 MIDI PC0..7 and bank8..39 retain their meanings; Signature factory40..44.
  DRUM PC0..8 retains kits,9..15 appends standalone KIT modes without changing the
  current sound's other settings. Normal factory loads apply complete recipes.

Fresh vocal demo, Blank electro template, existing kits/SQR BASS/BELL/CLOUD PAD,
TRK_DEF, autosave precedence and32 user slots remain. New explicit project loaders
use normal reset/load operations, confirm replacement in the browser, load stopped,
and never write user slots. No startup substitution or public preview deployment.

## Resource / arrangement facts

Eight shared voices; FM6 maximum6/track. Six operators are not six voices.
EL CHORD live root gives root,+3,+7,−12 (MIN/+OCT), up to4; sequencer demos store
final tones explicitly, so no double expansion. A second held root can steal at
FM6's6voice cap; releases participate in inherited global stealing. Released voices
are preferred, then surplus unison, then oldest non-lowest held poly voice; mono
bass and each poly track's lowest held note retain inherited protection.

At90/120/140BPM, four-bar demo A reached7/8/8 voices; B4; choke exercise3. No steals
in these defaults. Separate deliberate overload coverage requires steals and checks
bounded output/release. Arbitrarily stacking long chords can steal; no promise of
unlimited tails. Kick, snare and hats in retained kits still consume separate lanes.

No enlarged engine-memory union or track struct; clap reuses otherwise-unused
snare lane state. New mutable drum bookkeeping:5bytes (four origin flags + group
block decision); patch constants775bytes plus preset/selection/code tables.
Additional clap work is one snare DSP layer and a bounded local coefficient setup.
Browser WASM923430bytes versus baseline916060bytes. Served SHA256:
`938cde01dfe3e239e7cfb2ec626ccd8f102bf0f0ff4e7915b0d032a755f0bb17`.
Local file and HTTP artifact hashes match. Target RAM fit/stack/deadlines/xruns
and physical output levels remain unmeasured; host timing is not hardware proof.

## Listening artifacts / objective measurements

Ignored local gallery: `local/signature/index.html`;29 unnormalized stereo16bit
44.1kHz WAVs +29 whole-file RMS-matched copies under `matched/`.
`measurements.json` records hashes, exact frames, peak/RMS/DC/mono sum and comparison
gain. `context.json` binds source revision, commands, MIDI/velocity/effects.
`manifest.json` includes exact patch bytes, drum controls and curated levels.
Originals are never overwritten by normalization. Whole-file RMS matching is only
a comparison aid (subs and transient crest factors differ), not a loudness verdict.

| Dry reference at velocity100 | Peak | Last sample above relative−60dB |
| --- | ---: | ---: |
| BG SUB CLEAN / MIDI36 |0.0696 |8.058s (8s held) |
| BG SUB DRIVE / MIDI36 |0.0645 |8.058s |
| BG KICK |0.0883 |0.450s |
| BG SNARE |0.1346 |0.319s |
| BG CLAP |0.0984 |0.343s |
| BG CLSD HAT |0.1255 |0.097s |
| BG OPEN HAT |0.1141 |0.444s |
| BG IRON STAB |0.0921 |0.182s |
| BG EL CHORD |0.0799 |0.178s |
| BG CHIRP |0.0767 |0.194s |

Subs measure65.404Hz near expected65.406Hz at MIDI36. Second harmonic/fundamental
is0.0000037 for clean versus0.379 for drive, with comparable peaks: harmonic
addition, not a moving second fundamental. Default mono summing retains energy.
The snare's measured−60dB tail exceeds the initial250ms audition target; its decay
is adjustable and its final tightness is explicitly a listening decision. Initial
attack/time targets were not literal DX-rate bytes. Bass release measures~58ms;
attack rate88/key-sync1 is explicit, not claimed as a calibrated millisecond knob.

Re-render (no private assets):

```sh
cc -O2 -w -ffp-contract=off \
  -Ifelucca/firmware/hal -Ifelucca/firmware/bsp/chip \
  -Ifelucca/firmware/bsp/boards/fm1 -Ifelucca/firmware/src -Ifelucca/build/gen \
  -o felucca/build/host/render_signature tools/render_signature.c -lm
mkdir -p local/signature
felucca/build/host/render_signature local/signature
python3 tools/analyze_signature.py local/signature
python3 tools/signature_gallery.py local/signature
```

Reference context: bass MIDI36 held8s, other MIDI60 gates125ms, velocity100;
clap is one trigger; chord is one live MIDI root. Dry sends0; restrained-wet DLY14,
REV12. All sounds' original factory recipes are dry. Demos are four bars each at
90/120/140BPM, complete editable arrangement—not ten concurrent tracks.

## Local browser audition (no deployment)

Local-only transient user service `bgirl-signature-local.service` serves
`felucca/build/emu` on127.0.0.1:8786. Existing preview services/Pages unchanged.
For access from Doug's machine:

```sh
ssh -N -L 8786:127.0.0.1:8786 dev@openclaw-fedora.tail549b77.ts.net
```

Then open http://127.0.0.1:8786/ → Switch on → Signature blank template → confirm.
Choose a sound → Load sound on selected track → play keys/MIDI. ALGORITHM selects
tracks. Compare demo A/B and hat choke, press PLAY, then STOP. Existing Load electro
demo must still return the vocals. Cancel a replacement to retain current work.

Listening gallery is copied locally to
http://127.0.0.1:8786/listening/signature/ (not checked in or deployed).
To reproduce serving after rendering: copy the gallery and WAV directory to
`felucca/build/emu/listening/signature`, then run Python's HTTP server bound to
127.0.0.1. The gallery needs the separately copied manifest/context files too.

Remaining gates: **Doug listening verdict on all families**, optional recipe/level/
release revisions from that feedback, physical-device fit/performance and actual
hardware MIDI/choke/output checks. No resonance extension justified by listening
has been identified. No Ableton changes, hardware build/flash, merge, release,
new generation, private-asset redistribution or public deployment performed.

## Follow-up: BGirl patch ownership labels

Doug reported being overall happy with the audition and requested a `BG ` prefix
to distinguish our patches from inherited sounds. All ten Signature factory
labels and the web selector now use that prefix; CLOSED HAT is `BG CLSD HAT`
to fit the existing 12-character field. Embedded FM6 voice names are also
BG-prefixed within their existing 10-character field. Existing saved user names
are preserved, not migrated.

Verified: all ten labels fit; manifest IDs, indices, mixer/effect settings and
FM6 synthesis bytes0..144 are identical to the preceding revision. The audio
golden keys were renamed to follow the factory labels; **no golden hash changed**.
Root browser build passes, all108 renders are unchanged, and native/WASM output
is identical for88200 frames. Actual Chromium through HTTP8771 → HTTPS8448
passes all ten sound loads/audio, confirmation, stopped templates, demo/STOP,
panel selection, saved flash/reload, vocal coexistence and mobile checks with
zero page errors. Separately checked all ten web labels and visually inspected
the normal panel's BG OPEN HAT label.

Local receipts: `/tmp/bgirl-prefix-{build,rebuild,tests,browser}.log`;
`/tmp/bgirl-prefix-preset-browser.png` and
`local/signature/bg-prefix-browser/{desktop,mobile}.png`.

# Vocoder clarity — host/browser evidence and audition

2026-11-17 · draft [PR27](https://github.com/track-forge/bgirl-fm1/pull/27),
branch `plan/vocoder-clarity`, main base
`4c7628420b8dd8db7eb2ff81759105794ec80650` (six tracks).
Implementation commits `f945fec` (DSP/control) and `3e3c643` (examples/tools).
Final submitted head and CI receipt are in the PR handoff.
**Human musical approval PENDING. Physical-device feasibility UNVERIFIED.**
No merge, existing public/private preview-service change, device build/flash, dependency
pin change, paid generation, private audio publication or Ableton changes.

## Delivered / compatibility

- BANDS identities0/1 stay8/12;2 adds24. Old coefficients and gain8 remain exact.
  All ordinary panel, editor, MIDI, project and sound-preset controls use the
  firmware descriptors. Whole/chop and stable saw/GRAIN comparisons are explicit
  stopped loaders, not a new startup policy. Existing electro and blank loaders
  remain unchanged; autosave still wins at boot.
- New saved RANGE discriminator: LEGACY retains REL1..127 = approximately2..128ms;
  LOG uses REL0..127 =5..500ms. Display rounds to integer ms. Nearest reference
  choices are code63 =49.101639ms,83 =101.404358ms,94 =151.107132ms.
  These are approximate one-pole e-folding constants, not time to silence or an
  Ableton algorithm match. Existing attack/smoothing stay unchanged.
- [Exact additive wire/default/preset contract](VOCODER_CLARITY_CONTRACT.md).
  Old126-field B6S1 projects/autosaves and old count-mapped sound presets retain
  their release meaning. New127-field records preserve frozen defaults via an
  accessor, shifting old engine motion IDs explicitly. No pre-B6S1 migration.
  New log-mode presets use rejecting versions9/10. Existing valid MIDI-learn
  codes, favorites, slot count and3840B full-save atomicity remain.
- Mode changes clear filter/envelope/held state, including while frozen. A bank
  change while HOLD is on holds an empty new bank; unfreeze and refreeze to
  capture fresh speech. STOP/panic/load still release through normal machinery.
- Appended ANALOG preset12 **BG CARR SAW**: one undetuned saw, open nonresonant
  filter, fast attack, full sustain, no noise, sweep or sends. Stable preset IDs
 0–11 unchanged. Local generated wrapper leaves engine/BSP pins untouched.
  GRAIN comparison uses existing CLOUD PAD unchanged, not an SH-01A patch clone.

## Reproduce locally

From repository root, with documented baseline dependencies and Emscripten6.0.10:

```sh
source /home/dev/.local/share/bgirl-emsdk/emsdk_env.sh
./build.sh --web
./tests/run_tests.sh
# Optional offline listening/measurement tooling additionally uses NumPy:
python3 tools/clarity_audition.py
python3 -m http.server 8794 --bind 127.0.0.1 --directory felucca/build/emu
```

Open <http://127.0.0.1:8794/>. For this OpenClaw host from Doug's workstation:

```sh
ssh -N -L8794:127.0.0.1:8794 dev@openclaw-fedora.tail549b77.ts.net
```

1. **Switch on** → scroll to **Vocoder clarity** → **Clarity whole phrase** →
   confirm → **PLAY / STOP**. Default:24bands, BG CARR SAW,151.107ms, fully wet.
   G6/W01 plays at original speed; MIDI46/velocity94 carrier is independent.
2. Stop; choose12 or24 and explicitly reload the whole phrase for each comparison.
   Choose approximately50/100/150ms, saw/GRAIN, or100%/117-of127 wet. The blend
   is **92.126%**, nearest the92.5% reference; it is not wet-only intelligibility.
3. **Clarity dry speech** and **Clarity carrier only** isolate the two inputs.
   **Clarity chopped phrase** provides rhythmic chops. **Clarity electro
   arrangement** loads an editable four-bar whole hook, answering chops via normal
   bank-view motion, independent carrier melody, and sparse drums/bass.
4. Normal panel T4 → FX twice: VOCODER, BANDS/WET/MON. Next tap: VOC INPUT
   (HOLD), next VOC COLOR (REL inms), next VOC TIME (LEGACY/LOG and REL).
   MON hides direct carrier but retains its feed; ordinary carrier MUTE removes
   the feed. Select T3 and play carrier notes without restarting T4 speech.
5. [Local matched listening page](http://127.0.0.1:8794/listening/clarity/):
   compare whole-12/whole-24, then release-50/100, grain-12/24, blend-24,
   narrow-24/whole-24. Each has raw and matched players. Existing electro demo
   is still explicitly loadable above; cancel preserves unsaved music/latches.

Existing saved slots are never erased by loaders. Use ordinary project SAVE for
an audition; ordinary autosave quiet/write-spacing rules still apply. No new
startup/demo musical acceptance is implied.

## Listening artifacts / bounded coverage experiment

`tools/clarity_audition.py` builds `tools/render_clarity.c` with GCC -O2,
`-ffp-contract=off`, invokes real native web/firmware DSP (not an approximation),
and writes13 raw +13 matched12s stereo PCM16 WAVs, a gallery and JSON to ignored
`local/clarity/`. The gallery copies go only into ignored local build output.
The manifest records source SHA, revision, exact settings, all WAV hashes,
matching gains, compiler/machine, block sizes/timing and tested WASM SHA.
Public-authorized G6 is the only source; no private fixture is required.

Matching: fixed scalar to **-24dBFS whole-file RMS**, retaining deliberate silence;
common target is reduced if any file would exceed0.9 peak. No limiter, trimming,
compression or runtime normalization. Raw files remain unchanged. Twelve seconds
includes the repeated four-bar loop, same carrier note/velocity and release for
isolated tests. Arrangement/chops deliberately change phrasing and are not
isolated DSP comparisons. Gallery browser checks prove player operation, not
that an agent listened to or understood speech.

24-band candidates, holding band count/carrier/release/wet/gain constant:

| Bank | Centers / Q | Outer -3dB edges | Summed power response over center span |
| --- | --- | --- | --- |
| Narrow |180–6500Hz /6|165.6–6977.7Hz|+0.455…+2.324dB|
| Selected wider |100–9000Hz /5|90.5–9679.7Hz|-0.391…+2.151dB|

These are constant-peak RBJ bandpasses; response is sum of squared magnitudes,
not a phase-coherent unity-transfer or full vocoder intelligibility prediction.
All generated filter poles are inside the unit circle; widest candidate max
radius0.9985763. The candidate widens low speech coverage and upper consonant
coverage without pretending the22.05kHz source reaches12kHz. Q scales with log
spacing, not simply the12-band Q. Offline narrow build uses only the scoped
`BGIRL_VOCODER_NARROW24` diagnostic define; no extra runtime EQ parameter.

Runtime gain stays **8**, calibrated against the same G6 source and two carriers:

| Source/carrier |12band RMS / peak|24band RMS / peak|
| --- | --- | --- |
| G6 / saw |0.032454 /0.152313|0.032868 /0.148773|
| G6 / GRAIN |0.020148 /0.134750|0.016143 /0.106522|

24band saw RMS is+0.110dB vs12; GRAIN is-1.925dB. No blanket gain doubling or
carrier normalization: it would overcompensate saw and change inherited modes.
Local matched copies make level bias explicit. This candidate is a measured
engineering choice, **not Doug's band/range/timbre preference**.

`tools/clarity_source_audit.py`:44.1kHz stereo G6 lasts1.918073s; the existing
packager averages channels, box-averages adjacent frames to22.05kHz, applies
-3dB/fades and IMA4. Full-phrase Hann-window energy:3.607% at100–180Hz;
0.124% at6500–9000Hz;0.021% above11025Hz. IMA decoded SNR vs post-gain/fade PCM:
**26.592dB**. Box attenuation at6500/9000/11025Hz:0.966/1.923/3.010dB.
Box averaging is weak anti-aliasing; IMA increases high-frequency noise. Output
at44.1kHz cannot restore original content above11.025kHz. No subjective claim
that these losses do or do not explain articulation; pipeline redesign deferred.

## Verification receipts

Baseline: pristine planning head `e1f7b96` in independent detached checkout;
pinned recursive submodules initialized; root web build + complete root suite PASS.
Implementation validation:

- Root web build and complete host suite; exact status/final revision in PR.
- 109 golden renders:108 inherited/Signature hashes unchanged, only the newly
  appended carrier has a new golden (`f9741419e7e783b8`). No old golden rewritten.
  Explicit legacy8/12 synthetic32k-frame hashes remain875ba8e9/ee7cbd4f.
- Existing heavy-song native/WASM88200frames and clarity12/24 **176400frames**
  bit-identical. New editor test checks every128 log/127 legacy displayed value.
  Firmware editor DESC/SET exposes three bands and both release formats; RANGE
  edits trigger descriptor refresh. MIDI endpoint and all-ID mapping tests pass.
- Compatibility: authentic126-count B6S1 fixture with T6 step63 engine lock,
  old count-mapped vocal preset, new project/preset round trips and vocal identity,
 30 whole/chop/carrier/dry/arrangement configurations, REL locks/motion/MIDI,
  bounded stressed mode/HOLD switches, STOP/reset. Full saves remain atomic;
 3839/3840/3841-byte edges and20000 checksum-valid corruptions retained.
- Extended workers:504 original pair/core-mask comparisons,270 additional T5/T6
  comparisons,216 whole/chop comparisons;182 eligible-carrier comparisons retained.
  Rejected/partial/completed-failed recovery, <=1 render/track/block, all modes.
- CPU2-off clarity and ASan codec/clarity runtime checks pass. No TSAN claim.
- Actual Chromium: splash, smoke, vocoder, demo, vocal, dance, six-track,
  Signature and clarity suites. Clarity covers five nonzero loaders,8/12/24 normal
  panel selection,5/500ms normal knob endpoints, Freeze/STOP, cancel/latches,
  log-mode autosave/fresh instance, manual save and mobile no horizontal overflow.
  Gallery checks26 players and actual playback of five matched12s files.
  Desktop/mobile screenshots visually inspected; no page errors observed.

Logs: `/tmp/bgirl-clarity-{baseline-build,baseline-tests,web,tests,focused,midi,editor,
single,asan,asan-runtime,browser,gallery-browser,audition}.log` and
`/tmp/bgirl-clarity-<suite>.log`. Browser screenshots: `local/clarity/browser/`
and the existing suites' ignored `tmp/` directories.

The retained listening manifest records clean implementation revision `3e3c643`;
later changes are single-core padding, display captions, tests and documentation.
The submitted WASM matches its recorded SHA above.

Actual Chromium reproduction (set installed module/executable as appropriate):

```sh
export PLAYWRIGHT_MODULE=/tmp/moltazine-351-deps/node_modules/playwright
export CHROMIUM_PATH=/home/dev/.cache/ms-playwright/chromium-1228/chrome-linux64/chrome
export BGIRL_URL=http://127.0.0.1:8794
for suite in splash-browser browser-smoke vocoder-browser demo-browser vocal-browser dance-browser six-browser signature-browser clarity-browser clarity-gallery; do
  node tests/$suite.cjs
done
```

## Measured cost — host only, no FM-1 deadline claim

GCC15.3.1, -O2/noFMA, Linux/AMD EPYC with IBPB,44.1kHz,128-frame blocks;
five alternating24s runs per mode, identical six active tracks, eight admitted
voices reached, competing FX/sequence/whole-hook/chop activity. All counters and
per-run values are retained in the local manifest. Shared host, not CPU-isolated.

| Mode |Median of mean block time|Per-run p99 range|Largest observed block|
| --- | ---: | ---: | ---: |
|12|40.031µs|89.034–102.444µs|2067.105µs|
|24|52.585µs|106.732–133.640µs|843.947µs|

Mean-cost ratio approximately**1.314×**. Scheduling outliers/maxima are not a
claim that24 is faster at worst case or meets device deadlines. Browser suites
reported no page/AudioWorklet failures; no physical-xrun instrumentation exists.

DSP state348→688B (**+340B**); captures280B unchanged. CPU2-on track_t1856B stays
unchanged by reusing padding; single-core track_t1840B also retains alignment.
Decoded project7284→7296B (+12); wire3840B/slot unchanged. Same native-check
harness BSS1654400→1654880B (**+480B**), data1588B unchanged; text901764→905164B.
Host BSS includes emulator state and is not target linker RAM. Browser WASM
916254B vs baseline919864B; SHA256
`054e7204a628ec4a494c7dc69a5b7649585d4493f129ffbb20532fad0cbae863`.
WASM size is not hardware flash fit. No render-path allocation/I/O/blocking added.

## Remaining gates

Doug must compare the same complete phrase12vs24 at matched levels, judge wet-only
intelligibility separately from dry blend, then approve or reject the arrangement.
Record carrier/bands/release/wet/range with that verdict. **Musical gate open.**
Physical RAM/linker/cache/stack fit, actual FM-1 deadlines/xruns, USB/TRS behavior,
new-chip/storage/package compatibility and flashing are separately authorized
future work. **Hardware feasibility unresolved;8/12 modes retained.**

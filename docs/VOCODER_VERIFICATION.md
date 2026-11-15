# Vocoder host/browser verification

2026-11-15. [Draft implementation PR #6](https://github.com/track-forge/bgirl-fm1/pull/6). Implementation review on `feat/sample-fed-vocoder`, stacked on open
planning PR #5 (`7aa442c281293698b3f8b5c0fa47286a6105d20d`). No merge, device build,
flash, release/site publication, BSP/engine pin advance or installer activation.

## What shipped for review

- Four original dry VoiceDesign phrases, stable IDs and source/output provenance.
- Existing SLICE/IMA decoding, keys/steps, one-shot/retrigger/original-pitch bank.
- One selected SLICE track / selected carrier, bounded single-writer captures,
  unchanged fork/join scheduler, one post-join processor before shared buses.
- Actual carrier audio after DIST/INSERT/SLICER and before mixer level/pan/sends;
  independent dry monitoring, three normal four-knob pages, Freeze and character.
- FUNB save variant in the existing3840-byte object: controls/routing/bank saved,
  live HOLD cleared. All old-format readers retained. Sound presets clear routing.
- Early local WASM speech lab plus five comparable9s FLAC references.

## Exact checks and results

From repository root, generated assets/dependencies initialized:

```sh
source /path/to/bgirl-emsdk/emsdk_env.sh # Emscripten6.0.10
./build.sh --web
./tests/run_tests.sh
./tests/render_vocoder.sh
python3 tools/package_listening.py
python3 tools/package_speech.py
```

Observed PASS:

- Baseline runner before source changes (same existing toolchain/goldens).
- Final root browser build and complete documented root runner: storage, user
  presets, input, TRS MIDI, old/current projects, persistence, UI/help, MIDI/clock,
  long-run MIDI phase/drift, swing, backup web tests, BGIRL baseline integration.
- **98 upstream golden renders unchanged**, zero health/voice/routing failures,
  no rewritten goldens. CPU instruction counters zero on host: not target budgets.
- Native/WASM heavy-song output **88200 frames bit-identical**; full upstream WASM
  boot, keys, MIDI, transport, knob and saved-preset checks passed.
- New integration: original-pitch key mapping, one-shot release (including note-off
  before first render), STOP on user-loaded modulators, newest phrase/no
  fallback (also with stored UNISON), independent carrier notes, Freeze continuing
  after sample end, unfreeze decay, stop/panic, invalid/self-source, no stale input
  or carrierless consonant bleed, pre-level carrier feed, monitoring/sends,
  speech level/pan/mute, project controls/bank roundtrip, load unfrozen and preset
  routing clearing. Factory speech slice editing refused safely.
- Authentic old **FUNA** fixture restores edited sound42 / step41 and routing OFF.
  Fixture generated with baseline source **45590dfe97d11dfaf022cfd002266ac7218481b9**,
  SHA256 `507c6a780121fb98dd794908d731157088103347ddf70f258c158bce43c1693a`.
  `tests/make_legacy_fixture.c` refuses to compile against the new writer, ensuring
  this fixture isn't silently regenerated as FUNB. All older format suites pass.
- New DSP stress: **32000 frames**, maximum trims/character and oversized inputs,
  bounded finite output. This is numerical evidence, not listening approval.
- `tests/vocoder_dual_test.c`: **336** bit-identical ordered-pair/core-split comparisons
  (both8/12 bands,12 ordered track pairs,14 nontrivial splits), plus **182** comparisons
  across all13 eligible carrier engines. Actual pthread worker with block-local
  modulation, per-track render counts <=1; **12 each** rejected-before-execution,
  partial-started and completed-but-failed join scenarios. No partial-job replay.
- Upstream `felucca/tests/dualcore_test.c`: **28 scenes, zero differences** across
  its rotating/automatic/fixed split matrix, including spread, FX and modulation.
- Explicit `-DFELUCCA_CPU2=0` build/new integration passed (not merely a disabled
  worker in a CPU2-enabled binary).
- `-DBGIRL_VOCODER=0` native heavy-song bytes identical to normal route-OFF build
  via `cmp`; ordinary feature-OFF audio equivalence also covered by98 goldens.
- Two offline speech packaging runs produced identical manifest/header/blob hashes.
- `git diff --check` passed; dependency gitlinks unchanged.

### Actual Chromium

With `python3 -m http.server 8771 --bind 127.0.0.1 --directory felucca/build/emu`:

```sh
PLAYWRIGHT_MODULE=/path/to/playwright-core CHROMIUM_PATH=/path/to/chrome \
 BGIRL_URL=http://127.0.0.1:8771 node tests/browser-smoke.cjs
PLAYWRIGHT_MODULE=/path/to/playwright-core CHROMIUM_PATH=/path/to/chrome \
 BGIRL_URL=http://127.0.0.1:8771 node tests/vocoder-browser.cjs
```

Both passed with Playwright1.59.1 / Chromium149.0.7827.55. Baseline checks real
AudioWorklet, panel, all track screens, sound save/reload and confirmed reset
without erasing slots. Speech-lab test verifies actual dry/8/12 audio, normal
FX-page/knob screen changes (VOC INPUT HOLD ON), five served lossless references,
portrait width and zero page errors. Desktop/portrait screenshots inspected;
no horizontal overflow. Timing-dependent peaks are not quality scores.
Private detailed logs/screenshots: local `/tmp/bgirl-vocoder-*.log`,
`tmp/vocoder-browser/`, `tmp/browser/`. No tokens/private receipts committed.

### Extra native commands

Use `I='-Ifelucca/firmware/hal -Ifelucca/firmware/bsp/chip -Ifelucca/firmware/bsp/boards/fm1 -Ifelucca/firmware/src -Ifelucca/build/gen'`:

```sh
cc -O2 -w -ffp-contract=off $I -pthread felucca/tests/dualcore_test.c -lm -o felucca/build/host/dualcore_test
felucca/build/host/dualcore_test
cc -O1 -w -ffp-contract=off $I -DFELUCCA_CPU2=0 tests/vocoder_test.c -lm -o felucca/build/host/vocoder_single_test
felucca/build/host/vocoder_single_test
cc -O2 -w -ffp-contract=off $I -DBGIRL_VOCODER=0 felucca/web/emu/native_check.c -lm -o felucca/build/host/emu_native_off
felucca/build/host/emu_native_off felucca/build/host/emu_native_off.f32
cmp felucca/build/host/emu_native_off.f32 felucca/build/host/emu_native.f32
```

## Resource evidence, not target fit

Native GCC15.3.1 / Python3.14.5. Default flags: SLICE=1, CPU2=1, FM4=0,
ADPCM=0, BGIRL_VOCODER=1. Alternative engine/target flag combinations are not
claimed by this browser milestone. DSP state348bytes, capture+metadata280bytes
(two mono int32[32] inputs alone256bytes). Speech transition state48bytes;
track_t1856bytes in CPU2 build /1840bytes in explicit single-core build. Track
parameter/pad growth is16bytes per track. Coefficients400read-only bytes; ADPCM
**54357bytes /108714samples at22050Hz (4.930340136s)** plus immutable SLICE tables,
source labels and descriptors. See packaged manifest for individual phrase lengths.
Initial milestone local WASM841523bytes, SHA256
`c37e01a3d0aabe99930ac498d8b7349216deec4ae95478b42caf9006b0d5ea69`.
Twelve-band algorithm runs24biquads per sample; eight runs16 (operation estimate,
not cycles). Actual target alignment, linker map, cache visibility, stack and CPU
cost remain unmeasured. WASM byte size/hash is a toolchain artifact, not device flash.

## Remaining acceptance / limitations

- **Human intelligibility/source quality and8/12 preference pending.** Both remain
  selectable;12 is provisional. Consonants default OFF. No automated sound approval.
- ThreadSanitizer attempted with `-fsanitize=thread`: **UNAVAILABLE**, linker cannot
  find `/usr/lib64/libtsan.so.2.0.0`. No race-free claim. Pthread checks passed.
- No physical-device render deadlines/xruns, worst-case4-track load, linker RAM/
  flash/stack fit, new-chip compatibility, physical USB/TRS/Web MIDI or latency proof.
  Inherited full upstream runner also needs unavailable device SDK/packages/target
  disassembly; only documented root subset and explicit extra tests are claimed.
- One vocoder owner (lowest eligible track wins); self-route unsupported. Missing
  source displays NO SRC, conflicting owners BUSY. No new general modular graph.
- Participating tracks use mono shaping (saved SPREAD ignored while connected).
  Ordinary existing user SLICE sources retain their own modes; strict factory
  SPEECH mapping is fixed forward/original-pitch/one-shot with four phrase boundaries.
- New FUNB saves cannot be read by old firmware; old saves remain readable here.
  Retired DIGITAL OP fields are absent from new compact records; active DIGITAL
  is refused by writer (pinned build already disables FM4). No flash map rewrite.
- Worker failure recovery requires the BSP join failure contract: worker is held
  before return. Inherited target cache/hold/synchronization not physically proven.
  Drop worker sums, kill affected voices/insert state; no rollback or seamless audio.
- Existing chorus is mono insert thickening, not circuit-accurate SVC/BBD ensemble.
  Three character groups, no custom compressor/mic/USB audio or time-stretching.
- Host output can retain tiny inherited master-filter quantization tails; routing
  silence is asserted at the captured/processed bus separately from downstream tails.
- TTS stochastic regeneration isn't bit-identical; retained original FLAC/PCM and
  deterministic offline packaging are the reproducibility boundary. Rights/provenance
  caveats are explicit in the speech manifest; no voice impersonation/source record.


## Bell-carrier listening follow-up (2026-11-15)

Doug's feedback: overall a win, and he expects the vocoder to work. Requested
front-loaded vocoder pages and borrowing the electro demo's bell as carrier.

- FX page1 stays FX; pages2/3/4 are VOCODER / VOC INPUT / VOC COLOR, followed
  by SLICER / INSERT / INSERT2 / DLY / REVERB / CHORUS. Parameter IDs unchanged.
- Wet8/12 buttons now retain electro-demo T1 drums/T2 bass/T3 FM6 BELL sounds
  and patterns; only T4 VOX is replaced with speech. CARR=T3, MON=ON.
- Dry speech and existing five isolated ANALOG comparison assets stay separate.
  No DSP, save schema, bank assets, engine/BSP or default startup changes.
- Focused host checks assert all three original parts' sounds/steps unchanged,
  both band choices, correct actual FM6 bell carrier, four speech triggers and
  removal of the old VOX reply; FX page-order checks retained alongside normal
  keyboard/panel routing tests. Upstream second-FX-tap assertion deliberately
  now expects VOCODER instead of SLICER, as requested.
- Shared downstream FX tails survive project replacement intentionally; isolate
  bus assertions before running the full electro-groove demo checks.
- Actual tailnet HTTPS Chromium audition harness passes8/12 output, CARR=T3 /
  MON=ON / VOCODER2/10 inspected in screenshot, third-page Freeze knob, served
  references, portrait and zero page errors. These are operation checks, not a
  new intelligibility verdict on the sparse, decaying bell carrier.

Refresh https://openclaw-fedora.tail549b77.ts.net:8447/ then explicitly load8/12
and PLAY (an already running browser retains its old project until reloaded).
Select track4, tap FX slowly twice: CARR=T3; fourth knob MON disables the dry bell
without disabling its feed. Track3 keys play the bell independently of speech.
For a less busy audition, mute tracks1/2; do not mute track3 to hide the dry bell.

## Sustained-pad listening follow-up (2026-11-15)

Supersedes only the live bell setup above: wet8/12 now use a sustained, open-filter
detuned ANALOG saw carrier on T3, C3/G3 with tied steps. CARR=T3, MON=OFF.
T1/T2 patches/patterns, factory startup electro, speech sources, isolated reference
recordings, DSP, storage and dependency pins unchanged. Refresh the same private
HTTPS URL, explicitly load8/12 again, then PLAY. MON ON auditions the raw carrier.

- Web build:841820B WASM, SHA256
  `3ff7d40f1d6b6a1f536a61bc1d83fda8b45c759c0288e19bbcbd18867211ca8e`.
  Download from actual HTTPS preview matches local output exactly.
- HTTPS Chromium speech lab: dry/8/12 produce bounded audio; normal Freeze knob,
 5 served original references, portrait layout and zero page errors pass.
  Routing screenshot confirms CARR=T3 / MON=OFF / VOCODER2/10.
- Host demo regression checks preserved drums/bass, both bands, default MON OFF,
  tied saw notes and a nonzero actual carrier feed across four seconds per mode
  (not merely the drum/bass mix). Full host runner results recorded in PR.
- Human pad intelligibility approval and physical performance remain unverified.

## Main-feature electro demo follow-up (2026-11-15)

Base: freshly fetched main `f493fddafae61c52dfcc8f5290b8a93c239736ad`,
merged PR6 stack. Doug called the sustained-pad audition a win.
Fresh startup and Load electro demo now share the12-band pad+speech arrangement.
The8/12 speech-lab buttons reuse it with a band override; isolated diagnostics
and old recordings remain unchanged. Autosaves, saved slots, blank template and
BGIRL_VOCODER=0 original startup demo are preserved. No new DSP/save format/pins.

Private preview: https://openclaw-fedora.tail549b77.ts.net:8447/ . Refresh, click
**Load electro demo**, accept replacement of unsaved work, then PLAY. Existing
autosaves deliberately do not update themselves. T4 second slow FX tap shows
CARR=T3 / BANDS12 / MON OFF. The blank template remains bell/VOICE, no steps.

Web build842141B SHA256
`6bca3c4b783caf98be42829c5ddf39888edd05938ce3113beea40e32c577fa2d`.
Screenshot: `docs/img/electro-vocoder.png`.
Host tests cover fresh/explicit demo, real processed speech-bus energy (not
backing-only output), edited project/autosave preservation, blank palette,
STOP/release and an explicit compile-disabled original-demo run. Exact results
and revision recorded in the follow-up PR. Physical performance still unverified.

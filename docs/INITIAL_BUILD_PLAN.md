> **Historical prototype only.** Source/assets/tests and old relative paths refer to
> checkpoint `4cc1e11`, not the active Felucca edition. See [current status](BUILD_STATUS.md)
> and [provenance](UPSTREAM.md). Vocoder integration is deferred.

# BGIRL FM-1 — initial build plan

Date: 2026-11-15. Status: **handoff-ready plan; electro implementation not started**.
Repository: https://github.com/track-forge/bgirl-fm1

## Intent and tonight's finish line

Build a pocket electro instrument, not a general-purpose synth workstation.
Drums and bass supply the groove; playable robot voice is an identity-defining
experiment, not a cosmetic effect. B-girl/crew visuals remain exploratory.

Tonight's minimum useful delivery is a reproducible, locally playable browser/host
slice with a 16-step kick/snare/closed-hat groove, mono bass, start/stop, tempo,
and drum mute/return. Attempt one short sample-fed playable vocoder early, timeboxed
before polishing the groove. Deliver the voice findings even if the experiment fails.
A passing template alone is setup, not this finish line. A partial result is fine
if explicitly labeled; do not claim an entire firmware or hardware readiness.

## Starting point (already prepared)

- `base/`: pinned recursive fm1-base/BSP dependencies; see [provenance](UPSTREAM.md).
- `app/`: unchanged upstream template source, config, words, and scripted tests.
- `build.sh`: host/web-only wrapper. `tests/run_tests.sh`: repeatable baseline checks.
- No electro DSP, sequencer, custom UI, generated speech, device package or publishing CI.
- TEMPLATE name, Bluetooth behavior, flash offsets and package prefix are temporary
  baseline values, not chosen BGIRL architecture. Do not ship under that identity.

## Work sequence and acceptance gates

### 0. Reproduce the baseline

1. Read this plan, `AGENTS.md`, `base/template/README.md`, `base/DESIGN.md`,
   `base/LICENSING.md`, and the app/platform audio and input contracts.
2. Create `feat/initial-electro-slice`; initialize recursive submodules without updating pins.
3. Run `./tests/run_tests.sh`. Record compiler/Python versions and exact revisions.

**Pass:** host build and all copied tests pass. Capture baseline WAV/screenshots locally.
Do not run the Bluetooth-dependent baseline with `--no-ble` and then delete its failures.

### 1. Own the app identity and prove the browser loop

- Rename display/build/USB metadata to BGIRL; update screen-reader words and assertions
  consistently. Remove inherited upstream installer/site claims; don't invent live URLs.
- Keep template internal symbol names until changing them is useful; no broad refactor.
- Install/pin Emscripten (upstream template references emsdk 6.0.10; verify this revision's
  requirements). Build `./build.sh --web` and serve `app/build/emu/` locally.
- Verify browser audio after user interaction, keys, knobs, note release and screenshot.
  Document a reproducible local server command and the actual listening test status.
- Add host-only build/test CI with recursive checkout and read-only repository permissions.
  Browser CI is optional once its toolchain is pinned. Do not add release/site publishing.
- Package identity allocation/uniqueness, install metadata and device config stay deferred;
  preserve the host/web-only wrapper until those questions are resolved.

**Pass:** branded app builds, host regression passes, local browser controls and audio work.
If browser setup blocks, retain a host audio demo and report the browser blocker explicitly.

### 2. Early voice feasibility spike (before extensive groove/UI work)

Timebox approximately 60–90 minutes; this is a suggestion, not a promise of completion.

- Use one short, dry, original speech phrase as the modulator; a small band-bank vocoder
  and a playable monophonic carrier are the simplest experiment worth testing.
- Load/decode off the render path; bound buffers and per-block work. Pick band count
  from measured quality/cost, not an assumed FM-1 budget.
- Compare the same phrase as dry speech, live carrier-controlled vocoding at several
  pitches, and optionally a pre-rendered robot hook. Save WAVs and exact reproduction steps.
- Preferred source workflow is off-device Moltazine/Crucible TTS, discovered at runtime.
  Do not make model/API availability a build dependency. An original recorded phrase
  supplied with documented rights works too; synthetic noise is only a DSP test fixture,
  not evidence of intelligibility. If no suitable speech exists, report that gap.
- Record duration, sample rate/format, bytes, provenance and conversion command. Keep
  API credentials and private download links out of this repository.
- Record static/buffer RAM, sample bytes, build size and host timing as host-only data.
  At 44.1 kHz and 256 frames the nominal block interval is about 5.8 ms; actual target
  worst-case rendering must fit its block deadline with UI/MIDI load and margin.

**Pass:** evidence of audible articulation and carrier pitch control, or a concrete
failure report with next experiment. Pre-rendered playback is a labeled fallback,
not proof of a playable vocoder. Hardware CPU, xruns, RAM and storage fit remain unknown.
Write `docs/VOICE_SPIKE.md`; don't let a difficult voice spike consume the whole session.

### 3. Tiny musical vertical slice

Suggested narrow defaults (change with documented reasons):

- One fixed 16-step pattern at 120 BPM; adjustable tempo, no pattern editor yet.
- Three lightweight synthesized drums: kick, snare, closed hat. Original sound design.
- One mono bass with short amplitude envelope and one useful tone control.
- One voice reply only if milestone 2 produced usable output; otherwise make voice
  absence/fallback explicit. Never silently drop the voice goal from the roadmap.
- Minimal screen: running state, tempo, active part, current step and mute state.
- Provisional controls: ARP start/stop, K1 tempo, PRESET select bass/voice play mode,
  keys play selected part, FX drum mute/return. Check platform semantics first and
  document final assignments; preserve MASTER and note-off/panic behavior.
- Use audio-frame timing, not UI frame timing, for step advancement. Keep fractional
  timing so long loops don't drift; define stop/restart (restart on step 1 is fine).
- Conservative master gain/headroom; explicit overflow/clipping handling when mixing.

Suggested modules once warranted: `app/dsp/`, `app/sequencer.*`, `app/assets/`.
Use the build's documented source discovery/`o2` options, not a parallel build system.
No new abstraction framework required. Start in the existing app and extract clear units.

**Pass:** reproducible 30-second beat/bass demo, hands-on mute/return, stable tempo,
no stuck notes after stop/release/panic, bounded mixed output, and passing regression.
Add focused tests for step timing over multiple loops, restart/mute behavior and
worst-case mix bounds. Keep functional MIDI/input/accessibility checks as the UI changes.
Use unit tests or host script assertions where they actually catch errors; listening
and browser verification cannot be replaced by a peak threshold.

### 4. Close out with evidence

- Update README with actual commands, controls, implemented/missing features and demo recipe.
- Write `docs/BUILD_STATUS.md`: exact commit/tool versions, commands and outcomes,
  local artifact paths, voice decision, known limitations and next tasks.
- Commit coherent chunks; push the feature branch and open a review PR if remote access
  is available. Do not merge, tag, publish a site/release, or flash during this handoff.
- Keep a useful partial result if blocked; state the first failing command and recovery path.

## Explicitly outside tonight's scope

Full pattern/song editor, preset browser, external clock sync, sample import UI,
polyphonic vocoder, time stretching, effects suite, radio/network features, art/animation,
and automatic releases. Persistence redesign can wait; don't claim new musical state
survives reboot unless implemented/tested. Do not expand the old template flash map casually.

## Subsequent device milestone (separate from tonight's acceptance)

1. Review a distinct package identity and a radio-off device configuration; keep host
   baseline radio coverage separately. Review licenses for the chosen linked components.
2. Set up pinned JieLi toolchain/AC79 SDK from upstream instructions; record reproducibility.
3. Audit linker/map RAM, firmware/settings/resource regions, sample encoding and flash fit.
4. Build a development package without publishing. A successful package is not a device test.
5. Schedule physical testing with Doug: stock baseline, backup/recovery readiness, then
   approved flashing. Measure worst-case combined rendering/UI/MIDI, xruns and latency.
6. Decide live vocoder versus rendered fallback using measured device evidence and listening.

## Copy/paste handoff

> Work in `track-forge/bgirl-fm1` (local `codebase/bgirl-fm1`). Read `AGENTS.md`
> and `docs/INITIAL_BUILD_PLAN.md`, then implement the initial electro slice on
> `feat/initial-electro-slice`. Start by reproducing `./tests/run_tests.sh`.
> Follow gates 0–4: own the app identity, prove local browser audio/controls, timebox
> an early sample-fed playable-vocoder spike, then build the minimal drums/mono-bass
> groove with start/stop, tempo and mute/return. Keep the pinned base/BSP unchanged.
> This is implementation authorization for the app and local tooling, not flashing,
> release/site publication or merging. Preserve provenance and real-time audio discipline.
> Deliver small commits, tests, a 30-second demo recipe, voice findings, BUILD_STATUS,
> and a review PR if access allows. Distinguish host/browser evidence from untested
> hardware feasibility; document blockers instead of pretending they passed.

# Vocoder clarity tranche: 24 bands, release range, and audition presets

Status: **plan only; implementation and human listening pending**.
Date: 2026-11-17.
Planning base: `4c7628420b8dd8db7eb2ff81759105794ec80650` (main, six-track PR #26 merged).

## Goal and delivery boundary

Make an intelligible, musically useful classic electro robot voice and provide a
repeatable way for Doug to choose its character. The existing vocoder works; this
is a focused quality/usability tranche, not a replacement synth architecture.

This PR initially adds only this document. When Doug hands this plan to the
implementation agent, continue on this PR's branch with small reviewable commits.
Do not implement during the planning turn. No merge, public deployment, device
build/flash, release/tag, dependency-pin changes, paid generation, or changes to
Doug's Ableton set/hardware are part of the implementation handoff. Local host and
browser builds, local audition artifacts, code/tests/docs and PR updates are in
scope. Keep existing preview services unchanged unless separately requested.

Read AGENTS.md, README.md, BGIRL_BASELINE_PLAN.md, UPSTREAM.md, BUILD_STATUS.md,
VOCODER_CONTRACTS.md, VOCODER_VERIFICATION.md and SIX_TRACK_VERIFICATION.md first.
Historical documents are evidence, not proof of current runtime defaults.

## Evidence and current baseline

Inspect these at the implementation base before editing:

- `felucca/firmware/src/main.c`: `bgirl_demo_vocoder()` currently selects engine8,
  factory GRAIN / CLOUD PAD on T3, MIDI36/43 tied notes; T4 uses G6 chops where
  available, 12 bands, carrier monitor off, consonants off. Do not repeat the older
  ANALOG-saw description from historical contract notes.
- `felucca/firmware/src/bgirl_vocoder.c`: real two-input filterbank, 8/12 bands;
  rectified analysis envelopes, fixed .025 attack and .01 additional smoothing;
  release coefficient `1/(44.1*(P_VREL+1))`; gain factor8; high-passed original
  speech consonant assistance, not a voiced/unvoiced noise classifier.
- `tools/vocoder_coeff.py`: logarithmic centers180–6500Hz; Q2 for8, Q3 for12.
- `felucca/firmware/src/params.c`: WET default127/127; REL1..127/default32,
  approximately2..128ms/default33ms under the current coefficient formula.
- `tools/package_vocals.py`: packaged speech is mono22.05kHz IMA4. Decoding to a
  44.1kHz render stream does not recover source content above11.025kHz.
- Six full tracks share eight voices. B6S1 has frozen defaults and a bounded3840B
  object. Preserve full-save atomicity, settings, editor counts and motion behavior.

### Doug's Ableton reference (inspected 2026-11-17)

Live12.4.6,128BPM. VOCALS originals plays whole T75_S008_candidate_float, loop4beats,
warped. SH-01A clip plays one MIDI46 note, velocity94, duration8beats, direct track
muted. Saved set routes SH-01A PostFX into Vocoder. Hardware octave/transpose means
MIDI46 alone does not prove the actual acoustic fundamental.

Live API values: range80Hz–12kHz, attack1ms, release150ms, width100%, neutral
formant, depth100%, Precise/Retro off (Precise), Enhance off, unvoiced level -inf,
stereo carrier mode, wet92.5%. Doug preferred24 bands over12 and36 over24 but
wants bounded cost. Live API does not expose band count; saved raw enum5 is not an
independently verified current selector. Saved wet100% differs from live92.5%.

No reference audio was heard/captured by the agent; no authoritative SH-01A patch
dump was obtained. Photo sliders are clues, not recalled-patch truth. This is a
parameter reference and Doug's listening report, not a proven sonic match.
Private local receipts exist at `tmp/vocoder-reference-20261117/` in the OpenClaw
workspace; do not require them for public builds or commit the private ALS/audio.

[Ableton Vocoder manual](https://www.ableton.com/en/live-manual/12/live-audio-effect-reference/#vocoder)
explains bandwidth, envelope timing, Enhance and unvoiced synthesis.
[Egyptian Lover's modern Moog16-channel demonstration](https://www.youtube.com/watch?v=Pze1mCblmBI)
is a musical reference, not evidence of his original-record equipment or a claim
that band count alone determines intelligibility.

## Required scope

### 1. Selectable 24-band mode

- Retain8/12 modes and their established sound/routing where unchanged.
- Add24 through normal panel/browser/editor/serialization paths; preserve existing
  enum identities. Audit every fixed-size array, loop, clamp, test and client count.
- Generate appropriate coefficients for24; measure aggregate response, crossover
  coverage and filter stability. Do not reuse12-band Q blindly or simply double
  sum gain. Calibrate gain/headroom with repeatable inputs and source pairs.
- Coefficients/configuration must not be regenerated with expensive math per sample.
  No allocations, logging, file I/O or blocking inside render.
- Test mode changes while playing and frozen: no stale bands, invalid accesses,
  NaNs, stuck state or uncontrolled spikes. State transitions must be explicit.

### 2. Expanded release control

- Extended range5–500ms, logarithmic mapping over the existing compact control
  range where practical; show milliseconds on the panel and relevant browser/editor
  surfaces. Document endpoints, rounding and quantization.
- Provide a nearest-representable150ms reference preset and record its actual value.
- Use one shared mapping for DSP, display and authored presets. Convert control
  values to coefficients outside the per-sample inner loop. Define time constants
  explicitly; Ableton and BGIRL labels do not prove identical envelope algorithms.
- Inventory project/autosave, sound presets, parameter locks, motion, MIDI and editor
  storage before changing interpretation. Do not silently reinterpret old REL values
  as a new time or change version-frozen `project_defaults.h`.
- Prefer an additive mapping/version discriminator with missing legacy fields
  retaining old semantics, or another narrowly scoped explicit compatibility design.
  Document the chosen wire/default contract before coding and test old/new fixtures.
  The previous six-track migration waiver is not blanket permission for another
  unrelated save break. No broad storage-format rewrite in this tranche.

### 3. Repeatable presets and examples

- Named BG-prefixed stable saw carrier and existing GRAIN/CLOUD PAD comparison.
  Reuse existing engines; no new oscillator required. Keep inherited presets intact.
- Whole, original-speed phrase as diagnostic baseline, before rhythmic chopping.
  Separate whole-phrase and chopped musical examples; normal independent carrier
  notes must not retrigger speech. Expose carrier-only monitoring and dry speech.
- Fully wet and lightly blended (~92.5% wet) variants, using existing wet control.
  Record actual quantized wet value; no false exact-match promise.
- Use150ms as reference release; compare50/100/150ms with other settings fixed.
  Existing CONS may be auditioned but a new unvoiced algorithm is out of scope.
- Preserve the current electro demo as an explicitly loadable comparison. New
  auditions load stopped, require existing replacement confirmation, preserve
  saved slots and never override autosave restore. Cancel leaves music unchanged.
- Develop an editable electro arrangement from the preferred voice: complete hook,
  answering chops, independent carrier melody and room in drums/bass for articulation.
  Do not declare the new startup/demo musically approved before Doug listens.

### 4. Bounded frequency-range experiment

- Compare current180–6500Hz center coverage against one wider speech-focused
  candidate (starting hypothesis roughly100–9000Hz, refine from source spectra).
  This is a measured design experiment, not a new multi-knob EQ subsystem.
- Distinguish filter centers from outer bandwidth edges. Keep number of bands,
  carrier, release, wet mix and level constant when isolating coverage changes.
- Do not blindly copy80–12000Hz: the packaged source bandwidth is lower. Inspect
  dry source versus packaged/decompressed speech for articulation loss before
  blaming the carrier or filterbank. Audit resampling/IMA effects; document findings.
  Any substantial sample pipeline redesign is a follow-up, not hidden scope.
- Record the selected24-band range/Q/gain and why; retain old8/12 coefficients
  unless an intentional independently justified change is documented.

### 5. Listening comparison page and reproducible artifacts

- Extend existing local listening infrastructure where possible. Clear settings,
  dry speech/carrier/wet players, sample provenance, loop identity and revision.
- Start with a bounded matrix:12vs24 at otherwise identical settings; then
  release50/100/150; sawvsGRAIN; fully-wetvsblend; oldvswider coverage. Reuse common
  baselines rather than generating every Cartesian combination.
- Retain raw renders plus consistently level-matched audition copies. State the
  matching method/gains, avoid clipping, and retain deliberate silence. Matching
  is offline audition processing, not a change to runtime output.
- Render the real native/WASM DSP, not a separately implemented approximation.
  Record configuration, source identity/hashes, frame rate/length, engine revision
  and repeatable commands. Capture settings alongside any human listening verdict.
- Public fixtures use existing redistributable speech and build without private
  assets. Doug's exact source can be a separate ignored local/private audition.
  Do not commit private vocals, Ableton projects, or commercially sampled records.
  No live Ableton monitoring or musical approval claims from signal checks alone.

## Implementation sequence and gates

1. **Baseline and contracts.** Verify current main/branch/pins, initialize pinned
   dependencies, run existing web build and root host suite. Inventory all affected
   parameter/persistence surfaces and current examples. Write the small compatibility
   contract, baseline render settings and performance measurement method.
2. **DSP/control.** Add24-band mode and release mapping with regression tests and
   UI/editor parity. Preserve8/12 baseline output where settings are unchanged.
3. **Experiments and examples.** Run bounded coverage/release/carrier comparisons,
   create presets and whole/chopped examples, expose the listening page. Correct
   stale carrier documentation while retaining historical context.
4. **Verification and handoff.** Run required suites and actual Chromium checks,
   record performance/memory and listening artifacts, update PR/docs. Leave a clear
   audition recipe and ask Doug for a listening verdict; do not merge/deploy.

Use small commits separating contracts, DSP/control, examples and verification.
Do not casually refresh goldens; explain every intentional audible change.

## Acceptance: technical gate

- `./build.sh --web`, `./tests/run_tests.sh` and applicable actual Chromium suites
  pass at the submitted revision, including six-track and editor regressions.
- Normal panel and browser controls select8/12/24, display release inms, and restore
  both correctly after save/reload; verify boundary and midrange values, locks,
  motion/MIDI control, legacy semantic fixtures and failed/full saves.
- Direct monitoring off still feeds carrier; missing carrier produces correct
  silence; routing across T1–T6, Freeze/unfreeze, mode switches, retrigger, one-shot
  note-off, STOP/panic and load/reset are bounded and do not leak/stick.
- Retain serial/worker equivalence, worker failure recovery without double render,
  CPU2-off and vocoder-disabled behavior, finite output and native/WASM comparison.
- Exercise six active tracks with eight admitted voices plus vocoder, including
  competing synth/FX/sequence activity. Report12vs24 timings for the same workload,
  block size, sample rate, build and machine; include repeated high-percentile/max
  observations, state/static memory deltas and browser audio failures if any.
- Browser/host timing is not FM-1 deadline proof. No target-safe claim without
  separately authorized target evidence. Report hardware feasibility as unresolved
  if no valid target budget exists; retain lower-cost modes.
- All audition loaders preserve confirmation/cancel, saved slots, autosave precedence,
  stopped load and mobile usability. No unexpected public/private asset exposure.

## Acceptance: musical gate

Doug auditions the same complete phrase in12/24 modes at matched levels and judges
intelligibility and desired electro character. Assess100% wet and light dry blend
separately; dry blend is a legitimate preset, not proof of wet-only intelligibility.
Then audition the full arrangement and approve a candidate demo or record what
still misses. Technical PASS does not imply musical approval; leave that gate open
when Doug has not listened. Keep the existing demo available throughout.

## Explicitly deferred

36+ bands; new voiced/unvoiced classifier/noise synthesis; formant shifting;
carrier spectral normalization/Enhance clone; new oscillator engine; hardware
patch-dump tooling; broad sample-format rewrite; live audio monitoring installation;
new track/voice budgets; physical-device rollout; unrelated UI or storage redesign.

## Implementation-agent final handoff checklist

Return PR URL and exact pushed head/base; scoped changes and compatibility decision;
commands/results with host/browser/target evidence separated; settings and artifact
links; exact local launch/load/listen recipe; measured cost and remaining risks;
and the status of Doug's listening gate. Keep the PR reviewable, unmerged and
undeployed. Do not claim completion of listening or hardware checks not performed.

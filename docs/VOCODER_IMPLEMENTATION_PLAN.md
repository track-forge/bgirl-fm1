# BGIRL sample-fed vocoder — implementation plan

Date: 2026-11-15. **Planning stub only: no implementation or generated assets in this PR.**
Baseline inspected: `45590dfe97d11dfaf022cfd002266ac7218481b9`, merged Felucca-based BGIRL.

## Goal

A sequenced speech instrument that makes another synth track talk. Reuse Felucca
SAMPLE/SLICE, sequencing, synth engines, parameter pages, effects and persistence.
Add one vocoder processor on a speech track with another track supplying actual
carrier audio. Two tracks, not a separate third processing track. No new rebase,
parallel sequencer or general-purpose modular audio graph.

Doug accepted the behavior below and explicitly requires creating a reference
speech bank as part of implementation. SVC-350-inspired character is the goal,
not an independently verified circuit-accurate Roland clone.

## Scope and repository instructions

Read AGENTS.md, README.md, docs/UPSTREAM.md, docs/BUILD_STATUS.md and the historical
docs/VOICE_SPIKE.md. Active source is `felucca/`; the prototype at `4cc1e11` is
reference material, not the shipping architecture. Its DSP owns a hard-coded
sample and saw oscillator; it is not an external-input drop-in processor.

This is the follow-up milestone explicitly deferred by BGIRL_BASELINE_PLAN.md.
When implementing this plan, the old baseline-only prohibition on vocoder work
is superseded for this scoped feature. Preserve all other safeguards: pinned
engine/BSP dependencies, attribution, normal controls, no render-time allocation,
file/network I/O, logging or unbounded waits. Narrow backward-compatible storage
extensions needed here are allowed; unrelated format/protocol rewrites are not.
Update AGENTS.md to reflect the new milestone in the implementation PR.

Do not merge, flash hardware, publish releases/sites, activate installer workflows
or change device identity. Browser/host delivery first; target/device work remains
an explicitly separate acceptance phase. Do not claim hardware success from WASM.

## Accepted v1 behavior

- Keys/steps select phrases/slices at original playback pitch. Carrier notes supply
  melody independently; changing carrier notes does not restart speech.
- One speech phrase at a time. Retrigger replaces/restarts it with a short anti-click
  transition. Default one-shot: releasing its key does not truncate the word.
  Gated/looped modes are optional reuse, not required expansion.
- Tap the carrier after its sound shaping/insert, before mixer level, pan and output
  suppression. Provide independent carrier dry-output on/off; turning this off must
  not stop its internal render/feed. Define ordinary mute/solo separately.
- Freeze captures band envelopes, not an audio loop. Speech playback and analysis
  continue underneath; carrier notes remain playable. Unfreeze transitions smoothly
  to current analysis. Stop/panic clears Freeze and speech playback; preserve normal
  downstream effect-tail behavior unless explicitly documented otherwise.
- Raw-speech/wet-vocoder balance is separate from dry-carrier monitoring. No accidental
  raw-speech duplication. Vocoded output follows the speech track's effects, level,
  pan and sends. Leave its existing insert available for post-vocoder chorus.
- Projects restore phrase selection, carrier routing and settings. Save Freeze
  configuration, never a captured vowel; load unfrozen. Preserve old projects/presets.
- One vocoder instance initially; forbid self-routing/feedback. Support both a borrowed
  musical carrier part and a dedicated carrier-melody track.

Before coding, write down engineering contracts for: eligible carrier engines;
invalid/deleted/replaced source handling; exact mute/solo behavior; phrase-to-key
mapping; sound preset behavior for project-specific carrier references; source
changes while playing; Freeze with no prior signal; bypass/disable and transport
behavior. Choose conservative deterministic behavior and expose missing sources
clearly. These are bounded engineering choices, not permission to change the accepted
behavior. Explain any product tradeoff that requires Doug's decision.

## Reference speech bank — required work

Use Moltazine/Crucible to generate a small original bank off-device. Discover the
current CLI skill/workflows at runtime; do not invent a TTS endpoint/model. Generate
only a bounded development bank, not a large production library. If access, credits,
workflow or voice rights block generation, report the concrete blocker; do not silently
substitute pre-vocoded hooks or leave the asset task out of the deliverable.

Suggested initial size: 3–5 short dry phrases, including clear consonants and a
sustained vowel. Example texts, editable by the engineer: “enter the circle”,
“bring it back”, “your move”, “freeze”. Use an available original/licensed voice,
not an imitation of an identifiable performer. The final crew vocabulary is not a gate.

Deliver:

- Original dry audio and transcripts; workflow/model/voice identifiers and generation
  settings/provenance, rights basis and attribution where applicable.
- Reproducible offline trim/normalization/resampling/encoding pipeline and runtime
  assets with stable IDs, lengths, rates, checksums and measured byte sizes.
- A manifest separating distributable provenance from private job receipts; no tokens,
  signed URLs or other credentials committed.
- Small curated bank bundled with the build; no service/network dependency at build
  or runtime. Ordinary SAMPLE/SLICE user slots do not automatically package a new bank.
- Dry and processed listening examples using the same phrases, plus a playable demo
  showing independent speech rhythm and carrier melody.

Use SAMPLE/SLICE decoding and event semantics where practical. Do not pitch speech
accidentally because note numbers are selecting phrases. Ordinary keyboard zones,
SLICE source selection and bank mapping need explicit integration.

## DSP and UI scope

Compare 8 and 12 analysis/synthesis bands; select by intelligibility and cost rather
than assuming the historical 12-band spike proves device feasibility. Engineer chooses
filter design, coefficients, fixed-point/float representation and safe gain/headroom.

Include character-band shaping, Freeze, input trims as needed, speech/wet balance,
carrier selector and dry monitoring. DSP band count need not equal visible slider
count: grouped controls or two four-knob pages are reasonable. Reuse parameter/page
conventions and document exact ranges/defaults. Separate live Freeze state from its
saved configuration. Reuse existing carrier distortion for harmonics and existing
chorus initially. Consider controlled high-passed speech for consonants; verify its
behavior with absent carrier/Freeze. Existing mono chorus is not stereo BBD ensemble.
Custom stereo ensemble, guitar compressor, external microphone/USB input, real-time
stretching and a hardware H/M/L pad are not first-delivery requirements.

## Routing implementation recommendation

Keep Felucca's scheduler. Capture selected inputs once, then process after the existing
join and before shared FX:

```text
block-local control/routing snapshot
  -> existing track rendering on available cores
     -> retain carrier block
     -> retain speech block; defer speech-track output processing
  -> existing core join
  -> vocoder once
  -> speech track post-vocoder effects / level / pan / sends
  -> existing shared buses / master
```

Do not merely add the vocoder to the master or render source tracks twice. Preserve
block-local modulation values when deferring downstream work. Each captured input has
one writer; readers consume it only after completion. Clear/mark absent input every
block. Freeze retains envelopes deliberately, never stale input buffers. The inactive
track early exit must not kill a held vowel when its speech sample ends.

At current `CTL=32`, two mono `int32_t[CTL]` input buffers are 256 bytes before alignment,
metadata, work/output buffers or DSP state. This is arithmetic, not a RAM-fit claim.
Respect internal headroom and the platform's shared-memory visibility contract.

Worker recovery deserves an explicit audit: submission declined before execution can
run locally; a job interrupted after starting may already have advanced state. Do not
blindly replay it or consume partial inputs. Reclaim ownership safely, distinguish
these cases in tests and document any necessary scoped change to inherited recovery.

### Pinned borrowing references (source-inspected, not locally validated)

- **Optimist** `66c4cfb82967b7e7b035edf682a4f5928ce3c7eb`:
  [track capture](https://github.com/w0ts/optimist/blob/66c4cfb82967b7e7b035edf682a4f5928ce3c7eb/firmware/src/fx/fx.c#L2747),
  [dual-core pipeline](https://github.com/w0ts/optimist/blob/66c4cfb82967b7e7b035edf682a4f5928ce3c7eb/firmware/src/system/dual.c#L972).
  Borrow retained per-track capture pattern, NOT its post-level/mute/duck semantics.
- **Melodee** `165d177a425944f3d30b0b669255a6ed0050c3a5`:
  [worker ownership/failure](https://github.com/keremimo/melodee/blob/165d177a425944f3d30b0b669255a6ed0050c3a5/firmware/src/audio_worker.h),
  [FX staging](https://github.com/keremimo/melodee/blob/165d177a425944f3d30b0b669255a6ed0050c3a5/firmware/src/fx.c#L576).
  Borrow explicit prepare/submit/join and interrupted-job lessons, not its entire rack.
- **SLOOP** `bac9c05ea7627449405aa10e48da21d75b6c8c56`:
  [mixer](https://github.com/isod89/sloop-fm1/blob/bac9c05ea7627449405aa10e48da21d75b6c8c56/firmware/src/fx.c#L1185).
  Similar fork/join, largely already inherited. Exact selectable vocoder routing was
  not established. Main-loop scratch is not automatically safe for concurrent audio.
- **fm1-sampler** `e4aa8fa01d905050db07289c4aca394c70c2eaa0`:
  [architecture](https://github.com/ferluht/fm1-sampler/blob/e4aa8fa01d905050db07289c4aca394c70c2eaa0/README.md).
  Phone-hosted sidechain ducking is a level-envelope problem, not an embedded waveform
  feed. Host-engine copying rights were not established; low-priority reference.

Preserve exact component licenses/attribution on any transplant; do not assume a repo
license covers every file. No candidate is a verified drop-in routing module.

## Staged execution and acceptance

### 1. Contracts and assets

Inspect current main and actual build flags; record exact dependency pins. Document
contracts above, generate/reference the speech bank, prove reproducible packaging and
normal dry playback through the existing sequencer. No new synth/sequencer framework.

### 2. First playable and listening checkpoint

Integrate bounded capture, vocoder, monitoring, retrigger and Freeze. Produce comparable
8/12-band renders and a runnable browser demo. Compare dry speech with processed speech,
consonant treatment and existing chorus. Ask Doug to audition; automated signal checks
are not intelligibility approval. Continue independent tests/docs while feedback is
pending, but do not label sound curation approved without a human response.

### 3. UI, persistence and regression proof

Finish normal four-knob pages and stable parameter IDs, project/preset semantics and
additive compatible storage. Extend existing tests, including `felucca/tests/dualcore_test.c`:

- Carrier/modulator on either core, all eligible track orders, single/dual-core output
  equivalence and race detection where supported; absent/rejected/interrupted workers.
- Each source rendered once; no stale capture on silence, source changes or failure.
- One-shot/retrigger/pitch independence; held-envelope operation after sample end;
  smooth unfreeze and explicit stop/panic/reload reset.
- Carrier-only monitoring; specified mute/solo; no dry-speech duplication; voice output
  still follows insert, track level/pan and effect sends.
- Projects round-trip bank IDs, routing and controls; old saves still load; sound
  presets obey the documented routing rule; loads never restore a frozen vowel.
- Feature disabled preserves existing behavior/audio; do not casually regenerate
  goldens. Bounded output/headroom, silence/release and no real-time allocations/I/O.
- Run relevant upstream host tests, WASM build and actual Chromium/panel acceptance.
  Document omitted device-dependent suites, exact commands/results and limitations.

### 4. Review hand-off and separate target gate

Deliver scoped reviewable implementation PR(s), source and bank provenance, reproducible
build/tests, listening renders, demo instructions, accepted/unverified behavior and RAM/
CPU estimates clearly separated from measurements. Do not merge or deploy.

Physical-device compatibility, linker RAM/flash fit, real render deadlines/xruns,
USB/MIDI and worst-case concurrent-track load remain a separately authorized hardware
phase. Browser success is a completed browser milestone, not a device-ready release.

## Local context (optional, not required to execute this plan)

Notebook: `obsidian-agent-tasks/Creative/Music/BGIRL FM-1 - Vocoder Design.md`.
This repository plan contains the implementation scope so an engineer without notebook
access is not blocked. Notebook updates should remain aligned with implementation
findings; the present PR intentionally contains only this plan document.

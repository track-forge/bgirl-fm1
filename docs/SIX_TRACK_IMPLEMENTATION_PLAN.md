# Six full tracks — implementation plan

Date: 2026-11-17. Status: **implemented and host/browser verified; draft review pending**.
Baseline: `4a5f1fbc414a32cd907b2b215060dca503004e6c` (main, Signature bank merged).
Work branch: `plan/six-tracks`. Continue this branch and PR for implementation.

## Product contract and explicit scope override

Expand BGIRL from four to **six ordinary, fully editable tracks**, retaining the
shared **eight sounding voices**, 64 steps per track, existing engines, Signature
sounds, vocal samples/vocoder, arp dance, song chain, MIDI and browser workflow.
Tracks 5/6 are not MIDI-only or restricted “light” tracks. No additional voice
budget, new engine, pattern-pool architecture or upstream wholesale merge.

Doug explicitly clarified: **there is not a single important existing save;
legacy save migration is unnecessary.** This supersedes the old-save preservation
requirements in AGENTS.md and earlier plans for this feature's affected formats.
Use a clean-break, versioned format. Do not implement legacy migration, a converter,
or dual writers. Safely identify/refuse incompatible data; document how to reset
old local emulator data or explicitly overwrite an old slot. Do not automatically
erase unrelated presets, samples, banks or settings. Unaffected formats can remain.

The original stub changed only this plan. Implementation continues on this branch; an agent may implement,
test locally, commit and push to this same PR. No merge, Pages/public deployment,
release, device build or flashing is part of the handoff. Other AGENTS safeguards
still apply. This is currently a host/browser instrument, not verified hardware firmware.

## Findings and pinned reuse references

All code paths below are relative to `felucca/firmware/src/` unless stated otherwise.

### BGIRL baseline

- `core.h`: `NPART=4`, `NTRK=NPART`, `NVOICE=8`, `NSTEP=64`.
  Much runtime state already scales with NTRK, but track_t contains per-part voice
  storage; an unchanged active-voice budget does NOT imply unchanged RAM.
- `project.c`: fixed 3,840-byte project payload; four FM6 patches, chain, motion,
  MIDI channels and optional vocal-bank IDs share it. Frozen legacy structs also
  use NTRK, so simply changing the constant breaks their layouts/assertions.
- `storage.c`: 4,096-byte sectors, 256-byte payload offset, 3,840-byte capacity.
  Existing A/B copies, CRCs and header-written-last commit protocol are worth retaining.
- `core.h` motion_event_t: one byte combines four tracks ×64 steps. Six tracks
  require more than eight bits. Redesign the encoding rather than aliasing T5 to T1.
- `midi_learn.c` / `midi_control.c`: learned mappings use two track bits
  (`(e >> 7) & 3u`); the adjacent parameter-code field must move or be redesigned.
- `project.c`: MIDI routes packed into three bytes with four 5-bit channel fields;
  six fields need a new layout. `chain_config_t` has asserted serialized sizes.
- `ui.c` home_trk uses `trk[k & 3u]`; mixer/home levels, track graph, menus, quick
  controls and editor/browser assumptions need explicit treatment.
- `song_chain.c`: multiple cached sources hold NTRK × NSTEP patterns. Include those
  caches, undo, project scratch, MIDI ownership, engine state and worker buffers in RAM accounting.

### JIANT: closest compact-save reference

Pinned source: [`juanjiant-bit/JIANT @ da8baa1`](https://github.com/juanjiant-bit/JIANT/tree/da8baa1eed7c4e75f1ed9bcf433797b3aeace107).

- [`core.h`](https://github.com/juanjiant-bit/JIANT/blob/da8baa1eed7c4e75f1ed9bcf433797b3aeace107/firmware/src/core.h): six parts, eight voices.
- [`project.c`](https://github.com/juanjiant-bit/JIANT/blob/da8baa1eed7c4e75f1ed9bcf433797b3aeace107/firmware/src/project.c): `proj_pack`, `proj_unpack_c`,
  `proj_step_size`, `proj_fm_kept`, size calculation and `proj_full`. Sparse
  parameter and step maps; omit defaults; conditional FM6 payloads; bounded packing.
- [`storage.c`](https://github.com/juanjiant-bit/JIANT/blob/da8baa1eed7c4e75f1ed9bcf433797b3aeace107/firmware/src/storage.c): retains a 3,840-byte object and A/B commits.

**Important:** its six-track codec can fail with **SECTION FULL** on dense content.
It is evidence for a useful implementation pattern, not proof all BGIRL states fit.
Its FUNC magic collides with BGIRL's existing vocal-save magic: do not copy format IDs.
Its engine/default tables, extended step fields and memory placement differ from ours.
Inspect its tests and originating six-track commits before porting a bounded subset.

### Skiff: six-track UX and clean-break format reference

Correct repository: [`illuminus5/skiff @ b257ddf`](https://github.com/illuminus5/skiff/tree/b257ddf6a15a21f6c5956abc9c19c0571daa8e15).
Not the unrelated `chancehouston/skiff-fm1` research repository.

- [`core.h`](https://github.com/illuminus5/skiff/blob/b257ddf6a15a21f6c5956abc9c19c0571daa8e15/firmware/src/core.h): NTRK=6, but T5/T6 described as light;
  different voice budgets and architecture. Do not import those restrictions/budgets.
- [`ui.c`](https://github.com/illuminus5/skiff/blob/b257ddf6a15a21f6c5956abc9c19c0571daa8e15/firmware/src/ui.c) and sibling UI modules: inspect selection/mixer/overview conventions
  and adapt only controls that fit BGIRL's physical panel and browser.
- [`project.c`](https://github.com/illuminus5/skiff/blob/b257ddf6a15a21f6c5956abc9c19c0571daa8e15/firmware/src/project.c): normal firmware reads SKF3/SKF4; old formats show
  OLD FORMAT; legacy conversion is separate. Borrow explicit rejection semantics,
  not the converter. Its pattern-pool/song architecture is not a drop-in save codec.

Both references declare GPL-3.0-only firmware. Verify each borrowed file's notices,
retain copyright/SPDX, and record repository, full SHA, file/function, adaptation and
license in docs/UPSTREAM.md. Reuse algorithms/tests with attribution where suitable;
do not copy installers, flash addresses, assets, branding or dependency pins.
These are source observations, not independently reproduced hardware proof.

## Implementation gates and commit sequence

### 0. Baseline and inventory

Read AGENTS.md, README.md, BGIRL_BASELINE_PLAN.md, UPSTREAM.md and current verification
docs. Reconcile current main if it has moved; preserve concurrent feature work.
Initialize pinned recursive submodules without advancing them. Run `./build.sh --web`
and `./tests/run_tests.sh` before edits; retain logs and exact revisions/tool versions.
Use the documented browser harness for an initial working reference.

Inventory track assumptions across runtime, codecs, settings, SysEx/editor, web UI,
asset/demo generators and tests. Classify literal 4s: track count is NOT the four
knobs, four-note step chord capacity, four project slots, timing fields or DSP widths.
Record the actual extension sites; never do a global numeric replacement.

### 1. Decide and prove the save codec first

Prefer preserving the existing flash map, four project slots and A/B storage layer.
Prototype a BGIRL-specific versioned compact codec using JIANT's sparse encoding
ideas. Use explicit field widths and bounds, not native struct memcpy. New magic
must differ from BGIRL and reference formats; version/count validation precedes mutation.

Persist all six tracks' parameters, engine/preset identity, full FM6 OWN edits,
64-step data (including dormant fields that affect later editing), chain, automation,
locks, MIDI routes, vocal IDs and vocoder state, names and applicable global state.
Parameter-default omission must use version-stable codec defaults, not silently
change saved sound when future preset or parameter defaults change. Round trips must
preserve stored/editable semantics, not just the currently audible notes.

Produce a byte-budget table for empty, typical electro, six-FM6, maximum automation,
and adversarial dense six-track/64-step/four-note patterns. Include decoder scratch,
serialized motion changes and every extension. Preserve the existing 128-event shared
motion capacity unless a separately justified change is required.

**Capacity decision:** a compressed one-sector MVP may legitimately report PROJECT
FULL, following JIANT, provided typical musical fixtures fit and failures are explicit
and lossless. Document measured capacity and worst cases before settling this route.
Do not promise arbitrary dense states fit. On overflow, keep the in-memory song and
last valid saved copy; do not truncate tracks, notes, locks, names or FM6 data. Manual
save, autosave, undo and editor export all need honest failure behavior; avoid retry
storms or showing SAVED after failure. UI must expose a useful occupancy/full indication.

If this is impractically restrictive, record a larger-object alternative and its
flash-region/slot/autosave/voice-bank implications before implementation. Do not steal
sample/OTA/bank regions or silently reduce slots. A physical storage-layout change
requires a separate reviewed design; continue the host/browser spike and report that
specific blocker rather than claim device-ready storage.

Old project/autosave images must be rejected deterministically before applying state.
On startup, fall back to fresh stopped defaults with an understandable notice/reset
path. Explicit overwrite of an old slot is allowed; no migration or backups required
by this feature. Update legacy-format tests deliberately to rejection tests for affected
formats; retain malformed-input and atomic-write coverage. Settings/MIDI-learn records
whose layouts change also need versioning/reset rules. Never reinterpret two-bit tracks
as new six-track bindings.

### 2. Runtime, addressing and resource accounting

Set six tracks; keep eight active voices. Expand track bounds, arrays, masks,
selection, routing, panic, note ownership and per-track initialization. Give T5/T6
normal usable defaults with empty sequences; preserve T1–T4 sound/demo intent.

Replace packed motion addressing with explicit helpers covering tracks 0..5 and
steps 0..63. Update all producers, consumers, wire records, undo and validation.
Expand MIDI-learn track encoding and editor channel representation consistently.
Audit song chain, clock source, recording, per-track locks, copy/paste, load/save,
FX/vocoder owner/modulator selection and render-worker indexing on tracks 5/6.
Maintain vocoder single-owner/no-self-route rules and sample-bank identity checks.

Measure static state and runtime scratch before/after: track_t ×6, per-part voices,
chain caches, project/undo buffers, effects/engine histories and single/dual-core
snapshots. No allocation, logging, file I/O or blocking in the audio render path.
Exercise admission/stealing under the fixed voice budget; no out-of-range indexes,
starved note-off handling or silent relaxation of overload protection.

### 3. Panel, browser and editor parity

ALGORITHM/ordinary selection reaches all six tracks, wraps correctly and releases
held notes safely. Six-track overview must show selection, mute, recording and playhead.
Use two clearly labeled four-knob banks for level/mixer controls (T1–T4, then T5–T6
with two inactive cells) as the default design. Choose a non-conflicting existing
navigation gesture after inspecting UI events; document and test the exact gesture.
Inactive cells must not edit hidden tracks. Ordinary sound-edit pages retain four
parameter knobs, not six. Follow selection between banks without parameter jumps.

Expose all six tracks through the existing browser controls and editor protocol,
including MIDI channels, sound loading, sequencer, chain and automation. Advertise
new counts/capabilities; reject incompatible fixed-four-track clients or packets
cleanly rather than misparse. Update backup/export/import and reset UI as applicable.
Do not require a separate UI-only workaround for T5/T6.

### 4. Verification and evidence

Run focused checks during development, then the root web build/full host suite and
actual Chromium acceptance. Extend existing harnesses rather than creating a parallel
sequencer or persistence implementation. Required cases:

- Six independent sounds/engines/64-step patterns; all six audible in a controlled
  sparse fixture, with T5/T6 usable for FM6, DRUM and vocal/vocoder routing.
- Selection/wrap, mixer bank switching, mute/solo/record/erase/copy, chain transitions,
  live keyboard/MIDI on channels 1–6, MIDI learn, held-note switch and panic.
- Motion on T5/T6 step 63; no T1/T2 aliasing. Chain/source indexes, bounds and
  invalid track 6+ inputs rejected without partial state changes.
- New-format exact semantic round trip, fresh-instance autosave restore, settings
  reset/version handling, editor backup/import and deterministic old-save rejection.
- Dense/full codec boundary, malformed/truncated data, bad version/count/CRC, failed
  packing and interrupted writes: old committed copy and live song remain intact.
- Eight-voice overload/stealing with six tracks and long tails; silence after stop/
  release as appropriate; bounded output, worker failure/recovery and no duplicate render.
- Existing Signature drums/choke, FM6 sounds, vocal banks, vocoder, arp dance and
  disabled-feature behavior retained. Four-track reference renders with T5/T6 idle
  should stay identical where semantics are unchanged. Explain any golden changes;
  no blanket regeneration to hide regressions.
- Real browser audio after gesture, all six selectable/editable, save/reload, mobile
  layout/screenshots, zero page errors. Produce a short six-track electro audition
  and a dense stress fixture. Automated energy checks are not human musical approval.

Report host RAM/timing/build size separately from target feasibility. This handoff
prohibits device builds/flash; physical RAM/deadline/xrun and flash compatibility remain
unverified until a separately authorized hardware milestone. Source-level memory audit
and host results are useful, but do not substitute for it.

### 5. Close out on this PR

Keep commits reviewable: codec/contracts; track/addressing runtime; controls/protocol;
acceptance/docs (adjust order when dependencies demand it). Update this plan with the
chosen codec, actual capacity and any departures. Update README/provenance and add a
six-track verification report with commands, revisions, results, local audition paths
and remaining blockers. Run `git diff --check`; push the same PR, report CI honestly.
Do not merge, publish previews, modify running audition services or flash hardware.

## Copy/paste implementation handoff

> Continue the draft six-track planning PR in track-forge/bgirl-fm1 on branch
> plan/six-tracks. Read docs/SIX_TRACK_IMPLEMENTATION_PLAN.md and repository AGENTS.md.
> Implement the plan in this same PR, using an isolated checkout/worktree and preserving
> other work. Doug explicitly waived legacy-save migration: clean-break formats and
> safe rejection/reset are wanted, not converters or compatibility engineering.
> Start with baseline tests, then the save-capacity spike. Study the pinned JIANT
> compact codec and Skiff six-track UI/old-format rejection; selectively adapt useful
> code/tests with attribution, not entire architectures. Deliver six full tracks,
> retain the eight-voice budget, preserve BGIRL sound/vocal/vocoder features, and verify
> host + actual browser operation, codec limits and atomic-save failure behavior.
> Keep an explicit PROJECT FULL path if the documented compact-format capacity requires
> it; never silently drop state. No merge, deployment, device build or flashing.
> Push implementation commits to this PR and return exact tests, revision, audition
> recipe, capacity/RAM findings and remaining device-only limitations.

## Implemented choice and departures

See [capacity decision](SIX_TRACK_CODEC_SPIKE.md) and
[verification report](SIX_TRACK_VERIFICATION.md). B6S1 keeps the existing storage
map and rejects old project formats, retaining the explicit PROJECT FULL path.
Frozen parameter/INIT defaults avoid future sound changes from sparse omissions.
Six tracks use nine-bit motion places, 16-entry/three-bit-track MIDI learn,
selection-following level banks and a dedicated six-row CHANS menu. Editor
namespace F6 rejects fixed-four clients. Undo remains native and independent of
packing. Ordinary MIXER knobs retain selected-track LEVEL/PAN/REV/MUTE behavior;
GLO/HOME LEVELS supply the four-knob banks. No extra navigation chord was needed:
ALGORITHM selection is the bank gesture. No light tracks or hardware work.

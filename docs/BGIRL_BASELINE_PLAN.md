# BGIRL baseline — Felucca foundation

Date: 2026-11-15. Status: **approved direction; implementation not started**.
This document is a plan, not evidence of a migrated build.

> **Felucca behavior intact, BGIRL electro defaults, working browser build.**

## Decision and scope

Make BGIRL an opinionated electro edition of [Felucca](https://github.com/hugelton/Felucca),
then add the distinctive voice and b-girl features in subsequent work. Inherit the
instrument application, not merely its hardware support. Preserve normal track
selection, sequencing, editing, MIDI/clock, effects, presets and project behavior;
do not rebuild these in our current minimal app.

The initial prototype was merged in [PR #1](https://github.com/track-forge/bgirl-fm1/pull/1).
Its merge checkpoint is `4cc1e11e50ffb1d2a06a50547780b9e828a8b15f`. It proves a
host/browser development loop and contains useful DSP/voice experiments, but is
not the application architecture for this next milestone.

This plan supersedes the architectural direction of `INITIAL_BUILD_PLAN.md` for
the next implementation. That document and `BUILD_STATUS.md` remain historical
evidence. `AGENTS.md` now points to this migration direction while distinguishing
the existing prototype from the intended baseline. Keep it current with the actual
layout during implementation, retaining real-time audio, provenance and verification safeguards.
Do not mistake the old base-pin rule for a requirement to port Felucca onto
fm1-base. Use Felucca's own supported dependency/build layout as the starting point.

**This planning PR adds this document and updates `AGENTS.md`.** No source,
dependency, defaults, workflow, license or notebook changes are part of it.

## What we inherit

Upstream's [feature documentation](https://github.com/hugelton/Felucca/wiki/Features)
currently describes four tracks selected with ALGORITHM, per-track engines,
sequencing/recording/parameter locks, projects, presets, performance effects,
USB/TRS MIDI and clock, and browser tooling. These are the baseline to retain,
not new BGIRL implementation tasks. Check the exact selected revision: the wiki
can move independently of releases and is not a pinned specification.

The README observed during planning names stable release **1.8.2**, GPL-3.0-only
firmware, MIT Felucca-Engines and MPL-2.0 felucca-bsp. Verify the selected tag,
resolved commit and recursive dependency revisions before implementation. Browser
demo/editor code may live in separate upstream repositories; inspect their build
instructions and licenses rather than assuming our existing browser wrapper fits.

## Musical defaults, not restrictions

Proposed first-boot/new-project setup:

| Track | Role | Baseline treatment |
| --- | --- | --- |
| 1 | Electro drums | Curated existing DRUM-engine kit: kick, snare/clap, hats and metallic punctuation |
| 2 | Rubbery mono bass | Existing engine/preset tuned for a short, punchy electro bass |
| 3 | Metallic stab / icy lead | Existing engine/preset with space for rhythmic call-and-response |
| 4 | Robot reply | Existing sample/engine capability if suitable original material is already available; otherwise a clearly named synth placeholder |

Exact engines and parameter values are sound-design choices to audition, not
settled specifications. Keep ordinary engine/preset selection available on every
track. Respect upstream voice sharing and other resource limits; four tracks do
not imply four independent polyphonic budgets.

- Provide one small, loadable electro demo project/pattern using existing sequencing.
  Boot stopped; make the blank electro starting template separately accessible.
- Apply defaults only at fresh initialization or an explicit new/reset operation.
  Preserve restored music, saved projects and user presets. Do not silently reset
  a user's work on every boot or overwrite upstream factory content unnecessarily.
- Reuse existing save/project machinery. Document whether defaults live in code,
  preset data or a bundled project, and why that is the smallest maintainable change.
- Use original or appropriately licensed material with provenance. No new paid
  generation, commercial-record samples or external service dependency is needed.

## Implementation sequence

### 1. Preserve the prototype and establish upstream provenance

1. Read this plan, repository instructions and upstream `BUILDING.md`, licensing,
   dependency and contribution documentation. Check current Git status and branches.
2. Work on a dedicated migration branch/worktree; do not rewrite or force-push main.
   Keep PR #1 and its checkpoint reachable. A local archival ref is optional;
   no published tag/release is required.
3. Select and pin a stable Felucca revision and recursive dependencies. Prefer an
   upstream remote plus an ancestry-preserving import/merge so later upstream
   comparisons and updates are practical. Inspect for overlapping paths before
   merging unrelated histories. Do not squash away attribution or import unrelated
   generated artifacts. Record the actual method and exact revisions in provenance.
4. Keep the existing repository URL/issues/history. GitHub fork-network membership
   is not required. Avoid layering the entire old app over Felucca or maintaining
   two active sequencers/UI state models.
5. Inventory reusable prototype assets: `app/dsp/vocoder.*`, original speech and
   provenance, voice render harness and findings. Preserve them in history or a
   clearly non-shipping experiment area; do not integrate the vocoder in this milestone.

**Gate:** reproducible source/dependency selection, preserved prototype checkpoint,
clear active source tree and license/attribution inventory.

### 2. Reproduce unmodified Felucca first

1. Follow the selected revision's own build/test instructions, including the browser
   path and any separately maintained emulator/editor components.
2. Establish an upstream control build before applying BGIRL changes. Record tools,
   commands and results. Resolve baseline build blockers before adding features.
3. Exercise browser audio after user interaction, input, ALGORITHM track selection,
   transport, sequencing and save/load where the browser supports them.
4. Keep release/site publishing disabled. Do not blindly activate imported workflows,
   upstream deployment destinations, installer links or release credentials.

**Gate:** local upstream browser build runs, with a documented comparison baseline.
If upstream browser assembly is not reproducible, report that blocker explicitly;
do not substitute our old app and call it Felucca parity.

### 3. Apply the small BGIRL delta

1. Add the electro defaults and demo project above through upstream's existing models.
2. Make minimal visible BGIRL naming/palette/about changes; retain upstream credit.
   Do not rename internal symbols broadly or remap familiar controls for branding.
3. Keep user-data formats and protocols compatible where practical. Document any
   necessary changes; do not imply compatibility with the old prototype's `BGR1`
   record format or flash layout. No automatic device-data migration in this milestone.
4. Replace obsolete build wrappers/instructions as needed, update `AGENTS.md`, README,
   provenance and build-status documentation to describe the real architecture.
5. Preserve applicable GPL-3.0-only obligations and component notices, including
   notices for assets/fonts/browser components. Do not present the combined Felucca
   derivative as MPL-only because the previous scaffold used fm1-base.

**Gate:** recognizable BGIRL startup palette, with Felucca's normal behavior intact
and a small, explainable delta against the pinned upstream.

### 4. Verify and hand back a reviewable migration PR

Run upstream's applicable tests and add focused regressions for our changes, not
a parallel imitation test suite. Keep existing prototype tests only where they
remain meaningful; document retired assumptions rather than silently deleting failures.

| Area | Required evidence |
| --- | --- |
| Build | Exact upstream/dependency/tool revisions and repeatable local browser commands |
| Track/input | ALGORITHM changes selected track; keys/knobs affect intended track; releases/panic leave no stuck notes |
| Transport/sequence | Play/stop/restart and editing work through the inherited sequencer; no replacement fixed-loop engine |
| Defaults | Fresh state has the four documented roles; blank template and demo are available; subsequent restore preserves edits |
| Persistence | Save/load round trip for a changed sound and project through supported browser/host paths |
| Audio | Browser audio works; audition the electro roles and demo, distinguish automated signal checks from human listening |
| MIDI/clock | Preserve upstream routing/clock implementation and run available regressions; test browser Web MIDI if available, record unsupported paths |
| Visuals | Desktop/mobile screenshots, legible branding and usable inherited controls; no dance animation required |
| Source hygiene | License/provenance retained, no accidental deployment workflows or secrets, no unrelated changes |

Record observed pass/fail/not-tested for each area. Browser tests are **not** proof
of physical USB sync, flash/RAM fit or device render deadlines. Hardware USB/TRS,
audio/MIDI latency and worst-case CPU/xruns remain a separate physical acceptance
pass. An optional device compile is build evidence only, never permission to flash.

Deliver the implementation PR, exact tested commit, commands/results, local demo
URL/start instructions and a short demo recipe: boot fresh, play groove, select
each track with ALGORITHM, edit a sound, save/reload, stop and release all notes.
Include remaining blockers and the upstream-to-BGIRL change summary. Do not merge,
flash hardware, publish tags/releases/sites or change GitHub repository identity.

## Explicit follow-ups — not baseline blockers

- Sample-fed playable vocoder integrated into Felucca's track/engine model, informed
  by the existing `docs/VOICE_SPIKE.md`; compare intelligibility and measured target
  headroom before deciding live DSP versus a labeled pre-rendered sample fallback.
- Dedicated BPM-linked silhouette dance page, preserving other instrument pages:
  [issue #2](https://github.com/track-forge/bgirl-fm1/issues/2).
- Richer BGIRL artwork, crew voices, larger electro banks and custom performance moves.
- Device identity/packaging/storage review, approved physical testing and publication.

## Local context and handoff

Workspace root: `/home/dev/.openclaw/workspace`.

- Repository: `codebase/bgirl-fm1` (inspect status before using; prefer a new worktree).
- Musical north star: `obsidian-agent-tasks/Creative/Music/FM-1 Classic Electro - Concept Notebook.md`.
- Historical setup rationale: `obsidian-agent-tasks/Creative/Music/FM-1 Classic Electro - Setup and Plan.md`.
- Future visuals: `obsidian-agent-tasks/Creative/Music/BGIRL FM-1 - Silhouette Dance Page.md`.
- Hardware context: `obsidian-agent-tasks/Creative/Music/M-VAVE FM-1 Pocket Synth.md`.
- Prototype evidence: `docs/BUILD_STATUS.md`, `docs/VOICE_SPIKE.md`, `docs/UPSTREAM.md`.

Notebook setup/status sections still describe the earlier fm1-base direction and
may predate PR #1's merge. Treat this plan as the new baseline scope, the concept
note as musical guidance, and the old setup note as historical context. Update
those status sections when implementing; commit/push scoped notebook changes under
the workspace's notebook convention. Local notes are supplementary: this document
contains the complete baseline scope for contributors without vault access.

### Copy/paste implementation brief

> Implement `docs/BGIRL_BASELINE_PLAN.md` in `track-forge/bgirl-fm1` on a new
> migration branch, starting from the branch containing this plan (or main after
> it merges). Goal: **Felucca behavior intact, BGIRL electro defaults, working
> browser build.** Preserve the prototype/history, establish a pinned unmodified
> Felucca browser baseline, then add curated electro startup defaults and minimal
> BGIRL identity through existing upstream models. Preserve track selection,
> sequencing, save/load, MIDI and clock behavior. Update obsolete architecture
> instructions and provenance as part of the migration. Keep vocoder integration
> and the dance page out of this milestone. Read the local concept/voice notes
> listed above if available. Deliver a review PR with exact revisions, verification
> evidence and a runnable browser demo recipe. Do not merge, flash, or publish
> releases/sites; distinguish browser evidence from untested physical behavior.

# Portable song snapshots — implementation plan

Status: plan only, 2026-11-18. Owner: [issue #37](https://github.com/track-forge/bgirl-fm1/issues/37).
Inspected main: `c7116c5451bcfca9325ce71a71f7356ab1bd1b2c`.
Continue implementation on `plan/song-snapshots` and its existing draft PR.
This planning commit contains only this document; it does not implement the feature.

## Outcome and user loop

Doug edits the browser instrument, exports `electro-doug-v7.bgirl.json`, hands it
 to an agent, and imports the returned file to audition changes immediately.
Ordinary note/parameter edits must not require a firmware build or PR. The browser
and agent edit the same declarative musical document, not parallel sequencers.

MVP: Export song, Import song, versioned readable JSON, deterministic schema,
small offline validate/inspect/diff tooling, documented round trips and meaningful
browser coverage. Single-sound libraries, browser version history, automatic chat
handoff, MIDI/Ableton import and a new graphical song editor are follow-ups.

## Verified foundations and constraints

Read AGENTS.md, README.md, docs/BGIRL_BASELINE_PLAN.md, docs/UPSTREAM.md and
BUILD_STATUS.md, plus the six-track/vocoder contracts relevant to touched code.
Active implementation is `felucca/`, not the historical prototype.

- `felucca/web/emu/index.html`: loadFlash/saveFlash persist simulated sectors under
  `bgirl-felucca-v1-flash`. This is origin-specific browser storage, not song JSON.
  Autosave is delayed; exporting stored sectors can miss the latest live edits.
- `felucca/web/emu/worklet.js`: UI/worklet message bridge, WASM exports, normal
  project-load input-release handling and flash sector synchronization.
- `felucca/firmware/src/project.c`: project_capture, serialized validation,
  apply/load machinery, autosave, four project slots and chain_prepare. SONG rows
  reference slots; copying only chain_config is NOT a portable arrangement.
- B6S1 is a bounded 3840-byte A/B project record. Dense valid live music can exceed
  save capacity. Do not truncate, inflate storage, change frozen defaults or add
  legacy converters as a shortcut for browser interchange.
- `felucca/firmware/src/editor_backup.c`: live project capture already exists for
  backup object 0, but packing can fail FULL and archive restore has wider write
  scope. Reuse helpers where appropriate, not the entire backup operation.
- `felucca/web/editor.html`: existing patch JSON download includes a selected
  sound/steps; this is not a full six-track portable song. Editor backup is also
  distinct. Preserve existing file/protocol behavior.
- Six full tracks, 64 steps, eight shared voices; musical state includes custom
  FM6 patches, engine-specific values, locks/motion and vocal-bank fingerprints.

Issue #30 owns SH-01A capture and #31 its ANALOG translation. Their external synth
snapshot format does not replace a BGIRL whole-song format. #35 owns visual
redesign; use unobtrusive shell controls and avoid coupling to that unmerged work.

## Document contract

Publish a machine-readable version-1 schema and a readable example during
implementation. Final field spelling is an implementation choice; the following
semantics are requirements, not an already-working API:

- `format` discriminator, integer schema version, song title, optional snapshot
  identity/parent identity and creation time; producer build SHA and capability /
  parameter-layout identity. Build metadata is provenance, not an unnecessary
  exact-SHA lockout when capabilities are compatible.
- Canonical exact engine/parameter IDs and values, with readable labels/units for
  inspection. Use stable semantic keys or an explicit versioned ID mapping, never
  assume array offsets survive parameter insertion. Resolve conflicting labels
  and values deterministically: canonical values own playback; labels are advisory.
- Global musical controls, track MIDI-channel assignments where project-owned,
  six tracks, complete current sounds (not just factory/user preset numbers), FM6
  operator payloads, step events/chords, velocities, gates, timing, probability,
  pattern length/division/swing, mixer/effects/vocoder routing, locks and motion.
- Arrangement rows and ALL referenced pattern dependencies, with document-local
  IDs. Preserve existing SONG semantics (current instruments plus referenced
  pattern/timing/motion), rather than inventing sound changes per section.
- Resource manifest: sample/vocal bank identities and required entries, factory
  capability identities and fingerprints as available. User sample bytes are not
  automatically exported in MVP; unresolved external assets are explicit.
- Field-coverage table mapping each musical field to source capture/apply code.
  List exclusions explicitly: transient voices/playhead/FX tails, held physical
  keys, visual preferences, learned mappings and unrelated libraries/settings.
  Capture musical settings, not a bit-exact continuation of audio DSP history.

Stable ordering and numeric representation permit useful Git/agent diffs. Metadata
like timestamps is excluded from semantic comparison. JSON must be editable:
never make an opaque base64 blob the sole authoritative musical representation,
and never let a stale embedded binary payload override edited readable fields.
Avoid duplicating a second set of default tables or DSP behavior in JavaScript.

Unknown format/major versions or required capabilities fail clearly. Define
optional metadata handling, numeric ranges, integer requirements, finite values,
maximum file bytes, track/event/section/resource counts and duplicate-ID rules.
No evaluation of file content, filesystem paths or automatic URL downloads.

## Live export

1. Request an explicit snapshot through the UI/worklet/WASM bridge. Capture all
   accepted edits up to a defined request boundary, not a stale browser cache.
2. Follow existing main-loop ownership and scratch lifetime rules. Stage bounded
   immutable data coherently, outside audio rendering; encode JSON on the UI side.
   Do not perform file I/O, allocation or unbounded serialization in audio render.
3. A short stopped/idle capture requirement is acceptable for MVP: show an honest
   busy/stop-first message. Do not silently stop or persist music during export.
   Never return a mixture of pre- and post-edit tracks/dependencies.
4. Download without a network service. A new click produces a new snapshot;
   concurrent requests use correlated IDs and cannot return stale results.
5. Capture dense live states even when B6S1 cannot save them, using a bounded
   logical-state bridge rather than forcing a flash encode. Report save-fit
   separately. If a state exceeds interchange limits, refuse with a specific
   reason and leave state intact; never emit a partial file labelled complete.

## Import and arrangement dependency policy

Parse and validate the entire file, capabilities and resources before replacing
music. Present a song summary and dependency/save-fit status, then explicitly
confirm replacement. Cancel/error/timeout must not partially mutate live music,
storage or control latches. Enforce bounds again at the WASM mutation boundary.

On acceptance, use normal stop/panic/load machinery and input release handling;
clear stale queued note-ons so held frontend keys cannot resurrect voices. Apply
atomically from staged state, load stopped, preserve normal six-track sequencing,
and refresh the panel. Explicitly define autosave: successful import becomes the
current project and may subsequently autosave under normal policy; export or a
cancelled/rejected import must not force a save or overwrite named slots.

**Do not overwrite destination saved slots to make a SONG work.** Stage imported
pattern dependencies in a bounded session-owned source mapping and reuse the
existing chain playback/cache preparation path. This is a source-resolution
extension, not an alternate sequencer. Document lifetime, reset/replacement,
export-after-import behavior and precedence over destination slot numbers.
Include all referenced patterns even when two tracks/rows use different slots.
Measure the required staging/cache memory; do not place large structures on stack.

Persistence is explicit: a session dependency map must never silently disappear
on reload while rows bind to unrelated slots. Implement origin-local snapshot
restoration for imported dependencies, paired with the correct imported song, or
an equally bounded persistence solution without named-slot writes. Specify how
it cooperates with existing autosave and new-project/load/reset operations; stale
sidecar state must never attach to unrelated music. Browser storage failure must
produce a truthful session-only warning and preserve the downloaded file. Test
refresh as well as same-session playback. Hardware persistence is not promised.

If implementation evidence shows this cannot be bounded without a wider project
rewrite, record that concrete blocker rather than silently shipping an incomplete
"portable song" or weakening slot preservation. A simple-pattern-only exporter
may be a development milestone but does not close this issue.

Missing resource identity: block normal successful import with actionable details;
optionally offer an explicitly labelled incomplete/silent audition, never substitute
an unrelated vocal/sample. Validate G6 identity. No automatic bundling of private
recordings, unrelated presets, device preferences or entire simulated flash.

## Agent tooling and factory promotion

Provide a small local CLI (or equivalent documented script) to validate, summarize
and semantically diff snapshots without a running browser or firmware rebuild.
Share schema/mappings rather than hand-maintaining a competing validator. Human-
readable paths should identify track/parameter/step changes; nonzero exit on error.
No daemon, cloud service, authentication or automatic chat sending is required.

An agent can edit supported canonical JSON values and return the file. Demonstrate
this loop with one parameter and one note edit and untouched-field comparison.

Factory demo promotion is distinct: preserve an approved canonical snapshot in
repository data and adapt it through a deterministic build-time import/generator
when explicitly requested later. Document that future path and compatibility
constraints; do not replace the current demo or rewrite startup/TRK_DEF in this
feature. Installing the interchange capability may require one WASM rebuild;
subsequent compatible song-file changes do not.

## Implementation order and validation budget

1. Inventory capture/apply coverage and dependency ownership. Specify schema,
   capacity limits, restoration semantics and focused acceptance cases first.
2. Implement snapshot capture + export; prove unsaved changes and dense-state
   behavior. Add a small public fixture using existing authorized assets only.
3. Implement staged validation/import, dependency mapping and reload lifecycle;
   prove cancellation/failure atomicity and saved-slot preservation.
4. Add offline inspect/validate/diff, desktop/mobile controls and user docs.
5. Complete focused acceptance, relevant existing regression coverage and exact-
   revision handoff. Keep commits scoped; continue the existing PR.

Testing policy must be documented in AGENTS.md/README during implementation,
reconciling current blanket instructions rather than silently ignoring them:

| Change | Required verification |
| --- | --- |
| This plan only | Markdown review and git diff --check; no runtime build/tests |
| Routine compatible song data | Schema/capability validation, semantic diff, affected song import/load/play smoke; no firmware rebuild or full engine suite |
| Browser UI/JSON tooling | Schema/negative cases, actual browser export/import flow, affected shell tests |
| Project/bridge/chain/storage code | Relevant codec, six-track, editor, persistence, chain/input and browser regressions; existing required CI remains enabled |
| DSP/scheduler or release | Broader applicable regression/target gates; DSP changes are outside this scope |

For implementation run the repository-required full host/CI acceptance at the
final frozen runtime revision, not repeatedly after every JSON tweak. Re-run a
suite when changed code or a failure invalidates its evidence. Do not regenerate
audio galleries, broaden fuzz matrices or rerun unrelated suites gratuitously.
Do not claim browser/host results establish physical-device fit or listening approval.

### Required proof cases

- Immediate unsaved knob, step, mix and motion edits appear in export without
  waiting for autosave; a failed export never changes saved storage.
- Round trip across a fresh browser profile preserves all declared fields,
  custom FM6 payloads, T5/T6, step63 and motion/locks. Compare normalized semantic
  state, not timestamps or browser identity; inspect actual audio playback.
- Arrangement with several source slots works when destination slots contain
  different music; unchanged destination slots verified before/after and after
  refresh. Subsequent new/load/export cannot reuse stale dependency state.
- Changed canonical JSON parameters/notes take effect without a build. Invalid
  types/ranges/counts/versions/IDs, truncated files, oversized input, missing banks,
  request timeout and cancelled import leave current music/storage intact.
- Save-capacity boundary/full states stay truthful; document-level success does
  not imply the 3840-byte hardware slot can hold it. Existing FULL atomicity stays.
- Successful import is stopped, has no stuck/queued notes, preserves user assets
  and preferences, and supports normal editing, autosave and later export.
- Desktop/mobile accessible controls, download + upload using actual browser
  files, fresh-profile and storage-denied/session-only paths, no console errors.

## Completion and boundaries

Deliver same PR with implementation, schema/example, focused tooling, docs and
verification receipt: exact SHA, commands/results, local browser demo recipe,
remaining limitations and human listening/hardware gates. Link issue #37.

No merge, manual deployment, release/tag, device build/flash, engine/BSP pin change,
new audio generation, Ableton/MIDI access, cloud sync or unrelated visual redesign.
Do not restart or repoint the existing tailnet preview. A local loopback preview
for implementation verification is in scope; publishing it is separate.

## Copy/paste implementation handoff

Implement this plan in track-forge/bgirl-fm1 on the existing plan/song-snapshots
branch and draft PR, not a parallel PR. Local worktree:
`/home/dev/.openclaw/workspace/codebase/bgirl-song-snapshots-plan`.
Read AGENTS.md and this whole document. Verify current main/base and working state;
resolve routine integration conflicts without merging. Build the portable live
song export/import and offline inspect/validate/diff loop, including arrangement
dependencies, exact sound values, unsaved edits, atomic import and explicit reload
semantics. Preserve named saves/libraries/settings and all engine pins. Keep song
JSON editable without rebuilds. Follow the scoped validation budget; deliver
reviewable commits, update this same PR and report exact evidence plus a local
browser audition recipe. Do not merge, deploy, build/flash hardware, change the
factory demo or treat automated checks as Doug's musical approval.

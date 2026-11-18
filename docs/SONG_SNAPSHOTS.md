# Portable browser songs · v1

[Issue #37](https://github.com/track-forge/bgirl-fm1/issues/37) ·
[PR #38](https://github.com/track-forge/bgirl-fm1/pull/38) ·
[JSON Schema](song-v1.schema.json) · [Example](../examples/song-v1.bgirl.json)

## Make a file, hand it to an agent, play it

1. **Switch on**, edit normally, stop playback, release keys/buttons and unlatch
   held controls. Allow queued panel/MIDI edits to finish (a frame, not autosave).
2. **Export song** downloads `TITLE.bgirl.json` with live unsaved values. It does
   not save/erase flash, stop playback for you, or wait for the delayed autosave.
3. Edit the readable JSON locally or hand the file to an agent. For example:

   ```sh
   node tools/song-snapshots/song.mjs validate my-song.bgirl.json
   node tools/song-snapshots/song.mjs inspect my-song.bgirl.json
   # Edit a canonical value, e.g. song.tracks[5].parameters.P_PAN,
   # or song.tracks[4].steps[63].notes[0] (zero-based tracks/steps).
   node tools/song-snapshots/song.mjs diff before.bgirl.json after.bgirl.json
   ```

4. **Import song**, choose the returned file, review the summary/current-pattern
   B6S1 size estimate, and confirm replacing current unsaved music. It loads
   stopped through the normal project loader; press normal PLAY to audition.
   GLO → SONG → PLAY uses the ordinary arrangement player.
5. Later export includes your new edits and the arrangement's source patterns.
   **No rebuild is needed for compatible song changes.** One browser build installs
   this capability; future engine/layout changes may require a new capability.

`inspect` summarizes counts; `diff` names exact track/parameter/step paths and
ignores title/provenance metadata. Invalid files return nonzero. Offline validation
cannot tell whether a particular browser has an optional vocal bank; the WASM
validator checks that identity before replacement. Private SAMPLE/GRAIN/SLICE
user slots are explicitly unresolved in v1, never bundled/substituted. The CLI
reports them and returns nonzero; normal browser import refuses them.

## What the document owns

`format=bgirl-song`, integer `version=1`, and a capability including a descriptor/
factory-resource fingerprint are required. Producer build SHA is provenance, not
an exact-build lock. The layout is generated from `core.h` and the actual pinned
engine descriptors, not a second JavaScript default/DSP implementation. IDs use
stable `P_*`/`G_*` names; common mappings, per-engine ranges and alias exclusions
are in `snapshot-layout.mjs`. Numbers are **raw instrument counts**, not physical
units; the normal instrument display supplies physical units/enum names.
`midiChannel` retains the project's stored channel offset:0..15 means physical
one-based channel `((value + trackIndex) % 16) + 1`;16 is OMNI,17 OFF. Track indexes
are zero-based. This preserves normal per-track assignments, not MIDI learn or
device preferences. `layout.engines[engine].name` is the readable engine label. `P_E0..P_E7` are
engine-specific; consult that engine's parameter descriptor when editing them.

`title` is a free title (up to120 characters). `song.name` is the existing
12-character uppercase panel name; these are distinct and survive imported
browser checkpoints. Optional `metadata` is an object for IDs, parent IDs,
creation time and other advisory annotations; export refreshes its producer.
Unknown top-level fields/format/version/capability fail. Unknown metadata is kept
and has no playback authority. Reordering JSON object keys has no musical effect.

All127 sound counts and27 global slots are explicit. The three UI actions
G_SLOT/G_LOAD/G_SAVE are constant defaults, not musical destination controls.
Track `preset` is an advisory label, not a request to reload an external preset.
**Embedded FM6 bytes own its patch**, not SLOT or `preset`: when editing a patch,
set that track's `parameters.P_E7=8` (OWN). Normal `fm6_adopt` displays a matching
factory slot, otherwise OWN. This label may normalize if a file deliberately
contradicts its embedded patch. There is no stale binary overlay overriding JSON.
The128 FM6 bytes are the documented seven-bit packed operator/voice fields,
readable numeric JSON (not base64); `fm6_pack/unpack` is the existing codec.

| Musical data | Capture/apply source |
| --- | --- |
| Global tempo, swing, MIDI/clock, tuning, global FX; selected track/channels/name | `project_capture` / `project_restore_runtime` in `project.c` |
| Six complete sounds, mixer/FX/insert/slicer/vocoder routing and engine counts | `motion_base_value` / normal per-engine `param_fit` loader |
| Custom FM6 operator/voice payload | `fm6_pack` / `fm6_unpack`, `fm6_set_patch`, `fm6_adopt` |
| 64 steps each, all four dormant notes, time/ties, flags/accent/slide/ratchet/nudge, velocity, drum hits/accents, chance/conditions | `step_t` / `proj_steps`, existing sequencer |
| Length/division/swing/gate/quantize | `P_SLEN..P_SGATE`, `P_SQNT` / existing timing |
| Motion and locks (track, step, stable parameter ID, lock flag, value) | `motion_store_t`, `motion_valid` / normal motion loader |
| Six-lane SONG rows and up to4 complete source projects | `chain_config` plus slot or portable-source resolution / `chain_prepare`, `song_source_info` |
| Vocal identity and user-sample requirements | `bg_vocal_expected`, source/motion scan / pre-apply bank check |

Excludes transient voices/playhead/FX history, held keys, live vocoder HOLD/Freeze
(always0), performance mutes, display/preferences, learned mappings and unrelated
preset/sample libraries. This is musical settings, not sample-exact DSP resume.
No filesystem paths, code evaluation or automatic URL downloads are performed.

## Bounds and atomicity

File limit2MiB; logical transfer limit65,536 signed32-bit words; six tracks ×64
steps ×4 note fields,128 motion/locks per project,16 rows and four uniquely named
sources (`pattern-0..3`, `null` for silent). Source IDs address document-owned
patterns, never a destination named save. Dormant row/step fields are retained;
only `arrangement.count` rows execute. Source projects' own arrangements are not
recursive: SONG uses their step/timing/motion fields with **current instruments**.
Engine-incompatible source motion is filtered by the existing cache-preparation
logic. Source sounds are retained for identity/roundtrip, not section sound loads.

The shared module validates types, exact counts, IDs, duplicates, ranges, required
references and manifest consistency. WASM revalidates bounded fields before any
mutation and checks resource availability. Import preview/cancel/error/expired
request do not release controls or change music/storage. Accepted import clears
queued frontend and MIDI note-ons/encoder edges, stops/panics via normal project
restore, replaces the whole state, refreshes HOME and leaves all named saves,
user assets and preferences alone. Requests have correlated IDs and expiry times;
a queued expired request cannot later apply. Export refuses busy/queued input
rather than returning a mixture or stale cached state.

## Arrangement lifetime and refresh

Four browser-only source projects are session-owned, take precedence over local
slots0..3 while attached, and feed the **existing** `chain_prepare` and sequencer.
SONG's overview resolves their names, lengths and density through the same source
precedence; PROJECT's save/load browser continues to show the actual named slots.
Successful new/blank/demo/load/runtime restore detaches them; a failed load does
not. Changing an arrangement row to another source can resolve a local slot if
that ID was not imported; the next export includes every currently used source.
Unreferenced imported sources disappear from subsequent files. No slot is written
to make an arrangement work.

All successfully imported sessions use a complete browser checkpoint, including
FULL simple patterns. Their B6S1 autosave is suspended so no source-less imported
rows can reappear later. The initial accepted song is saved immediately; while
stopped and controls released, a UI-side2-second timer requests fresh live state
and checkpoints subsequent edits. No capture/JSON allocation/file I/O is added
to audio rendering. The checkpoint is in the **same atomic localStorage record**
as that browser's flash, under `_portableSong`, not a loose sidecar. On refresh,
startup restores flash then validates/restores that entire paired song stopped.
Storage failure warns **SESSION ONLY**; keep/export the downloaded file. Busy
playback/held controls defer checkpoints: stop, release and wait at least2seconds
before refresh, or export for a durable copy. This is not continuous recovery
from an abrupt close during playback or a silent promise of hardware persistence.
The browser's RESTORE LAST preference still gates checkpoint restoration.

New/load/reset removes the paired checkpoint and resumes ordinary firmware
autosave. Named saves remain available for ordinary simple imported patterns;
portable arrangements refuse SAVE with **EXPORT SONG FILE** because B6S1 cannot
carry their owned sources. The displayed bytes/3840 is a **current-pattern codec
estimate**, not a promise the arrangement is portable through hardware slots.
Dense JSON can be valid and complete while B6S1 says FULL; existing storage bounds,
last committed records and FULL atomicity remain unchanged.

### Memory

Browser-only static interchange allocation: **327,808 bytes** =262,144-byte int32
wire + five staged projects + four retained source projects (7,296bytes/project).
Existing chain cache is reused, not duplicated; no large stack object. Compiled
only under `FELUCCA_WEB`; no device-fit claim, flash map change or dependency pin
change. External/source-bank bytes are not included in snapshot files.

## Schema maintenance and validation budget

After an intentional descriptor change and ordinary asset generation:

```sh
python3 tools/song-snapshots/generate_layout.py
node tools/song-snapshots/schema.mjs
```

Root generation and tests check these artifacts without rewriting them. Reconcile
capability changes explicitly; do not update frozen project defaults. The JSON
Schema artifact provides structure/ranges; semantic/resource checks additionally
run through the shared JS and WASM validators.

- Compatible song edits: CLI validate/diff + affected import/play smoke only.
- UI/JSON code: negative/schema cases + actual files and affected browser tests.
- Project/bridge/chain/storage code: codec, six-track, editor, persistence,
  chain/input and browser acceptance; root full host runner **once on final
  frozen runtime**, with existing CI retained. Repeat only invalidated evidence.
- No gratuitous gallery generation, unrelated fuzz expansion or DSP gates for
  ordinary JSON changes. No human listening or physical-device claims.

Factory-demo promotion is deliberately separate: a later approved canonical song
can live in repository data and be consumed by a deterministic build-time
snapshot-to-project generator, checking this capability/resource layout. That
future generator must reuse these mappings/normal loaders, not rewrite TRK_DEF
or introduce a second sequencer. This feature does **not** change the demo.

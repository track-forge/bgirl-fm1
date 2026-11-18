# CREW HOME — the dancers are the instrument's front page

Date: 2026-11-17. Status: **draft implementation plan; no runtime changes**.
Inspected main: `c51c53885fd18c1e84be676d1644143803bc2e0b`.
Related: [merged Arp Dance PR17](https://github.com/track-forge/bgirl-fm1/pull/17),
[original exploration #2](https://github.com/track-forge/bgirl-fm1/issues/2),
[implementation handoff](CREW_HOME_HANDOFF.md).

## Product intent

A pocket electro crew: **one b-girl for every track**, reacting independently to
that track's music. Dancers are protagonists and a legible performance overview,
not decoration buried in the arp menu. HOME is the dance floor; ordinary sound,
sequence and effect pages remain the editing tools. ARP DANCE stays available with
its existing trigger-driven CRISP/GHOST contract.

Doug endorsed tempo-based movement with track-driven accents, recognizable rather
than perfect source motion, and a HOME-first experiment. This PR records the plan,
not completed implementation or final art approval. Recommendations below are
implementation defaults, not claims that every detail was individually approved.

**Baseline correction:** our conversation said four dancers, but current main has
**six full tracks**, 64 steps per track and eight shared admitted voices. Implement
for `NTRK` (currently six), including T5/T6. Do not regress to four tracks, hide the
last two behind a permanently four-person scene, or change the voice budget.

## Existing integration points

Read current source rather than treating historical four-track comments as truth:

- `felucca/firmware/src/ui.c`: HOME views SCOPE/TRACKS/LEVELS, `ui_home_view`,
  `home_param`, `home_trk`, `go_home`, `home_tap`, layout selection.
- `ui_graph.c`: HOME drawing/invalidation and existing fixed 96px dance renderer.
- `ui_draw.c`, `ui_input.c`, `ui_menu.c`, `menu_items.c`: labels, controls,
  preference navigation and interaction overlays (locate actual menu file).
- `voice.c`: `trk_note_on`, `trk_note_off`, voice start/admission and release;
  mute, deferred engine switches, mono priority and legato complicate raw counts.
- `seq.c`: normal keyboard/MIDI/sequence/arp note paths and transport timing.
- `arp_dance.h`: existing per-track atomic arp pose publication. It is not yet
  a general track-activity interface; keep its observable behavior intact.
- `assets/dance/`, `tools/convert_dance.py`, `tests/dance*` and
  `docs/ARP_DANCE_VERIFICATION.md`: reusable asset and verification foundations.
- `docs/SIX_TRACK_IMPLEMENTATION_PLAN.md`, `SIX_TRACK_VERIFICATION.md`,
  `SIX_TRACK_CODEC_SPIKE.md`: six-track controls and frozen B6S1 constraints.

## HOME and control contract

Add **CREW** to MENU → DISPLAY → HOME, preserving numeric IDs and behavior of
SCOPE/TRACKS/LEVELS. Recommend CREW for genuinely fresh settings only; honor restored
view choices and other saved preferences. Explicitly test unknown-value fallback;
do not reset settings or change projects to install the new default.

- HOME from an editing page returns to the chosen HOME view. Preserve the existing
  HOME → MIXER → CLOCK cycle and HOME-hold menu behavior.
- CREW's K1–K4 retain the selected engine's normal HOME parameters. Do not silently
  make them animation controls or six-track level controls.
- ALGORITHM selects T1–T6 normally. Each dancer has stable placement and a visible
  T-number; highlight selection using more than color alone. Keep selected sound,
  control labels/values, transport and useful clock feedback readable.
- GLO performance layers retain T1–4 / T5–6 banking and inactive-cell semantics;
  LEVELS/MIXER stay accessible. Do not invent six physical knobs.
- Menus, confirmations, temporary edit overlays and performance layers own their
  display when open. No dancers painting over them or stealing input focus.
- Preserve labeled browser controls. The animation supplements, not replaces,
  textual track/selection/mute information. Other HOME views offer a nonanimated
  alternative. No claim of new screen-reader parity.

### Composition decision: prove at actual size

The display is 240×240; the old dance page uses a roughly 122px graph below its
controls. Six 96px dancers cannot simply be placed there. Make actual-size
contact sheets and short browser previews of at least these two layouts:

1. **Equal crew:** six stable cells, initially 3 columns × 2 rows, compact labels.
   Compare with a staggered stage if it improves floorwork readability.
2. **Featured dancer:** larger selected dancer plus five smaller, always-visible
   crew indicators/dancers. Keep stable track identity when selection changes.

Prefer equal crew if the windmill/worm remain recognizable. Otherwise implement
the featured layout and document the readability reason. The implementer may
choose this bounded fallback with evidence, without stalling routine work for
approval. Do not call either layout human-approved until Doug has seen it.
Reclaim HOME-only vertical space if needed, but retain readable knob feedback;
show the proposed chrome at 1× rather than evaluating isolated large sprites.
LARGE mode, palettes, FLAT/LINE and mobile browser scaling need explicit review.

## Musical behavior: rhythm plus expression, not six independent GIF timers

Use the instrument's musical clock for the base phrase. Start with one cycle over
four quarter-note beats, with per-clip metadata if a move reads better over two or
eight. Tempo edits preserve phase; missed display frames jump to the current pose,
never slow audio or accumulate an animation backlog. Initial rendering target is
approximately 12–15 updates/s, capped and measured, not a device FPS promise.

| Track state/event | Proposed visible behavior |
| --- | --- |
| Admitted note attack | Accent the current move; bounded pose hold/contrast emphasis, not restart at frame zero for every hit |
| Dense notes/chord | Coalesce accents per display interval; chord/unison voice count must not multiply dance speed |
| Sustained/gated activity | Continue phrase movement without requiring repeated note-ons |
| Released / inactive | Complete a short bounded settle (initially one beat), then a readable freeze |
| Muted / excluded by solo | Dim and park immediately; no claim of activity from rejected notes |
| Unmuted | Rejoin on the next genuine activity; unmute alone must not manufacture a note |
| Selected | Persistent outline/cell focus/T-number, independent of musical activity |
| Transport stopped | Sequencer-driven motion settles; live keyboard/MIDI attacks still animate using a defined local BPM phase |
| Project change / panic / all-off | Clear transient activity safely; no stale dancing from the previous project |

Velocity can control a bounded accent strength, with a useful minimum. Never scale
sprites unpredictably into neighbors. One-shot drum/sample activity needs a bounded
activity envelope even after immediate note-off; a held MIDI key is not proof that
a one-shot is still sounding. Legato changes may accent without treating every
allocated voice as a fresh phrase. Define and test these distinctions explicitly.

MVP movement selection: deterministic track-to-move defaults distributed across
available clips (six dancers may share clips), distinct stable track labels/focus,
and independent phase/activity state. No instrument-role inference from engine or
track number is required. Add moves without rewriting ARP DANCE. Automatic move
switching, if included, occurs at a phrase boundary; arbitrary windmill-to-worm
cuts on every hit are not the default. User choreography editors and saved per-track
move assignments are follow-ups, not a new project-format requirement.

### Telemetry and timing design

Prefer internal accepted note/gate/one-shot state, not FFT or mixed-output analysis.
Keyboard, sequencer, arp and MIDI should converge on the same normal track path.
Inspect admission/voice stealing/mono priority/deferred switches before choosing
hooks. Do not increment at raw MIDI receipt and again at engine playback. Publish
what the display actually means: **musical track activity**, not a calibrated VU
meter or guarantee of audible output (zero level and vocoder routing can differ).
Mute/solo state must still be truthful.

Document each field, writer ownership, clock unit, wrap arithmetic and reset rule
before coding. Publish bounded coherent snapshots; use the existing lock-free
whole-word model where possible. If multiple words are needed, prove coherent
reads without spinning/blocking the audio producer. The existing arp-only ownership
assumption must not be copied blindly to voice/worker paths. No UI traversal of
mutable voice arrays across cores without a safe publication contract.

No allocations, logging, I/O, locks, graphics, video decoding, extra musical RNG,
or waits in the audio path. Do not mutate musical timing or sequencing to animate.
A separate deterministic visual choice must not consume the musical RNG stream.
External START/STOP/CONTINUE, BPM changes and free-running live-play phase require
explicit semantics and tests. Preserve supported browser vs host MIDI distinctions.
Hidden/covered CREW performs no animation painting or catch-up queue processing;
re-entry samples current state. ARP DANCE keeps its existing trigger semantics.

## Supplied video sources — all four are in implementation scope

Doug supplied these on 2026-11-17 and explicitly authorized all this information
for the public GitHub repository, including the supplied videos for sprite work.
Record that authorization alongside provenance; it is not a claim that public
availability itself grants rights or that model provenance has been independently
verified. Publish derived sprite assets/previews with the repository's GPL-3.0-only
convention and the attribution/permission record. No new generation is requested.

| Stable source ID | Doug's description | Public source |
| --- | --- | --- |
| `windmill` | windmill | https://www.moltazine.com/post/795f4882-e0a3-4fec-9de4-4a136c213ebb?media=visual |
| `reference-02` | unspecified; inspect and name after review | https://www.moltazine.com/m/stevedore/post/2f43ab03-2793-4b31-8ffb-be56f6a2bc0d |
| `wave-ish` | wave-ish (provisional) | https://www.moltazine.com/m/stevedore/post/c90acbbd-1cd3-4176-b170-ea9c8f74eb9d |
| `worm` | worm | https://www.moltazine.com/post/d0e1e91c-9280-42fc-9d1d-86ecc730d481 |

These are source-page links, **not verified direct MP4 URLs**. This planning pass
has not downloaded, decoded or visually assessed these four videos. Do not invent
duration, hashes, move names, frame counts or seamless-loop claims. The existing
FLOORWORK asset from PR17 is a separate fifth source, not one of these four.

### Conversion deliverables

1. Resolve each post's original visual video using supported Moltazine access.
   Record public post/creator attribution as observed, original media format,
   bytes, SHA-256, dimensions, frame rate/time base, duration and decoded frames.
   Do not commit credentials or expiring authenticated URLs. Keep downloaded
   originals in ignored local storage; commit reproducible metadata and derived
   assets, not large MP4s by default. A public immutable original URL is useful
   if available; source-page URL plus hash is the durable minimum.
2. Extend the existing offline FFmpeg/Pillow converter deliberately: it currently
   assumes twelve poses and a fixed output bank. Support named multiple clips and
   layout-appropriate sizes without overwriting the old FLOORWORK bank. Pin and
   record tools/settings, frame indices/timestamps and exact reproduction commands.
3. Use filled 1-bit masks with fixed per-clip union bounds, preserved aspect ratio,
   stable floor/anchor, and padding for every limb. No independent per-frame
   centering. Review segmentation, shimmer, negative space and extreme floorwork
   at the final display size, not just 96px or full video resolution.
4. Compare roughly 48/64px small silhouettes and larger featured sizes as the
   layout permits; choose from actual results. Start at 12 poses per move, vary
   only with documented readability/size benefit. Preserve source-order motion.
5. Produce per-clip contact sheets and standalone playback previews with intended
   BPM/phrase timing, explicit last→first seam, idle and mute/focus treatments.
   A hard cut or freeze is acceptable if documented; do not assert seamlessness.
6. Attempt conversion of all four sources and ship each usable pack. For an unusable
   source retain its metadata, evidence and precise blocker; do not silently omit
   it or spend credits regenerating. Existing FLOORWORK can remain a fallback.
7. Commit manifests, derived masks/headers, review previews, permission/attribution
   and converter tests. Independent repeated conversions must match. Normal build,
   CI and runtime consume committed assets offline with no Moltazine dependency.

Shared clip storage is per move/size, not per dancer. For illustration, four
12-pose 64×64 one-bit banks total 24,576 bytes before metadata; six dancers do not
multiply that by six. This is not a chosen final size or target-fit claim.

## Implementation sequence and acceptance

1. **Baseline:** read AGENTS and current feature docs, pin fresh main, create an
   isolated feature branch/worktree. Reproduce `./build.sh --web` and
   `./tests/run_tests.sh` before runtime changes. Record existing failures honestly.
   Coordinate with other open plans (e.g. Mega Jam demo PR28); do not bundle their
   musical/demo changes or require their merge for CREW.
2. **Art/layout spike:** convert the four references, compare six-person layouts
   at 240px, choose and document the bounded layout. Ship inspectable artifacts.
3. **Activity model:** implement/test coherent six-track telemetry and timing.
   Exercise dense/chord/mono/legato/one-shot/stolen/deferred/failed or suppressed
   notes, mute/solo, stop/live play, panic, project switch and counter wrap.
4. **HOME integration:** add CREW, fresh default and preserved saved preferences,
   controls/overlays/invalidation; keep all existing views and ARP DANCE working.
5. **Verification and handback:** evidence below, updated README, dedicated
   `docs/CREW_HOME_VERIFICATION.md`, exact tested revision and implementation PR.

Required evidence:

- Full root web build and host suite, focused CREW tests, applicable six-track,
  persistence, MIDI/clock, vocoder and original dance regressions. Keep golden audio
  unchanged; compare identical deterministic audio/RNG/sequence output with CREW
  disabled, visible, hidden and under dense all-six-track playback.
- Actual Chromium interaction/audio after user gesture: fresh CREW default,
  restored SCOPE/TRACKS/LEVELS/CREW, all six selected tracks, GLO banks/inactive
  cells, engine knobs, HOME/MIXER/CLOCK/menu, mute/return, live notes when stopped,
  sustained and one-shot sounds, save/reload and project replacement. No page errors.
- Screenshots/video at actual panel resolution and desktop/mobile, LARGE and
  representative palettes/styles. Demonstrate all six identities and multiple
  move packs; record visual findings separately from Doug's subjective approval.
- Coherent snapshot stress tests; skipped paints land on current phase; no hidden
  or unchanged-idle graph transfers; dirty regions restore old silhouettes without
  trails. Bound redraws with many attacks rather than one paint per event.
- Exact linked asset, text/rodata, mutable RAM and browser size deltas; no new
  framebuffer unless justified. Report bytes per changed region and measured
  display/worklet activity. Small masks do not make full-screen RGB565 transfers
  small. Existing dance page transfers 58,560 pixel bytes per changed graph.
- Performance evidence is host/browser only. Physical LCD/DMA tearing, target
  atomic codegen, linker fit, deadlines and xruns remain unverified separately.

Demo recipe: fresh boot → CREW → load existing six-track audition → PLAY → select
T1–T6 → mute bass and return it → isolate a sparse/held part → visit ARP DANCE and
return → STOP → play live notes → change HOME view → reload and verify preference.
Do not alter the default musical demo just to make the animation look busy.

## Boundaries and follow-ups

This PR changes documentation only. Subsequent implementation may add local asset
conversion, firmware/browser UI and tests, and an implementation PR. No new paid
image/video generation, DSP redesign, FFT, new sequencer, track/voice count change,
B6S1 format expansion, frozen-default edits, dependency pin changes, device build
or flash, release/tag, manual deployment or merge is part of this milestone.

Later possibilities: per-track move selection, beat-quantized choreography,
character-specific silhouette identities, audio-envelope modulation and a larger
performance-focused stage. None is required to make the initial crew useful.

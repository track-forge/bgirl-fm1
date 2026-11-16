# Arp Dance — one-move implementation plan

Date: 2026-11-15. Status: **plan only; no implementation or device proof**.
Implementation follow-through: [host/browser verification](ARP_DANCE_VERIFICATION.md)
(records shipped browser scope separately from this original plan).
Planning baseline: `c3edd758d6bc12ffb74f0522b4904210816ec93f` (main).
Related exploration: [issue #2](https://github.com/track-forge/bgirl-fm1/issues/2).

## Product decision and bounded delivery

A breakdancer is a bank of silhouette poses played by the existing arpeggiator,
not a video player. Add an **ARP DANCE** page in the selected track's ARP family.
Ship **one move, twelve poses**, with two styles: **CRISP** and **GHOST**.
Target 96×96 packed one-bit masks; compare 64×64 before choosing the shipping size.
The existing source clip below is the only required move. No new generation is
needed for this round. Future banks can replace the art through the same converter.

This narrows/supersedes issue #2's initial home-tab, two-move, free-running BPM
exploration. The current application is Felucca, not the historical `app/`/`base/`
prototype. This document is self-contained; the local Obsidian note is background,
not a dependency. Timing and control details below are recommended implementation
contracts: document any necessary deviation with evidence in the implementation PR.

## Current code map — recheck against implementation main

All following paths are under `felucca/firmware/src/`:

| File | Relevant existing behavior / intended seam |
| --- | --- |
| `params.c` | `FAM_ARP`, `GR_ARP`, ARP/ARP 2 page definitions; existing rate/mode/hold/swing/probability controls |
| `seq.c` | `arp_fire`, `arp_chord`, `arp_step`, `arp_external_due`; shared internal/external timing and probability decision |
| `core.h` | Track/runtime structures; inspect ownership before adding bounded display telemetry |
| `ui_input.c`, `ui.c`, `ui_help.c` | Family navigation, knob dispatch, help, track selection and layout |
| `ui_graph.c` | Graph renderer and `graph_signature` invalidation; avoid unconditional animation redraws |
| `lcd.c` | Existing DMA buffer ownership and synchronization; tiny source masks do not eliminate panel-transfer cost |

`arp_idx` advances before probability rejection and can reset on a fresh chord.
It is therefore **not** an exact successful-trigger counter. CHORD returns through
its own path. A display hook must cover that path once without altering audio/RNG.
Do not presume UI and audio are on isolated cores or safely share arbitrary structs.

## Page and control contract

- Normal ARP-button navigation cycles ARP → ARP 2 → ARP DANCE → ARP, retaining
  existing navigation/long-press/overlay behavior. Implement through normal family
  machinery, not a replacement global button handler.
- Follow the selected track. Display track identity, move name, style, and enough
  rate/mode context to explain a stationary dancer (ARP OFF / no held notes).
- Proposed four knobs: K1 existing ARP MODE, K2 existing ARP RATE, K3 CRISP/GHOST,
  K4 reserved/unassigned. MODE/RATE edits are real musical edits, visibly labeled.
  Do not add a fake move selector for the single available move.
- Retain keys, HOLD, transport, track selection, note release and panic semantics.
  ARP 2 continues to own swing/probability/hold/order; no duplicate musical state.
- Give the dance the largest stage that fits the real page layout without obscuring
  controls. 96×96 at 2× or 64×64 at 3× would be 192×192, but that stage size is not
  mandatory if page chrome requires less. Inspect actual 240×240 screenshots.
- Style is session-only UI state, initially CRISP (global UI preference is sufficient
  for this one-move milestone). Do not add persisted parameters, change `P_COUNT`,
  serialized formats or legacy project defaults solely for this visual toggle.

## Arp-to-pose timing contract

1. One successful musical arp trigger advances one pose in source order, wrapping
   twelve poses. The first trigger after reset displays pose 0; initial idle also
   displays pose 0, without a ghost. A chord trigger advances once, not per voice.
2. Probability-rejected steps do not advance. No held/latched playable notes, ARP
   OFF, or no arp triggers means hold the current pose; there is no fallback timer.
   Note-offs and gate expiry never advance or blank the silhouette.
3. Existing RATE, swing, tempo and internal/external clock determine event timing.
   HOLD behaves as it does for music. Do not copy the clock or generate MIDI notes
   for animation. Existing RNG must not be consumed by visual code.
4. Rate is **one pose per trigger**, not a separately specified twelve-pose cycle
   duration. At quarter-note triggers the move takes twelve beats; at sixteenth-note
   triggers it takes three beats. This intentionally differs from the earlier
   cycle-per-bar idea. Cycle-length/divider controls are a later feature.
5. Arp MODE still determines musical note ordering; it does not map note pitch or
   UP/DOWN/RANDOM onto visual direction in this round. Keep choreography forward.
6. Observe actual arp activity rather than `song.playing` alone: internally clocked
   held-key arps can operate independently of sequencer transport. Under external
   clock, honor existing start/continue/stop behavior without inventing new semantics.
7. Maintain bounded per-track visual progression while hidden, but do **no hidden
   drawing**. Re-entry/track switch shows that track's current pose, not a queued
   replay or an unrelated track's ghost. Late UI updates skip straight to the newest
   state; they must never backpressure audio or drain an unbounded event queue.
8. Reset visual progression on initialization, project replacement and panic/all-off
   reset paths; next trigger shows pose 0. Ordinary key release, ARP OFF and transport
   changes hold the pose unless the inherited path explicitly performs that reset.
   Test and document exact reset call sites; never change musical reset behavior to
   satisfy animation. Track selection/page entry alone does not reset progression.

Publish the smallest coherent snapshot (e.g. current/previous pose, validity and a
revision/reset generation) at the existing sequencing ownership boundary. O(1)
integer updates only in the audio/sequencer path: no graphics, allocation, file I/O,
logging, locks/waits or decoding. Select and document a concurrency-safe handoff
appropriate to this codebase; `volatile` alone is not synchronization. The UI must
read a consistent snapshot without making the audio thread wait. Tests must cover
multiple triggers between paints and reset/wrap behavior.

## Exactly two rendering styles

**CRISP:** clear/restore the old stage region, then draw the current filled mask.
No tween, crossfade or intermediate bitmap.

**GHOST:** clear/restore the old stage region; draw the immediately preceding
*logical triggered pose* in a dim, theme-compatible color; draw the current pose
solid on top. Current wins at intersections. Exactly one previous pose, no trail
buffer. Before the first transition, draw only the current pose. If UI frames were
skipped, ghost the previous logical pose, not the last pose the LCD happened to show.

The ghost persists until the next trigger or reset: one trigger of delay, not a
millisecond timer. No continuous alpha fade or extra timer-driven redraws. Switching
to CRISP removes the ghost immediately. Pauses retain the currently composed pose.
Restore all old bounds to prevent trails; respect existing LCD DMA synchronization.
Style, pose, selection, theme and page changes must invalidate the appropriate graph.

## Source clip and reproducible offline asset pipeline

Canonical source: https://www.moltazine.com/post/a559b395-af6b-49c0-b6e9-08b11a202269

Observed source at planning time:

- Author: `stevedore`; black silhouette floorwork, white background.
- MP4: 608×352, 24 FPS, 96 video frames, 4.000 seconds; 180,213 bytes.
- SHA-256: `acb274cd20f88d92a624453eb4b2eb4d1f3d7e2f3c687988e39475b207fd3d51`.
- Post describes a Qwen Image 2.1 still animated using MiniMax H3 reference-to-video,
  first-frame only, deliberately without a loop constraint. These are reported
  provenance, not independently verified model/job parameters.
- Workspace convenience copy: `tmp/bgirl-dance-inspect/source.mp4` beneath the
  workspace root, not the repository. Do not require that private filesystem path
  for contributor builds; retrieve the canonical post media and verify the hash.

Create a small explicit CLI/tool under `tools/` plus an asset manifest and generated
C/header data following existing repository conventions. Suggested steps:

1. Ingest a local MP4 and record source URL/hash, frame rate/count, chosen segment
   and decoder/converter versions. Ignore any audio stream entirely.
2. Establish one square stage over the entire selected sequence. Calculate union
   silhouette bounds and margins, then crop **or pad** to preserve extended limbs,
   floor contact and aspect ratio. No independent frame centering/scaling, and no
   blind square center crop that cuts off hands or feet.
3. Start with twelve evenly distributed timestamps/frame indices; allow explicit
   indices/holds in the conversion recipe so key poses can be hand-curated. Twelve
   distinct entries, no repeated endpoint inserted merely to close a loop.
4. Segment the dark figure, simplify wispy hair/noise, preserve negative space and
   inspect resampling/threshold order. Produce 96×96 and 64×64 comparisons using
   pinned settings; record any cleanup rather than relying on invisible manual edits.
5. Export row-major packed 1-bit masks with declared bit order, row stride,
   dimensions, anchor, pose order and byte count. Validate bounds/counts at build
   time. MVP runtime order is sequential with one trigger per pose; per-pose holds
   can remain tooling metadata/deferred rather than growing the runtime contract.
6. Produce contact sheets and a browser/offline preview at intended screen scale,
   including the 11→0 seam, CRISP and GHOST, and slow/fast trigger rates. This source
   is not guaranteed to loop: choose the best segment/poses, document a deliberate
   hard cut if acceptable, or report the seam as an art blocker. No silent rerender
   through a paid generation service, rigging, optical flow or synthetic inbetweens.
7. Record redistribution permission/license separately from provenance. A public
   post is not itself an asset license. Do conversion/preview locally regardless;
   verify applicable permission before committing derived art to this public repo.
   If unresolved, ship converter/integration with an original simple test fixture
   and a local sample-pack path; label the real-art publication gate explicitly.

Normal instrument builds consume committed, authorized generated data and require
neither API credentials, network calls, MP4 decoding nor image-generation services.
The offline converter must rebuild identical packed bytes from the same source,
recipe and pinned toolchain. Include a readme command for swapping the art and
rebuilding; runtime packs, SD/storage UI and firmware downloaders are out of scope.

## Size and performance boundaries

| One twelve-pose move | Raw masks only |
| --- | ---: |
| 96×96, 1-bit | 13,824 bytes (13.5 KiB) |
| 64×64, 1-bit | 6,144 bytes (6 KiB) |

Four 96×96 moves would be 54 KiB, but only one is required now. Metadata, alignment,
code and draw scratch are additional. Ghosting reuses masks, not a duplicate art
bank. Avoid decoding the whole bank into RGB framebuffers. Start with raw masks;
compression, spans and vectors are not required experiments for this scope.

Report actual generated data bytes, added static/runtime RAM and WASM/build-size
deltas separately. Account for clearing/compositing and actual display-transfer
bytes, not just the stored one-bit data. No assumed device budget from old fm1-base
notes, nominal chip RAM or host timing. Cap redraw work in the existing UI schedule;
rapid triggers coalesce into the newest snapshot. Prefer a smaller stage/resolution
if evidence warrants it; preserve the musical engine and never slow the arp to
accommodate graphics.

## Implementation sequence and acceptance

### 1. Establish baseline and prepare the one-move pack

- Fresh feature branch/worktree from main containing this plan (or apply the plan
  first if unmerged). Read root AGENTS/README/current build and vocoder contracts.
- Initialize pinned dependencies without advancing them; run `./build.sh --web`
  and `./tests/run_tests.sh`, retaining baseline receipts.
- Implement conversion, validate pack layout/determinism and inspect the two sizes.
  Choose twelve poses, resolution, anchor and seam with recorded reasoning.

### 2. Implement the arp-owned vertical slice

- Add bounded trigger telemetry, third ARP page, existing MODE/RATE controls and
  session-only style toggle. Implement both renderers and invalidation.
- Add focused tests for accepted/rejected/chord triggers, no held keys, hold/release,
  per-track isolation, hidden-page re-entry, skipped UI paints, wrap/reset, and both
  clock modes. Explicitly test a chord advances once and RNG/audio remain unchanged.
- Test CRISP replacement, GHOST layering/clearing and style/theme changes using small
  deterministic fixtures; validate no ghost survives a reset or leaks across tracks.

### 3. Verify browser behavior and musical non-regression

- Re-run root web build/tests and relevant existing Chromium baseline/vocoder suites.
  Preserve upstream goldens; compare identical musical inputs with dance hidden,
  CRISP and GHOST to show visuals do not change rendered samples/note events.
- Add actual-browser navigation/control acceptance: reach all ARP pages, select a
  track, hold notes/enable HOLD, alter rate/swing/probability, change style, leave and
  return, switch tracks, stop/restart clock, save/reload and release/panic cleanly.
  Existing save data remains compatible; session-only style may reset on reload.
- Inspect desktop/mobile and actual panel-size captures, including gaps between
  limbs, readable controls, seam and ghost contrast. Document layout/large-mode
  behavior; do not claim screen-reader parity absent in the existing app.
- Exercise busiest available groove + vocoder + rapid arp with both styles. Record
  host/browser measurements, redraw counts and memory/asset deltas. This is not
  evidence of target audio deadlines, SPI behavior or physical xruns.

### 4. Deliver reviewable implementation

Update README with exact controls, conversion/replacement command and demo recipe;
write verification/results including limitations and rights status. Open a scoped
implementation PR with exact tested SHA, commands, screenshots/art previews and
remaining device gates. No merge, device build/flash, release/tag or manual public
site deployment. Preserve dependency pins, existing UI/audio behavior and user data.
Physical memory/linker fit, display timing/tearing and worst-case audio deadline/xrun
validation are a separately authorized hardware milestone.

## Not in this round

Additional moves, move switching/choreography, random/reverse visual order, extra
cycle-rate/divider controls, generic modulation routing, fades/timed trails, vector
rigging/tweening, runtime video/segmentation, live pack loading, new generation runs,
new DSP, persistence redesign or hardware readiness claims.

## Copy/paste implementation handoff

> Implement `docs/ARP_DANCE_IMPLEMENTATION_PLAN.md` in `track-forge/bgirl-fm1`
> on an isolated feature branch from current main containing this plan; if the plan
> PR is still open, apply its docs commit first without merging the PR. Deliver one
> twelve-pose move from the linked Moltazine clip, an ARP DANCE page in the selected
> track's existing ARP family, existing MODE/RATE controls, and CRISP/GHOST styles.
> Use successful arp triggers (once per chord), not a UI timer or new sequencer;
> preserve RNG/audio/clock behavior and publish bounded race-safe display state.
> Build a reproducible offline mask converter, compare 96×96 vs 64×64, document the
> chosen framing/seam/provenance/rights, and avoid new generation. If art publication
> rights are unresolved, retain the sample locally and use an original fixture for
> public tests/builds while clearly reporting the gate. Follow the plan's validation
> and delivery sections; provide browser evidence, measured size deltas, exact test
> commands/results and a scoped implementation PR. Do not merge, build/flash a device,
> change pins, publish releases or manually deploy. Hardware performance remains
> unverified until a separately authorized device pass.

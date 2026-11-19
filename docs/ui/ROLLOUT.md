# Approved STUDIO rollout ledger

Current SYSTEM implementation stacks on [PR42](https://github.com/track-forge/bgirl-fm1/pull/42)
/ `c6cc2619153a07a5860eb0cd9832dbc27e197aa3`, branch `feat/studio-stage-rollout`.
Doug approved STAGE including the frame-clearance repair and authorized SYSTEM as
one complete set. No implicit merge or deployment; issue35 remains open.

## Coverage units

- [All 50 source page definitions](PAGES.md), [machine manifest](pages.json).
  Reproduce with `python3 tools/ui_inventory.py`; source names/bindings, no new IDs.
- EDIT1/2 are engine-defined: every selectable engine has its own labels, ranges,
  units, enum names and enabled controls. SAMPLE and SLICE engine variants now belong to the CUT completion pass; FM4 compatibility-only pages remain
  inventoried even when the build flag hides them. DRUM lane levels are CUT.
- HOME is not a PAGES row: CREW (approved), SCOPE, TRACKS, LEVELS (this STAGE pass).
  MIXER and GLO bank T1–4/T5–6; inactive two cells must stay inactive.
- Every page has normal/LARGE, FLAT/LINE, built-in light/mono and restored/custom
  palette variants. Approved typography/color semantics remain one design system.

## Delivery slices

| Slice | Coverage | Status / review gate |
|---|---|---|
| Foundation / PR36 | CREW STAGE, FM operator and operator2 LAB, drum STEP CUT, ENV | Doug approved; GitHub host checks green |
| Approved LAB / PR39 | Remaining envelope/LFO/modulation, synth EDIT/FILTER/VOICE, FM OP ENV, FX/INSERT/global sends and vocoder | [Native comparisons and verification](LAB_VERIFICATION.md); Doug approved |
| Approved CUT sequencing / PR40 | Melodic STEP/DETAIL, automation, SONG; DRUM DETAIL cross-check | [Native comparisons and verification](CUT_VERIFICATION.md); Doug approved |
| Approved CUT completion / PR41 | SAMPLE and SLICE EDIT1/2, SLICES, SLICER, drum LANES1/2 | [Native comparisons and verification](CUT_COMPLETION_VERIFICATION.md); Doug approved; no new editor or time stretching |
| Approved STAGE / PR42 | Other HOME views, MIXER/CLOCK, ARP/SCL/CHORD and ARP DANCE framing | [Native comparisons and verification](STAGE_VERIFICATION.md); Doug approved; existing dancer assets and choreography unchanged |
| SYSTEM / this stack | USER/PRESETS/PHRASES/PROJECT/TOOLS, menus, overlays and dialogs | [Native comparisons and verification](SYSTEM_VERIFICATION.md); Doug review pending |
| Browser shell | Align wrapper typography/color and focus/accessibility | Pending, after firmware coverage |
| Hardware | LCD contrast/legibility, transfer/tearing, linker fit/RAM/stack, audio deadlines | Not tested; separate authorized gate |

## Dynamic surfaces / state audit

These are not counted as separate pages and must not disappear behind a page total.
SYSTEM fixtures and interactions are recorded in [SYSTEM verification](SYSTEM_VERIFICATION.md).
Each retains its owning slice and human review gate:

| Surface | Cases to preserve | Owner |
|---|---|---|
| Global header/footer | Tempo widths/lock/rolling, REC own/other, count-in, internal/external clock, battery/USB, messages, page identity, long preset names | Shared; vinyl approved in PR39 |
| GLO hold | Track banks, mute/solo, ALL/LEARN/FILL/TAP, locked/held transitions | System; verified in this pass |
| ENV/LFO/FX/SEQ/REC hold layers | Momentary/latched maps, learn/assign/wait, return to underlying page, stale-pixel cleanup | System |
| MENU | DISPLAY/CONTROL/AUDIO/MIDI/SYSTEM, scrolling/end, calibration/about, custom palette editing | System |
| Naming | Cursor, character edit, long/full name, confirm/cancel | System |
| Confirmation | Save/erase/reset, cancel, PROJECT FULL, old-format refusal | System |
| Toast/help | Long hints, missing sample, MIDI learn, errors, no stale graph pixels | System |
| Preset/project browsers | Empty/occupied/favourite, selection, load/save/erase, bank names and capacity | System |
| Rhythm overlap | Cursor/playhead/event/accent/ratchet/muted lanes, 64-step banks; slicer patterns/playheads; sample source/missing/reupload | CUT fixtures and controls verified; Doug approved in PR41 |
| Sound edge cases | Disabled/missing source, signed/min/max/enums, frozen/unfrozen, zero sends, source trim/routing | LAB |

## Completion contract

Each slice supplies native240×240 before/after normal/LARGE, actual-browser knob
navigation, meaningful edge-state tests, unchanged audio goldens, measured assets
and redraw costs. Fix inherited layout defects on migrated surfaces; preserve an
exact broad-lint delta rather than blessing the remaining legacy findings.

Issue35 is done only when all source pages **and** dynamic surfaces have a recorded
review disposition, coverage evidence and no unexplained regressions. Browser/host
completion is distinct from physical-device readiness. Crowned animation remains
separate, as do sample-editing usability/features, DSP and musical changes.

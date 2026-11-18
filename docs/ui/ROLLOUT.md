# Approved STUDIO rollout ledger

Current CUT completion stacks on PR40 / `39a1b09`, branch
`feat/studio-cut-sequencing`. Doug approved the sequencing pass and authorized
finishing the remaining sample/slice/lane screens before STAGE as one set.
No implicit merge or deployment; issue35 remains open.

## Coverage units

- [All 50 source page definitions](PAGES.md), [machine manifest](pages.json).
  Reproduce with `python3 tools/ui_inventory.py`; source names/bindings, no new IDs.
- EDIT1/2 are engine-defined: every selectable engine has its own labels, ranges,
  units, enum names and enabled controls. SAMPLE and SLICE engine variants now belong to the CUT completion pass; FM4 compatibility-only pages remain
  inventoried even when the build flag hides them. DRUM lane levels are CUT.
- HOME is not a PAGES row: CREW (approved), SCOPE, TRACKS, LEVELS (pending STAGE).
  MIXER and GLO bank T1–4/T5–6; inactive two cells must stay inactive.
- Every page has normal/LARGE, FLAT/LINE, built-in light/mono and restored/custom
  palette variants. Approved typography/color semantics remain one design system.

## Delivery slices

| Slice | Coverage | Status / review gate |
|---|---|---|
| Foundation / PR36 | CREW STAGE, FM operator and operator2 LAB, drum STEP CUT, ENV | Doug approved; GitHub host checks green |
| Approved LAB / PR39 | Remaining envelope/LFO/modulation, synth EDIT/FILTER/VOICE, FM OP ENV, FX/INSERT/global sends and vocoder | [Native comparisons and verification](LAB_VERIFICATION.md); Doug approved |
| Approved CUT sequencing / PR40 | Melodic STEP/DETAIL, automation, SONG; DRUM DETAIL cross-check | [Native comparisons and verification](CUT_VERIFICATION.md); Doug approved |
| CUT completion / this stack | SAMPLE and SLICE EDIT1/2, SLICES, SLICER, drum LANES1/2 | [Native comparisons and verification](CUT_COMPLETION_VERIFICATION.md); Doug review pending; no new editor or time stretching |
| STAGE | Other HOME views, MIXER/CLOCK, ARP/SCL/CHORD and ARP DANCE framing | Pending; do not alter dancer assets or choreography |
| System | USER/PRESETS/PHRASES/PROJECT/TOOLS, menus, overlays and dialogs | Pending |
| Browser shell | Align wrapper typography/color and focus/accessibility | Pending, after firmware coverage |
| Hardware | LCD contrast/legibility, transfer/tearing, linker fit/RAM/stack, audio deadlines | Not tested; separate authorized gate |

## Dynamic surfaces / state audit backlog

These are not counted as separate pages and must not disappear behind a page total.
Each needs a native fixture and interaction check in its owning slice:

| Surface | Cases to preserve | Owner |
|---|---|---|
| Global header/footer | Tempo widths/lock/rolling, REC own/other, count-in, internal/external clock, battery/USB, messages, page identity, long preset names | Shared; vinyl approved in PR39 |
| GLO hold | Track banks, mute/solo, ALL/LEARN/FILL/TAP, locked/held transitions | STAGE/system |
| ENV/LFO/FX/SEQ/REC hold layers | Momentary/latched maps, learn/assign/wait, return to underlying page, stale-pixel cleanup | System |
| MENU | DISPLAY/CONTROL/AUDIO/MIDI/SYSTEM, scrolling/end, calibration/about, custom palette editing | System |
| Naming | Cursor, character edit, long/full name, confirm/cancel | System |
| Confirmation | Save/erase/reset, cancel, PROJECT FULL, old-format refusal | System |
| Toast/help | Long hints, missing sample, MIDI learn, errors, no stale graph pixels | System |
| Preset/project browsers | Empty/occupied/favourite, selection, load/save/erase, bank names and capacity | System |
| Rhythm overlap | Cursor/playhead/event/accent/ratchet/muted lanes, 64-step banks; slicer patterns/playheads; sample source/missing/reupload | CUT fixtures and controls verified; completion visual review pending |
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

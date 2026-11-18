# Approved STUDIO rollout ledger

Parent: PR36 / `3d125b2`. Doug approved the STUDIO first-screen direction on
2026-11-18 and authorized this stacked inventory + LAB pass. This PR targets
`plan/stage-lab-cut`, not main; do not merge the stack or deploy implicitly.
Issue35 remains the umbrella, not closed by this pass.

## Coverage units

- [All 50 source page definitions](PAGES.md), [machine manifest](pages.json).
  Reproduce with `python3 tools/ui_inventory.py`; source names/bindings, no new IDs.
- EDIT1/2 are engine-defined: every selectable engine has its own labels, ranges,
  units, enum names and enabled controls. The SAMPLE engine's existing waveform
  and slice UI belong to the later CUT pass; FM4 compatibility-only pages remain
  inventoried even when the build flag hides them. DRUM lane levels are CUT.
- HOME is not a PAGES row: CREW (approved), SCOPE, TRACKS, LEVELS (pending STAGE).
  MIXER and GLO bank T1–4/T5–6; inactive two cells must stay inactive.
- Every page has normal/LARGE, FLAT/LINE, built-in light/mono and restored/custom
  palette variants. Approved typography/color semantics remain one design system.

## Delivery slices

| Slice | Coverage | Status / review gate |
|---|---|---|
| Foundation / PR36 | CREW STAGE, FM operator and operator2 LAB, drum STEP CUT, ENV | Doug approved; GitHub host checks green |
| This stack / LAB | Remaining envelope/LFO/modulation, synth EDIT/FILTER/VOICE, FM OP ENV, FX/INSERT/global sends and vocoder | [Native comparisons and verification](LAB_VERIFICATION.md); Doug review pending |
| CUT | Melodic STEP/DETAIL, automation, song, existing SAMPLE/SLICES, SLICER, drum LANES | Pending; no new sample editor or time stretching |
| STAGE | Other HOME views, MIXER/CLOCK, ARP/SCL/CHORD and ARP DANCE framing | Pending; do not alter dancer assets or choreography |
| System | USER/PRESETS/PHRASES/PROJECT/TOOLS, menus, overlays and dialogs | Pending |
| Browser shell | Align wrapper typography/color and focus/accessibility | Pending, after firmware coverage |
| Hardware | LCD contrast/legibility, transfer/tearing, linker fit/RAM/stack, audio deadlines | Not tested; separate authorized gate |

## Dynamic surfaces / state audit backlog

These are not counted as separate pages and must not disappear behind a page total.
Each needs a native fixture and interaction check in its owning slice:

| Surface | Cases to preserve | Owner |
|---|---|---|
| Global header/footer | Tempo widths/lock/rolling, REC own/other, count-in, internal/external clock, battery/USB, messages, page identity, long preset names | Shared; vinyl change in this pass |
| GLO hold | Track banks, mute/solo, ALL/LEARN/FILL/TAP, locked/held transitions | STAGE/system |
| ENV/LFO/FX/REC hold layers | Momentary/latched maps, learn/assign/wait, return to underlying page, stale-pixel cleanup | System |
| MENU | DISPLAY/CONTROL/AUDIO/MIDI/SYSTEM, scrolling/end, calibration/about, custom palette editing | System |
| Naming | Cursor, character edit, long/full name, confirm/cancel | System |
| Confirmation | Save/erase/reset, cancel, PROJECT FULL, old-format refusal | System |
| Toast/help | Long hints, missing sample, MIDI learn, errors, no stale graph pixels | System |
| Preset/project browsers | Empty/occupied/favourite, selection, load/save/erase, bank names and capacity | System |
| Rhythm overlap | Cursor/playhead/event/accent/ratchet/muted lanes, 64-step banks | CUT |
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

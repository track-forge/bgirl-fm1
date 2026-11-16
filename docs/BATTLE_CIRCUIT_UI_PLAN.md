# Battle Circuit — BGIRL UI reskin plan

Status: **planning stub only; no implementation in this PR**.
Date: 2026-11-16.
Repository: `track-forge/bgirl-fm1`; target branch: `main`.
Inspected baseline: `7f36cdf7c09221342c46d0dd52f6c63bbdfcd48d` (includes Arp Dance PR #17).

## Direction

**A Fairlight recruited into an electro crew.** Crisp, stylish, urban, a little
hard. Battle Circuit combines Fairlight CMI's precise visual grammar with cassette
futurism and an independent electro record-label identity.

The guiding image: **expensive laboratory equipment; unauthorized basement party**.
Borrow strict grids, reverse-video headers, thin technical lines, generous dark
space and direct-looking waveform/envelope graphics—not historical screen density.
The instrument should look authored, not like a generic neon synthwave skin.

Doug selected Battle Circuit and supplied Fairlight references showing green
waveform waterfalls and a light-pen control matrix. References inform composition;
do not copy photos, Fairlight logos, proprietary fonts or artwork into the build.
This document is self-contained; private notebook access is not required.

## Scope and boundaries

Reskin both the **shared instrument framebuffer** and **browser emulator shell**.
Keep normal navigation, page order, knob assignments, hit targets, keyboard/MIDI
controls, musical behavior, startup/demo choices and save/restore behavior intact.
No new sequencer, DSP, vocoder analysis pipeline, project format or audio content.
No device build/flash, dependency-pin updates, merge, release or public deployment.
Browser/host evidence does not establish physical display or timing acceptance.

Current main has vocoder pages, optional vocal-bank UI and twelve-pose FLOORWORK
Arp Dance. Style these existing features; do not implement them again or replace
their assets. The public fixture is sufficient—no private recordings required.

## Visual system

Starting color targets (tune after native-size and contrast checks; not a promise
that RGB565 output exactly matches these RGB888 values):

| Role | Starting target | Use |
| --- | --- | --- |
| Screen background | `#091310` | Near-black, slight green cast |
| Phosphor | `#75F5BE` | Values, meaningful curves and selections |
| Primary text | `#D7F9E7` | Legible labels, brighter than secondary rules |
| Muted grid | `#315548` | Structural lines only; not essential text |
| Surround | `#D8D2BF` | Warm bone/concrete browser panel |
| Panel ink | `#151A17` | Heavy branding and shell labels |
| Warning / record | `#FF7847` | Sparse safety orange, paired with icon/text |

- Retire acid yellow as the defining accent. Mint, bone and black carry identity.
- Square corners, aligned cells, numbered sections and inverted selection bars.
- Crisp monospace values; condensed heavyweight BGIRL wordmark outside small
  parameter text. Reuse compatible bundled/licensed fonts where practical; a new
  font requires license/provenance and a generated-size/glyph-coverage check.
- Retain readable units, negative values, long names, disabled and error states.
- No fake CRT curvature, scanlines, distressed text, persistent glow or gratuitous
  animation. Browser-only phosphor effects are deferred, not an MVP requirement.
- Urban through typography, spacing and composition—not obligatory graffiti.
- Cassette-style boxed identifiers/stripes belong on the shell; `BG–01` is optional
  visual catalog text, never a replacement firmware identity or storage ID.

## Screen treatment

| Surface | Desired result |
| --- | --- |
| Shared header / parameters | Clear track/page identity, reverse-video focus, four aligned parameter/value areas; retain existing positional/control relationship rather than relocating controls to match a mood board |
| Sequencer | Rhythm-terminal grid, strong beat grouping, filled active steps; selection and playhead distinguishable when coincident, muted steps readable without color alone |
| Synth / envelopes | Useful sharp line drawings, bounded curves and clear numeric values; no decorative fake telemetry |
| Vocoder / vocal bank | Same parameter grammar, unmistakable routing/HOLD/MON and BANK MISS states; band-meter signature is deferred unless existing real data makes it cheap and UI-only |
| Arp Dance | Bold existing silhouette against restrained geometry, preserve 96×96 stage, CRISP/GHOST semantics, logical pose progression and LARGE labels |
| Menus / dialogs / names | Apply the same tokens, focus and square geometry; confirmations and save/load feedback stay legible |
| Browser shell | Bone panel, oversized black BGIRL identity, dark/mint display and restrained orange accent; preserve power gesture, labeled controls, reference/demo/bank sections and credits |

Treat the 240×240 framebuffer as an instrument display, not a desktop terminal.
Do not shrink controls or add dense labels just to resemble the references.
Preserve LARGE and supported style/palette options. Define the Battle Circuit
fresh-state appearance explicitly without overwriting restored user settings.

## Confirmed implementation touchpoints

Inspect these at the actual implementation base before editing:

- `felucca/web/emu/index.html`: browser shell styling, controls and responsive layout.
- `felucca/firmware/src/ui.c`, `ui_draw.c`: shared layout, cards, headers, LARGE modes
  and existing style behavior.
- `felucca/firmware/src/gfx.c` and `felucca/tools/gen_ui_palettes.py`: semantic
  colors, RGB565 and generated palettes. Reuse the token model, not per-page hex
  literals. Existing stored palette IDs are tagged/indexed; preserve IDs and
  migration mappings. Do not reorder palettes or force reset saved choices.
- Other `ui_*.c` renderers: graph, slice, menu, naming and page-specific exceptions.
  Trace dance/vocoder rendering from current code rather than assume file names.
- `felucca/tools/gen_aa_font.py`, bundled font assets and their notices: font path
  if the existing typography cannot meet the brief within the display budget.
- `felucca/tests/ui_test.c`, `ui_render.c`, `ui_render.py`, root browser smoke and
  dance suites: reuse existing render/state infrastructure for meaningful coverage.

Edit source/generators, not generated build output. Keep upstream attribution and
GPL/component notices. Do not broadly rename upstream symbols for branding.

## Implementation sequence

1. **Baseline and inventory.** Read AGENTS.md, README, provenance and current
   feature contracts. Fetch and verify fresh main, use an isolated feature
   worktree, initialize pinned submodules, build web and run the root suite before
   editing. Capture representative current screens and identify palette/default,
   font, redraw and responsive constraints. Record pre-existing failures.
2. **Establish visual grammar.** Implement shared tokens, header/cell geometry and
   browser shell. Produce real rendered sequencer and synth-page screenshots at
   native size plus desktop/mobile views. These are the first review artifacts;
   proceed with the chosen direction without requiring another design round.
3. **Complete the skin.** Apply the grammar across menus, mixer, dialogs, names,
   engine pages, vocoder, vocal bank and Arp Dance. Preserve interaction and all
   supported settings. Avoid new allocation/blocking in render/audio paths and
   expensive full-screen redraw regressions. No new telemetry solely for style.
4. **Verify and hand back.** Run web build, root suite and actual Chromium checks;
   inspect the visual matrix below. Add focused regressions for changed behavior
   boundaries (palette restore, selected/playhead states, LARGE clipping), not
   tests that merely assert CSS strings. Document exact commands, revision,
   screenshots, known limits and a short demo recipe in a review PR against main.

## Acceptance and evidence

- [ ] Recognizable bone/black browser shell and mint/near-black instrument; no
  remnants of conflicting skin conventions on reachable pages.
- [ ] Sequencer and synth screenshots demonstrate the direction at native 240×240,
  not only an enlarged glossy mockup.
- [ ] Desktop and 390×844 portrait checked: no horizontal overflow, clipped labels,
  obscured controls or lost pointer/keyboard focus. Touch targets remain usable.
- [ ] NORMAL/LARGE, menus/dialogs, long/negative values, active/selected/playhead,
  muted/disabled, record/warning, BANK MISS and dance CRISP/GHOST inspected.
- [ ] Essential text contrast checked (target 4.5:1); selected text remains readable
  on mint fills; states do not depend only on hue. Preserve browser labels/focus.
  Do not claim complete screen-reader parity for the framebuffer.
- [ ] Fresh state adopts the intended default; restored palette/style/LARGE/custom
  settings and saved projects are preserved. Retain existing palette IDs.
- [ ] `./build.sh --web`, `./tests/run_tests.sh`, `git diff --check` pass; audio
  goldens unchanged, no casual rebaseline. Actual browser smoke and targeted UI
  interactions pass, including key release, transport and save/load.
- [ ] No changes to demo musical content, DSP/arp timing/RNG, vocal routing,
  persistence schema, storage keys, pins or publishing configuration.
- [ ] Screenshot comparison, font/asset licensing and any output-size/redraw tradeoff
  documented. Label host/browser checks separately from untested physical behavior.

Do not run a hardware build to satisfy this plan. Human aesthetic approval remains
review, not something automated screenshots can certify.

## Implementation handoff (copy/paste)

Implement the Battle Circuit reskin in `track-forge/bgirl-fm1` following
`docs/BATTLE_CIRCUIT_UI_PLAN.md`. Create a fresh worktree/feature branch from current
`origin/main`; if this planning document is not merged, bring only its planning
commit onto the branch. Keep the planning PR docs-only and open a separate
implementation PR against main. Read AGENTS.md and current feature contracts.

Build the crisp Fairlight/cassette-futurist identity: near-black/mint 240×240 UI,
bone/black browser shell, restrained orange warnings/record, square technical
graphics and strong BGIRL typography. Preserve navigation, knobs, music, presets,
restored palette settings, LARGE, vocoder, vocal-bank and Arp Dance behavior.
Implement the complete scoped skin, not only CSS or palette changes. Use public
fixture assets and licensed/bundled typography. No new generation required.

Deliver real before/after screenshots, baseline/final build and test evidence,
actual-browser interaction checks, exact commit, runnable local demo instructions
and the separate review PR URL. Do not merge, deploy, flash, build device packages,
change dependency pins or introduce new musical features. If blocked, record the
specific failing command and remaining scope; do not claim visual or device proof
from compilation alone.

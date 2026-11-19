# Implementation handoff — BGIRL STAGE / LAB / CUT

Continue the **existing plan PR** on branch `plan/stage-lab-cut` in
[track-forge/bgirl-fm1](https://github.com/track-forge/bgirl-fm1).
Tracking: [issue #35](https://github.com/track-forge/bgirl-fm1/issues/35).

Read `AGENTS.md` and `docs/STAGE_LAB_CUT_IMPLEMENTATION_PLAN.md` in full, including
the linked concept board, pinned JIANT font reference, existing project docs and
validation gates. The plan is self-contained; no private notebook/chat is required.
Planning base is `c7116c5451bcfca9325ce71a71f7356ab1bd1b2c`; verify current main and
PR state before beginning. Continue this branch/PR, resolving ordinary drift without
losing newer work. Do not create a parallel implementation PR. If the plan has
already merged, explicitly identify that changed circumstance before choosing its
follow-up branch. Do not overwrite unrelated edits or advance engine/BSP pins.

## Authorized implementation milestone

Produce a genuinely new visual architecture, not a recolor: three real working
240×240 firmware screens—HOME/STAGE with existing selected-track CREW, synth/LAB
using an actual FM operator page, sequencer/CUT using the existing drum grid.
Use ENV as an early reusable-layout cross-check. STAGE/LAB/CUT are design contexts,
not a new navigation hierarchy. Preserve every physical knob assignment, input
behavior, musical feature and persistence contract.

Adapt JIANT's Chakra Petch Medium12 / Inter Tight SemiBold15 / Michroma18 typography
configuration through BGIRL's existing glyph generator. Preserve licenses, renderer
ABI and a genuinely readable LARGE mode; don't replace28px readouts with18px headings.
Use the plan's reference image as inspiration, not a pixel-perfect instruction.
Keep the strip canvas bounded and graphics outside audio; no fictional meters.

Start with baseline build/tests and screenshots, then implement in small logical
commits on this same PR. Run the plan's host/audio/native-layout and actual-browser
checks, inspect native screenshots, measure font/asset/RAM/redraw changes, and record
exact evidence in `docs/STAGE_LAB_CUT_VERIFICATION.md`. Preserve existing test coverage;
do not change audio goldens to hide a regression. Update the PR with implementation
summary, comparison images, tested SHA, results and a runnable local browser recipe.

**Stop for Doug's visual/playability review after these screens.** Do not roll out
through every page yet. Shared-font effects on other screens must still be tested.
No DSP/sequencer rewrite, crowned animation, new sample editor/time stretch,
slice-usability investigation, demo/sound changes, device build/flash, merge/release,
public/manual deployment or shared-preview service changes. Physical performance
remains untested unless separately authorized and actually measured.

Keep issue #35 open for the later full overhaul; use “Refs #35”, not “Closes #35”.
Deliver the updated existing PR plus exact verification and remaining review gates.

# bgirl-fm1 working instructions

Read `docs/BGIRL_BASELINE_PLAN.md` before implementing. The approved next milestone is:

> Felucca behavior intact, BGIRL electro defaults, working browser build.

The current source is still the initial independent fm1-base prototype. The migration
has not been implemented. `docs/INITIAL_BUILD_PLAN.md` and `docs/BUILD_STATUS.md`
describe that prototype and its evidence, not the new architectural direction.

- Establish a pinned, unmodified Felucca browser baseline before adding BGIRL defaults.
  Inherit Felucca's application and supported dependency/build layout; do not reconstruct
  its behavior in the old app or port it onto fm1-base just to preserve our current layout.
- Preserve the prototype checkpoint and Git history. Use an isolated migration branch/worktree;
  do not rewrite main. Dependency/layout changes required by this migration are intentional:
  document exact upstream revisions, provenance and reasons; avoid incidental updates.
- Keep track selection, sequencing, MIDI/clock, presets and save/restore behavior intact.
  Apply electro defaults at fresh initialization or explicit reset, not over restored user work.
- Keep vocoder integration and the silhouette dance page out of this baseline milestone.
- Keep work in small commits on a feature branch. Preserve source headers and notices.
- For prototype runtime changes, use `./tests/run_tests.sh`. For migration, establish
  upstream's applicable build/test baseline and add focused regressions for the BGIRL delta.
  Retire obsolete prototype assertions with documented reasons, not by hiding failures;
  retain applicable input, note-off, persistence and accessibility coverage.
  Documentation-only edits need whitespace/link/content checks, not runtime builds.
- No allocation, file I/O, logging, or blocking work inside the audio render path.
- Keep DSP/sequencing independent of UI. Validate output bounds and note-off/panic behavior.
- Preserve Felucca's GPL-3.0-only licensing and all component/asset notices; do not label
  the combined derivative MPL-only based on the old scaffold.
- Do not activate imported release workflows, merge PRs, or publish tags/releases/sites
  as part of this milestone. Review inherited deployment destinations and identities.
- Do not flash hardware. Device testing is a separate, explicitly scheduled step.
- Treat device package identity, storage layout and compatibility with prototype data as
  unreviewed until explicitly checked. Browser success is not device readiness.
- Label host, browser, device-build, and physical-device evidence separately.
- Record source/licensing/conversion details for any audio assets; no sampled commercial records.
- End with exact commands/results, commit, remaining blockers and a short demo recipe.

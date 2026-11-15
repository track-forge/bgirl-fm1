# bgirl-fm1 working instructions

Read `docs/INITIAL_BUILD_PLAN.md` before implementing. This is an independent app
on pinned fm1-base, not a platform fork.

- App changes belong in `app/`; avoid modifying `base/` or advancing its pin incidentally.
- Keep work in small commits on a feature branch. Preserve source headers and notices.
- Run `./tests/run_tests.sh` before changes and after relevant changes. Update template-specific
  assertions deliberately as behavior evolves; retain input, note-off, persistence and accessibility coverage.
- No allocation, file I/O, logging, or blocking work inside the audio render path.
- Keep DSP/sequencing independent of UI. Validate output bounds and note-off/panic behavior.
- Do not copy upstream release workflows or publish tags/sites as part of the initial build.
- Do not flash hardware. Device testing is a separate, explicitly scheduled step.
- Treat template package identity and storage addresses as unreviewed for this instrument.
- Label host, browser, device-build, and physical-device evidence separately.
- Record source/licensing/conversion details for any audio assets; no sampled commercial records.
- End with exact commands/results, commit, remaining blockers and a short demo recipe.

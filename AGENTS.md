# BGIRL working instructions

Read `docs/BGIRL_BASELINE_PLAN.md`, `README.md`, `docs/UPSTREAM.md` and
`docs/BUILD_STATUS.md`. Active source is now Felucca under `felucca/`, not fm1-base.

- Goal: scoped sample-fed vocoder per docs/VOCODER_IMPLEMENTATION_PLAN.md and
  docs/VOCODER_CONTRACTS.md; preserve Felucca behavior when disabled.
- Preserve upstream ancestry, GPL-3.0-only and component notices. Never advance
  engine/BSP pins incidentally. `felucca/.gitmodules` is historical upstream source;
  root `.gitmodules` maps the actual prefixed gitlinks.
- Keep BGIRL startup roles separate from `TRK_DEF`: the latter is also used by
  legacy project migration. Autosave restore happens after startup initialization.
- Reuse normal sequencer, track selection, sound loading and project operations.
  No remapped prototype controls, parallel sequencer or unrelated user-data format rewrite. Count-mapped additive
  vocoder parameters are allowed; preserve old saves.
- `./build.sh --web` generates upstream assets and builds WASM; then
  `./tests/run_tests.sh` runs the documented browser/host subset. Optional actual
  Chromium acceptance: `node tests/browser-smoke.cjs` with the README server running
  (set PLAYWRIGHT_MODULE and CHROMIUM_PATH when not installed in the usual place).
- No allocation, file I/O, logging or blocking in audio render. Project mutations
  stay outside render, using existing panic/load machinery. Check bounded output,
  note release and persistence for changes; do not update goldens casually.
- Old prototype assertions were retired with the prototype, not ported as controls.
  Preserve relevant input, MIDI/clock, note release, storage and UI coverage using
  upstream tests plus focused BGIRL tests. Upstream has labeled keyboard/panel
  targets, but no equivalent of the prototype spoken-screen reader: do not claim parity.
- Prototype assets remain historical at `4cc1e11`. New reference speech is generated
  via Moltazine/Crucible. Silhouette animation remains out of scope.
- Device package identity/storage/target compatibility remain unreviewed. No device
  build/flash, merge, release/tag or repository identity changes. GitHub Pages
  browser-preview deployment on main is explicitly authorized via host.yml; do not
  activate unrelated imported site/installer workflows.
  Root build intentionally exposes browser only. Do not activate imported workflows
  or inherited installer destinations. Label browser/host/physical evidence separately.
- Keep changes scoped, run `git diff --check`, end with exact revision/results,
  demo commands and remaining blockers.

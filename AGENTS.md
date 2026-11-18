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
  vocoder parameters are allowed. Six-track B6S1 is an explicitly authorized clean break:
  no legacy project/autosave migration; safe rejection/explicit overwrite. Preserve
  unaffected presets/samples/banks/settings. PER5 resets only incompatible learned mappings.
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
  via Moltazine/Crucible. Arp silhouette work is a separate, explicitly requested
  milestone scoped by `docs/ARP_DANCE_IMPLEMENTATION_PLAN.md`; it is not part of
  vocoder work. The planning PR itself authorizes no implementation or generation.
- Device package identity/storage/target compatibility remain unreviewed. No device
  build/flash, merge, release/tag or repository identity changes. GitHub Pages
  browser-preview deployment on main is explicitly authorized via host.yml; do not
  activate unrelated imported site/installer workflows.
  Root build intentionally exposes browser only. Do not activate imported workflows
  or inherited installer destinations. Label browser/host/physical evidence separately.
- Keep changes scoped, run `git diff --check`, end with exact revision/results,
  demo commands and remaining blockers.

## Six-track lane

- Six full tracks, 64 steps each, eight shared admitted voices. No light tracks.
- B6S1 stores explicit fields in the existing 3840-byte A/B object. PROJECT FULL
  must leave live music and the last committed copy intact; no truncation.
- `project_defaults.h` is version-frozen, not a regeneratable factory table.
- Motion addresses are nine-bit places in LE16 wire fields; MIDI learn has three
  track bits and retains16 slots. Never restore two-bit track maps as new bindings.
- Selection follows T1–4 / T5–6 banks for GLO/HOME LEVELS; two inactive cells do
  nothing. MIXER retains its selected-track LEVEL/PAN/REV/MUTE knobs.
- Editor uses `7D 46 36`, not fixed-four `7D 46 4C`. Run six_codec/six_track and
  editor host tests plus `tests/six-browser.cjs`. See six-track verification.

## Vocoder clarity lane

- Read docs/VOCODER_CLARITY_CONTRACT.md and VOCODER_CLARITY_VERIFICATION.md.
- BANDS 0/1/2 = 8/12/24. RANGE 0 preserves legacy REL; RANGE 1 is generated 5–500ms.
- B6S1 accepts 126/127 counts via explicit engine/motion ID adapter. Do not edit
  frozen default values. Log sound presets use versions 9/10, preserving VOCALS IDs.
- Regenerate time/coeff/carrier headers with tools (root web build does this).
  No engine/BSP pin edits. BG CARR SAW appends ANALOG preset 12.
- tools/clarity_audition.py produces ignored local audio/gallery/measurements;
  technical success never means Doug listening approval or physical-device fit.

- Live vocoder demos now use TRIO / FAT BASS (engine6/preset0), per Doug's listening.
  Clarity carrier IDs:0 saw,1 archived GRAIN for old gallery reproduction,2 TRIO.
  Browser exposes saw/TRIO only. Do not relabel or re-render the archived gallery
  as TRIO without a new explicit rendering task.

## CREW HOME lane

- Read docs/CREW_ACTIVITY_CONTRACT.md and CREW_HOME_VERIFICATION.md.
- HOME IDs0/1/2 stay SCOPE/TRACKS/LEVELS; CREW appends3. Fresh settings choose
  CREW; unmarked older settings adopt it once ([15][10] bit7). Thereafter preserve
  explicit HOME choices and unknown-value SCOPE fallback; keep ANIM IDLE bit0.
- One selected dancer; `NTRK` associations share five clip banks. No grid or six
  background choreography loops. LARGE keeps the graph and normal engine knobs.
- Core0 collects accepted activity; publish once after worker join/recovery. UI
  reads one whole word, never mutable voices. External clock advances are GRID
  units, not samples. Existing ARP telemetry and musical RNG are independent.
- CREW uses22-step ping-pong (0..11,10..1) per four beats; ARP DANCE unchanged.
  Windmill follows horizontal subject center at fixed scale/floor; worm samples
  only1–4s. Per-clip recipes are in assets/crew/sources.json.
- Four new clips and public-use permission: assets/crew/README.md. Offline repeat:
  tools/convert_crew.py ORIGINALS DEST. Do not overwrite original FLOORWORK.
- Root tests include tests/run_crew_tests.sh; real browser tests/crew-browser.cjs.
  Technical acceptance is not art/seam approval or device fit. No manual deployment.

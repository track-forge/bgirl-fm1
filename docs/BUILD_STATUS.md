# BGIRL Felucca baseline — verification

2026-11-15. **Browser milestone implemented; not installable/device-ready.**
Tested source revision: **`1bc9d29ebeae69c8bb045e7ca36c541616245fc1`**.
Later commits only record evidence/screenshots. See [provenance](UPSTREAM.md) for
upstream/dependency pins and [README](../README.md) for complete build/demo commands.

## Revisions and tools

| Component | Exact selection |
| --- | --- |
| Felucca | v1.8.1 → `f4df2964649f301ba5abd0ec6000110068672f25` |
| Felucca-Engines | `d0260074ce100a47675814b7f38d61f11176c926` |
| felucca-bsp | `5942e68b89ab9ccd23f2c541d712ca38fe635f11` |
| Prototype preserved | `4cc1e11e50ffb1d2a06a50547780b9e828a8b15f` (ancestor) |
| Unmodified import/control | `59fb7ad` (upstream tree under felucca/) |
| Emscripten | 6.0.10, compiler `d6c521a7f05449857c76bd99e396895583cf2083` |
| emsdk checkout | `708dcbcc9ee23bc7d2dad44fc90db377e38b771c` |
| Native / Python | GCC 15.3.1 (20260722 Red Hat); Python 3.14.5 |
| Asset generators | Pillow 12.1.1; fonttools 4.68.0 |
| Browser test tools | Node 24.22.1; Playwright Core 1.59.1; Chromium 149.0.7827.55, Linux headless |

Final WASM: **782271 bytes**, SHA-256
`d29c1e37d7dce2ddb8926d8929f64cb986e0fa4dc7788c2ef8b5ae12acf7d511`.
Compiler/tool-specific artifact hash, not a claim of identical binaries across systems.

## Results

| Area | Observed evidence |
| --- | --- |
| Unmodified browser baseline | PASS: upstream asset generation, `sh felucca/web/emu/build.sh`, `node felucca/web/emu/emu_test.mjs felucca/build/emu/felucca.wasm`; Chromium switch-on, keys, ALGORITHM, PLAY/STOP, screenshot, zero page errors |
| Final build | PASS: source emsdk environment; `./build.sh --web` (actual root wrapper) |
| Final host/browser runner | PASS: `./tests/run_tests.sh` |
| Unchanged upstream tests | PASS: storage, user presets, input debounce, TRS MIDI parser, legacy/current project formats, persistence, UI, MIDI control/clock, long-run MIDI drift, swing; backup web tests |
| Audio preservation | PASS: **98 upstream golden renders unchanged**, zero health/voice/routing failures; native and WASM heavy-song output identical for **88200 frames** |
| Upstream WASM test | PASS: boot/splash/LEDs, silent rest, keys, MIDI note queue, knobs, transport, preset saved via panel and restored into a fresh WASM instance, MIDI CC knob modes, deterministic heavy song |
| BGIRL delta | PASS: four sounds, editable demo and finite/bounded 8-second mix, ALGORITHM track selection, sound+step project roundtrip, changed autosave restored after startup initialization, blank template, repeatable explicit demo load, held-key track switch/release and final silence |
| Actual Chromium | PASS: user gesture starts AudioContext, demo signal, stop/tails decay, four distinct track screens, note signal on all three synth roles, panel sound edit/preset save, browser reload retains sectors, confirmed blank/demo operations, cancellation, saved sectors not erased; zero page errors |
| Visuals | Desktop 1300×1000 and portrait 390×844 screenshots inspected. No horizontal page overflow; portrait is tall and requires vertical scrolling, inherited controls retained |
| Human listening | **NOT VERIFIED**: signal/musical parameter evidence only, no human audition/intelligibility/taste approval |
| Physical browser Web MIDI | **NOT TESTED**: no attached controller; queue/CC and USB/TRS/clock host tests passed, unchanged browser channel-message routing retained |
| Hardware / target | **NOT ATTEMPTED**: no device compile, flash, physical USB/TRS sync, RAM/flash map, latency, CPU deadline, xrun or new-chip compatibility proof |

The runner intentionally selects upstream tests that do not require a device
package. Upstream `felucca/tests/run_tests.sh` as a whole additionally requires a
JieLi/AC79 SDK firmware build, OTA/loader packages and target disassembly. Those
checks, optional reference-library comparisons, and the entire upstream matrix
are **not claimed as run**. GCC renders validate audio/health; instruction-budget
counters report zero on this host, so **no target CPU-budget claim** is made.
Upstream tests/goldens are byte-for-byte unchanged (compare `v1.8.1:tests` with
`HEAD:felucca/tests`); no failures were waived or golden baselines rewritten.

### Reproduce actual-browser acceptance

With the README's server running:

```sh
# Use an installed Playwright/Chromium, or set these paths for this workspace:
PLAYWRIGHT_MODULE=/home/dev/.openclaw/npm/projects/askjo-camofox-browser-2f55486f67/node_modules/playwright-core \
CHROMIUM_PATH=/home/dev/.cache/ms-playwright/chromium-1228/chrome-linux64/chrome \
node tests/browser-smoke.cjs
```

Screenshot/evidence output defaults to ignored `tmp/browser/`. Headless signal
values vary with capture timing. No real audio output device was auditioned.
Build-only CI is checked separately in the PR; local passes are not hosted CI.

## Review-sensitive choices / remaining work

- v1.8.2 no longer has upstream browser support; v1.8.1 is the reproducible browser
  baseline. This is an independent edition, not an upstream-supported new platform.
- Factory `TRK_DEF` remains unchanged because legacy imports use it; BGIRL has a
  separate startup table. Normal CLEAR ALL remains Felucca's blank factory layout.
  Use **Blank electro template** for the four BGIRL sounds with empty sequences.
- Browser storage key is separate; no automatic stock/prototype data import. Same
  Felucca formats and save machinery, not the prototype's BGR1 record/storage layout.
- Browser edition retains labeled keyboard/slider/button targets. The prototype's
  spoken screen announcements are not available in this upstream UI; equivalent
  musical screen-reader access remains an accessibility follow-up, not a parity claim.
- Track 4 VOX LEAD is explicitly a **robot synth placeholder**. No vocoder or speech
  assets ship; human sound curation and later vocoder integration remain separate.
- Device identity, install destinations, storage compatibility, GPL source packaging
  and actual hardware performance need their own review before distribution.
- Vocoder findings remain in VOICE_SPIKE.md; dance animation remains issue #2.
- No merge, device flash/build, release/tag push, site deployment or repository rename.

## Local receipts

- `/tmp/bgirl-felucca-baseline-{build,test}.log`, control screenshot/build at
  `tmp/baseline/`; initial generation log `/tmp/bgirl-felucca-generate.log`.
- `/tmp/bgirl-felucca-final-{build,tests,browser}.log`, `tmp/browser/results.json`.
- Committed current screenshots: `docs/img/browser.png`, `docs/img/browser-mobile.png`.
- Initial exploratory failures are not hidden: missing fontTools was installed;
  changing factory TRK_DEF broke fixtures/legacy assumptions, so that approach was
  removed and all upstream tests restored unchanged. Focused tests were corrected
  for upstream mono-legato and one-track-per-encoder-event semantics. Final runner
  and actual browser checks pass; prototype controls were not reintroduced.

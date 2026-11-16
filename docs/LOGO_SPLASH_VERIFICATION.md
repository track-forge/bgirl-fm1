# Logo splash and clean browser panel

Base: main `c2c8fc74a93334a266b8e2a46a5b187baf28789c` (includes the restored
two-bar demo). Date: 2026-11-16. Host/browser evidence only.

## Changes

- The shared 240×240 instrument splash now displays the existing PR #19 wordmark,
  with a small Felucca version/maker credit below it. This is the firmware's actual
  first framebuffer, not an HTML overlay. Existing boot timing and held-key guard
  are unchanged; it transitions normally into the instrument.
- Removed the optional **BG–01 / Battle Circuit** inscription from the SVG synth
  panel and the redundant catalog text above it. The selected page-header wordmark,
  source links, credits, layout and all 49 panel hit targets remain.
- The unchanged source logo is resized to 224×145, composited on bone and packed
  as 4-bit indices into a 16-color RGB565 palette: **16,272 bytes**. GREY and MONO
  retain grayscale behavior. No new artwork generation or source PNG edits.
  [Artwork provenance and conversion](../assets/branding/README.md) remain separate
  from the GPL code license; no additional rights are asserted.
- Startup draws five bounded image strips into the existing canvas, plus the two
  credit lines. No allocation, new framebuffer, audio/DSP/demo, saved setting,
  storage format, boot-delay, dependency pin or deployment configuration change.
  CI's existing browser step now also runs the focused splash check.

## Screenshots

| Before | After |
| --- | --- |
| ![Original splash](img/logo-splash/before.png) | ![Logo splash](img/logo-splash/after.png) |

Both images are native 240×240 framebuffer captures: before from the baseline
WASM boot in Node, after from the first actual Chromium AudioWorklet framebuffer.
The after image is exported from that captured packet so slow screenshot runners
do not miss the brief splash; no test-only boot delay or renderer is used.

[Desktop panel](img/logo-splash/desktop.png) · [390×844 portrait](img/logo-splash/mobile.png).

## Local verification

```sh
./build.sh --web
./tests/run_tests.sh
python3 tests/test_package_logo.py
git diff --check
```

- Baseline and final full host suites pass. **98 audio goldens unchanged**;
  **88,200 native/WASM frames bit-identical**. Current two-bar demo, vocoder,
  vocal-bank, persistence, MIDI/keys, transport and dance tests remain intact.
- Two packaging checks pass: deterministic bounded packing and refusal of an
  unreviewed source-image checksum change. Generated headers stay out of git.
- Updated native layout audit: **233 scenes × 11 palettes**, including LINE/LARGE
  and value sweeps; **0** clipping/overlap/invisible-ink/grayscale findings,
  **0** alignment errors across **3,904,736** measurements. The obsolete text-square
  splash assertion was replaced with exact logo placement/pixels and corner margins;
  actual credit text/version and normal text-layout lint remain checked.
- Actual Chromium: `tests/splash-browser.cjs`, `tests/battle-browser.cjs` and
  `tests/browser-smoke.cjs` pass. **32,480 first-frame logo pixels match**, normal
  splash→instrument transition, no inscription at desktop/portrait, all **49** panel
  targets retained, no horizontal overflow, visible keyboard focus/edit, real audio,
  release/STOP, preset save/reload and demo/blank confirmations. **Zero page errors**.
- Local public-bank WASM: **902,450 → 918,843 bytes** (+16,393).
  SHA-256: `acc7876bca37f15d146034fb14e0f893efb438433323a5227a768c9dc28892c8`.
  No target RAM/timing/physical display claim; no device build or flash performed.

For the optional native layout audit, use the `ui_render` compile/run commands in
[BATTLE_CIRCUIT_VERIFICATION.md](BATTLE_CIRCUIT_VERIFICATION.md), with the new source.

## Demo / browser commands

```sh
python3 -m http.server 8782 --bind 127.0.0.1 --directory felucca/build/emu
# In another terminal (set PLAYWRIGHT_MODULE / CHROMIUM_PATH if needed):
export BGIRL_URL=http://127.0.0.1:8782
node tests/splash-browser.cjs
node tests/battle-browser.cjs
node tests/browser-smoke.cjs
```

Open http://127.0.0.1:8782 and Switch on to see the brief logo splash, then PLAY
for the unchanged demo. Remote browser use needs HTTPS or a localhost SSH forward.
No existing preview service, tailnet proxy or public site was changed. This PR is
not merged or deployed; physical-device acceptance is still untested.

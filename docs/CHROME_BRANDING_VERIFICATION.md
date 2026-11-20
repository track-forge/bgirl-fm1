# Chrome branding (#47): review plan and evidence

## Scope and plan

1. Preserve the exact selected JPEG and document truthful provenance/rights.
2. Decode a local browser/README PNG without recoloring or changing aspect ratio.
3. Reuse the existing indexed boot path; resize the full name, enlarge separate
   FM1 below, and keep the record/crown. No engine, boot timer, DSP or storage edits.
4. Build browser WASM, run root host suite, inspect real Chromium at desktop/mobile
   and its first native240×240 boot framebuffer, then open a main-target PR.

## Visual review

README artwork (same decoded master as the browser header):

![B-GIRL pink chrome with lavender bevels, mint highlights and separate FM1](../assets/branding/bgirl-fm1-wordmark.png)

Native first AudioWorklet framebuffer,240×240 (not a device photograph):

![Native boot framebuffer](img/chrome-boot-native.png)

[Desktop browser](img/chrome-desktop.png) · [Mobile390×844](img/chrome-mobile.png)

## Cost and boot behavior

The unchanged master is161,130 bytes; PNG is878,674 bytes (browser/README only).
Firmware references only the generated16-color224×104 indexed header:11,696
bytes including both palettes, versus16,288 previously. The32-row renderer still
uses its existing canvas,64-byte local color cache, one sync, and no allocation.
There are23,296 logo pixel visits instead of32,480. No new delay/timer was added.
Opaque near-black purple comes from the selected artwork; outside the logo is the
current screen palette. GREY/MONO use existing conversion. Source inspection and
host/browser proof do not establish device flash/RAM maps, timings or readability.

## Limitations

Human visual approval (including native-size record/crown and FM1) is pending.
No device build/flash, performance, physical display or hardware-fit claim.
No merge or shared/deployed-site update authorized: Pages remains at its existing
main revision until a separately authorized merge. Local project-prefix loading
is evidence for this PR, not proof the public Pages site serves this change.
Artwork/model commercial reuse and chrome-reference rights remain unresolved;
see [provenance](../assets/branding/README.md). No permissive license is invented.
MCP-backed work was unavailable in this run; implementation, browser and GitHub
verification use local tools/gh, not a claim of configured MCP success.

## Checks

- `./build.sh --web` with Emscripten6.0.10: PASS (1,072,885-byte WASM).
- `python3 tests/test_package_logo.py`: PASS, deterministic packing and changed
  master rejection; opaque alpha and224×104 size checked.
- `node tests/splash-browser.cjs`: PASS, all23,296 logo pixels and34,304 outside
  pixels in first actual AudioWorklet framebuffer checked, centered at(8,68),
  original boot transition,49 panel targets, desktop1300×1000/mobile390×844,
  no horizontal overflow, no page errors.
- `node tests/browser-smoke.cjs`: PASS, local PNG1952×544 loads, user-gesture
  AudioWorklet, demo peak0.11678, four distinct track screens, saved-sector flow,
  zero page errors. Human listening/physical MIDI not performed.
- `node tests/browser-shell.cjs`: PASS, desktop/mobile/320px/landscape/zoom sizing,
  control/input release, focus, contrast, reduced motion; no external requests.

Browser commands used `BGIRL_URL=http://127.0.0.1:8787/bgirl-fm1/` against the
built local artifact with Playwright Core1.59.1 and Chromium149. Screenshots are
unretouched full-page captures; native boot screenshot is exact framebuffer size.
To repeat, serve `felucca/build/emu` using the README recipe and set BGIRL_URL,
PLAYWRIGHT_MODULE and CHROMIUM_PATH as needed. No live shared service was changed.

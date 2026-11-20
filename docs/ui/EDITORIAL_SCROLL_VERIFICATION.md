# Editorial listening scroll · issue #52

Stacked on PR #53, base `issue/51-playable-flyer` at
`b28b629a32573888ee79ae105ef7351fddb9db5b`. Foundation verified by its local
build/tests and rechecked here; GitHub reports no checks on its current head.
Merge #53 first, then retarget this PR to main if needed. No merge/deploy performed.

## What ships

Root flyer remains the first playable interaction. Below it: vocoder dry/pitched
syllable, two factory bass patches, sample alone/drum context, then arp origin.
STAGE → LAB → CUT is a captioned still storyboard: existing real floorwork pose,
readable real patch details, same syllable with saw/TRIO carriers.
No new breaking artwork, robot-mouth hero, animation, scroll audio or scroll trap.

`tools/render_editorial.c` includes the production host instrument without changing
it. It renders the same first G6 chop through dry bypass, BG CARR SAW, and
TRIO / FAT BASS; BG SUB CLEAN and BG SUB DRIVE use the same MIDI 34/39 figure,
velocity, timing and mixer level. Sampler context restores a normal drum pattern.
`tools/editorial_assets.py` applies fixed whole-excerpt RMS matching within each
comparison, no limiter, checks finite/bounded output, and emits four-second mono
44.1kHz WAVs plus actual 240×240 EDIT 1 screen captures. Build generates public
assets from the authorized bundled G6 source, no private inputs or downloads.
Existing ARP DANCE capture/provenance is reused, not a vocoder animation.

All excerpt playback requires a button gesture, is mutually exclusive, has a
persistent Stop, and stops on backgrounding. Live flyer and excerpts stop one
another. No storage operations. No firmware, worklet, studio, pins or save format
changes. README opens with the vision/plain description and both Pages doors;
browser-only/do-not-install warning and linked Roots & credits retain notices.

## Local verification

- `./build.sh --web` with Emscripten 6.0.10: complete Pages artifact including
  host-DSP editorial rendering.
- `node tests/editorial-browser.cjs`: seven real audio excerpts start; keyboard/touch,
  Stop, exclusive audio/live flyer handoff, actual loaded stills, project-prefix
  links, desktop/mobile reduced motion, no overflow/page errors, storage intact.
- `node tests/flyer-browser.cjs`: AudioWorklet beat/manual answers/Stop/restart,
  touch/keyboard, audio-failure fallback, studio routes and unchanged storage.
  Existing fallback assertion now selects the first of multiple editorial images.
- `node tests/flyer-wasm.mjs`: manual answer differs from beat at 0.25/0.75/1.5s,
  finite bounded output.
- `node tests/song-browser.cjs`: actual download/upload, refresh, fresh profile,
  invalid/expired/missing-bank atomicity, arrangement audition, unchanged slots,
  FULL refusal, mobile/storage-denied paths; no page errors.
- `git diff --check`.
- Screenshots: [desktop](editorial/desktop.png), [mobile](editorial/mobile.png).
  Mobile inspected at 390×844; readable real display crops, static captions,
  no horizontal overflow or scroll trap.

Repeat under a repository prefix using the README server with a parent directory
containing a `bgirl-fm1` symlink to `felucca/build/emu`:
set `BGIRL_FLYER_URL=http://127.0.0.1:8852/bgirl-fm1/` and
`BGIRL_URL=http://127.0.0.1:8852/bgirl-fm1/play/`.
Set PLAYWRIGHT_MODULE / CHROMIUM_PATH as documented in BUILD_STATUS.
Editorial acceptance is also added to Pages CI.

Production WASM is byte-identical to the verified foundation artifact:
SHA-256 `087c0df03a32e8bf119c5c88691bf5a8f87290c2b66106c706d1fa43ea02bd1d`.
No full engine rerun was needed for this root-only presentation/offline renderer change.

## Limits

Local built-artifact Chromium evidence, not deployment or requesting-viewer
confirmation. Human listening/intelligibility/art judgment remains pending.
No hardware, Firefox/Safari, merge or deploy claim. Hosted CI is reported
separately in the PR. Inherited MCP configuration was incomplete; verification
used local shell/Chromium and GitHub CLI, not omitted MCP capabilities.

# Playable flyer / direct studio

Issue #51. Browser-only implementation of the notebook marketing spec at
`1bf6762`. Editorial scroll and README rewrite (#52) are not implemented.

## Routes and music

The Pages artifact's root is the lime typographic playable flyer; `play/`
contains the unchanged studio shell and all its relative assets, imports and
reference demos. README's live studio link points directly there. Both routes
work beneath `/bgirl-fm1/`. The studio retains the same origin-wide
`bgirl-felucca-v1-flash` key, project examples, import/export and editing.

Start beat makes a fresh disposable instrument using the existing clarity
arrangement: ordinary four-bar drum/bass/carrier tracks, with automatic voice
steps and voice motion removed. Answer immediately plays the publicly bundled
G6 whole phrase through SLICE and the real 24-band sample-fed vocoder (BG CARR
SAW, fully wet). No audio file overlay or alternate JavaScript sequencer.
Answer timing is not quantized. Stop uses normal transport/all-off and suspends
the context, silencing effect tails as well. Restart resets the short arrangement.
Leaving the tab also stops it. No microphone, MIDI setup, autoplay, storage read
or storage write occurs on the flyer. Flash messages are discarded.
Startup failures retain the real instrument screenshot and direct studio link.

The wordmark is lime live typography, not a generated artwork or an alteration
of the studio's existing chrome branding. The still is a capture of the actual
studio panel; the flyer contains no animation.

## Local artifact evidence

- `./build.sh --web`: Emscripten browser build.
- `node tests/flyer-wasm.mjs`: compares no-answer and manually answered audio at
  0.25, 0.75 and 1.5 seconds; answer difference energy 190.67 / 207.28 / 233.93,
  bounded peaks 0.153 / 0.164 / 0.1645. Transport stops in each case.
- `node tests/flyer-browser.cjs`: real Chromium AudioWorklet under the project
  prefix; no audio before gesture, keyboard answers at different times,
  stop/suspend/restart, mobile touch, reduced motion/no horizontal overflow,
  byte-identical preexisting studio storage, studio navigation/assets and
  deliberately failed WASM fallback. Beat peak 0.0711; no page errors.
- [Desktop](flyer/desktop.png) / [mobile](flyer/mobile.png): built root preview.
  Screenshots are local headless browser evidence, not deployed-site evidence.

CI runs the new flyer browser/WASM checks and retains every existing studio
browser check, now at `/bgirl-fm1/play/`. Only main can deploy; this PR does not.

## Repeat

```sh
./build.sh --web
mkdir -p .powderline/site
ln -s ../../felucca/build/emu .powderline/site/bgirl-fm1
python3 -m http.server 8841 --bind 127.0.0.1 --directory .powderline/site
# another terminal, with Playwright available:
BGIRL_FLYER_URL=http://127.0.0.1:8841/bgirl-fm1/ node tests/flyer-browser.cjs
BGIRL_URL=http://127.0.0.1:8841/bgirl-fm1/play/ node tests/song-browser.cjs
```

Open the root, Start beat, Answer on different beats, Stop, restart; open
`play/` directly and import/export/edit a song. Returning studio sessions keep
their previous saves. The flyer deliberately cannot export or save its jam.

## Limits

Signal/path verification is not human intelligibility or musical approval.
G6 is the existing authorized public voice, with its existing provenance.
Chromium desktop/mobile emulation is tested, not physical touch devices or
Safari/Firefox. No hardware build/flash, deployment, new art generation,
merge or #52 work. No requesting-viewer rendering confirmation is claimed.

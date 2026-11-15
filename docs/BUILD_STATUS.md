# Initial electro slice — build status

Date: 2026-11-15. **Reviewable host/browser prototype, not device-ready firmware.**
Implementation branch: `feat/initial-electro-slice`; review PR linked in the handoff.
Human listening approval is outstanding; signal output is not proof of intelligibility.

## Revisions / tools

- Baseline: main's initial plan/scaffold; baseline reproduced before modifications.
- Branded baseline: `eff76b1`; original-speech voice spike: `60fe195`.
- Groove/browser/tests: `7c2b09b`; bounded MIDI intake: `712d241`.
- fm1-base: `c8e05997ce970875038156d2bbd69e712eac1543` (unchanged).
- BSP: `18eadadb8c364abd026accd2b1ea1c14ca347f06` (unchanged).
- GCC `15.3.1 20260722 (Red Hat 15.3.1-1)`; Python `3.14.5`; Pillow `12.1.1`.
- Emscripten **6.0.10**, compiler revision `d6c521a7f05449857c76bd99e396895583cf2083`.
  emsdk checkout `708dcbcc9ee23bc7d2dad44fc90db377e38b771c`;
  SDK resolved to `666337b525e673e769121856d175f6f52b8ead64`.
- Browser: Playwright Core `1.59.1`, Chromium `149.0.7827.55`, Linux headless.

## What exists

- Fixed 16-step electro pattern, synthesized kick/snare/closed hat and mono bass.
- Integer audio-frame fractional clock (no rounded step length or UI-frame timing).
  60–180 BPM; preserves phase through tempo changes; stop/start resets to step 1.
- Start/stop, bass tone, drum mute/return, selected bass/voice keyboard part,
  smoothed master gain, bounded 24-bit mix and panic/release behavior.
- 12-band **live** sample-fed mono vocoder with original dry speech; not a rendered
  hook. No automatic sequenced reply until human audition approves it.
- Branded screen with transport/tempo/part/step/mute state, original platform
  keyboard/pointer controls, screen-reader announcements and helpful browser guide.
- Read-only host CI only; no release/tag/site workflow.
- DSP owns its state; UI sends scalar requests. Incoming MIDI processing is capped
  at 64 messages/render block. No heap, I/O, logging, locks or decoding in DSP.

## Commands and evidence

| Layer | Command | Result |
| --- | --- | --- |
| Baseline host | `./tests/run_tests.sh` before changes | PASS; original template input/note/MIDI/Bluetooth/save/accessibility assertions |
| Final host | `./tests/run_tests.sh` | PASS: retained baseline, focused DSP suite and electro integration script |
| DSP timing | included in test runner | PASS, 60 seconds each at 60/120/123/137/180 BPM with irregular block sizes; tempo change preserves fractional phase |
| DSP behavior/bounds | included in test runner | PASS restart, mute/return, full phrase at all 128 MIDI pitches with simultaneous drum/bass envelopes, release/mode switch/panic; ±7470000 maximum permitted output |
| Input integration | `app/tests/electro.txt` via runner | PASS transport, tempo/tone clamps, mute/return, voice input, outgoing original-note offs across octave/part changes, CC123 and HOME panic, screen reader |
| Sanitized DSP | command below | PASS AddressSanitizer + UBSan + float-cast-overflow, abort-on-first-error; app DSP only, not entire platform |
| Voice comparison | `./tests/render_voice.sh` | PASS finite/bounded output and stop check at 110/164.8138/220 Hz; WAVs saved |
| Web compilation | `source ~/.local/share/bgirl-emsdk/emsdk_env.sh; ./build.sh --web` | PASS pinned 6.0.10; expected missing device install/screenshot metadata warning and missing built-in reader voice notice |
| Browser | command below, local server | PASS user click starts AudioContext; actual keyboard/pointer keys/knobs/ARP/FX/PRESETS/HOME; signal after play, silence after release/panic; zero page errors |
| Screenshots | browser smoke at 1300×1150 and 390×844 | Inspected; mobile panel scrolls internally (no page overflow), portrait use is cramped; landscape recommended |
| Demo | `app/build/host/bgirl_host app/tests/demo.txt app/build/host` | PASS; 30.00018 s stereo 44.1 kHz, −7.44 dBFS peak, zero clipped PCM samples |
| Source hygiene | `git diff --check`, submodule status | PASS; base/BSP unchanged |
| Device build / physical | none | **NOT ATTEMPTED** |
| Listening / musical taste | none | **NOT VERIFIED by human ears** |

```sh
cc -std=c99 -O1 -g -fsanitize=address,undefined,float-cast-overflow \
  -fno-sanitize-recover=all -fno-omit-frame-pointer -Ibase/include \
  tests/electro_test.c app/dsp/electro.c app/dsp/vocoder.c -lm \
  -o app/build/host/electro_sanitized
app/build/host/electro_sanitized

# With the README's localhost server running:
PLAYWRIGHT_MODULE=/home/dev/.openclaw/npm/projects/askjo-camofox-browser-2f55486f67/node_modules/playwright-core \
CHROMIUM_PATH=/home/dev/.cache/ms-playwright/chromium-1228/chrome-linux64/chrome \
node tests/browser-smoke.cjs
```

Browser normalized output probes (linear): bass 0.08293, released 0,
groove 0.06643 (time-dependent), voice 0.02554, panic 0. No human audio
output/listening device was involved. This proves signal and controls, not taste.

Sanitizers initially found a signed-negative shift in pinned `fastmath.h` when
using A4-relative negative exponents. App-local conversion now uses MIDI-0-relative
**non-negative** exponents. Base wasn't patched or advanced. Initial sanitizer
linking also required installing local `libasan`/`libubsan`; no runtime dependency.
The upstream host compiler still emits two signedness warnings from `sense.c`.

## Local artifacts / measurements

All ignored unless noted. Paths below are relative to this repo unless absolute:

- `app/build/host/demo-30s.wav` — 5,292,076 bytes; demo instructions in README.
- `app/build/host/{note,master,voice-live}.wav`, host screenshots and demo metrics JSON.
- `app/assets/phrase.wav` — committed dry modulator, 126068 WAV bytes;
  125990 PCM bytes, 62995 frames at 22050 Hz / 2.857 seconds.
- `app/build/voice/vocoder-{110,164.8138,220}.wav`, `signal-check.json`.
- `app/build/browser/{desktop,mobile}.png`, `results.json`;
  desktop screenshot also committed as `docs/img/browser.png`.
- `/home/dev/.openclaw/workspace/tmp/bgirl-baseline/` — original template WAVs and
  branded-before-electro browser screenshot. Original host boot/key PNGs were not separately preserved before the
  branded regression overwrote them; baseline WAVs were preserved.
- `/home/dev/.openclaw/workspace/tmp/bgirl-voice/source.wav` — original TTS download.
- Exact command logs: `/tmp/bgirl-{brand-tests,final-tests,web,sanitized,demo,browser-final}.log`.
- Vocoder state 828 bytes; complete engine state **912 bytes** on host, excluding
  the read-only phrase and platform state. No dynamic DSP buffers.
- Standalone 8-second voice render + stdio: 12.05/13.35/18.82 ms at three pitches
  in the recorded run (only 2.857 seconds active). Host regression reports about
  0.2% render realtime; full DSP suite ~1.8 seconds host CPU. Not device timing.
- Host binary: 273952 bytes; WASM: 196948 bytes. `size` reported
  host text 250310, data 924, BSS 2066784 before the MIDI intake cap; simulator
  storage dominates BSS. **Do not treat this as linker-map/device RAM evidence.**

## Blockers, unverified assumptions and next work

1. **Listen first:** compare dry and three vocoded pitches blind; record surviving
   words and musical usefulness. Add consonant/noise path or compare 16 bands if
   needed. Basic saw oscillators are not band-limited. Voice remains experimental;
   no intelligibility, clipping-free taste or final sound-design claim.
2. **No target evidence:** package ID uniqueness, radio-off device configuration,
   inherited flash/settings offsets, linker RAM/flash/XIP fit and SDK/toolchain
   are unreviewed. 256/44100 ≈ 5.805 ms nominal deadline; worst-case device
   rendering with MIDI/UI, xruns and latency are unknown. Single-core request
   publication follows the current platform interrupt model, not tested hardware.
3. **Browser coverage:** Chromium headless only; real speakers/headphones,
   physical phones, Safari/Firefox and Web MIDI are untested. System screen-reader
   text works; separately licensed built-in fm1-reader voice bundle is absent.
   Words URL intentionally uses reserved `https://bgirl.invalid/words/`; never
   distribute device firmware pointing at it.
4. **Persistence:** octave/part/Bluetooth only, new `BGR1` record magic resets the
   inherited template record. Tempo/tone/mute/run are runtime-only. No new flash
   allocation or storage redesign.
5. Hosted CI is checked separately at PR handoff; local checks aren't hosted CI.
   No merge, flashing, release, site publishing or device package was performed.

Scope/acceptance source: INITIAL_BUILD_PLAN.md. Its listening/hardware uncertainties
remain open; this is useful implementation evidence, not an all-gates-green claim.

Recorded implementation source HEAD: `712d241cfc042612b0b973052f0f837b856c491c`.
Baseline plan/scaffold: `f67faa7f97b007cbe82c7fc4dd65aa9ea83489c4`.
Documentation-only commits may follow these measured source revisions.

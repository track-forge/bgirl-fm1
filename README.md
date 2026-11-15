# BGIRL FM-1

**Hype classic electro in a box.** A locally playable host/browser slice for the
M-VAVE FM-1: synthesized kick/snare/closed hat, mono bass, and an experimental
sample-fed playable vocoder. **Not device firmware ready for installation.**

![BGIRL browser demo](docs/img/browser.png)

## Run locally

Requires Git, Bash, Python 3.11+ with Pillow (`python3 -m pip install Pillow`),
a C99 compiler (`cc`), standard development headers and `libm`.

```sh
git clone --recurse-submodules https://github.com/track-forge/bgirl-fm1.git
cd bgirl-fm1
# For this unmerged review, check out feat/initial-electro-slice.
git submodule update --init --recursive
./tests/run_tests.sh
# Build only: ./build.sh --host
```

### Browser

Use **Emscripten 6.0.10**, matching the pinned platform's template toolchain:

```sh
git clone https://github.com/emscripten-core/emsdk.git "$HOME/.local/share/bgirl-emsdk"
"$HOME/.local/share/bgirl-emsdk/emsdk" install 6.0.10
"$HOME/.local/share/bgirl-emsdk/emsdk" activate 6.0.10
source "$HOME/.local/share/bgirl-emsdk/emsdk_env.sh"
./build.sh --web
python3 -m http.server 8765 --bind 127.0.0.1 --directory app/build/emu
```

If that emsdk clone already exists, skip cloning it. Open `http://127.0.0.1:8765`,
click **Switch on**, then ARP. Audio requires user interaction; Tab selects controls,
arrows turn knobs, and holding Space plays a focused key. On a phone, the panel
scrolls horizontally inside its box; landscape is easier. The web audio is the
same app DSP running in WebAssembly, not a replacement JavaScript synth.

## Controls

| Control | Action |
| --- | --- |
| ARP | Start/stop; restart from step 1; stop silences notes |
| K1 / KNOB 1 | Tempo, 60–180 BPM (default 120) |
| K2 / KNOB 2 | Bass tone, 0–100 (default 45) |
| FX | Drum mute/return; bass/voice and step clock continue |
| PRESET / PRESETS | BASS or experimental VOICE; releases held notes, groove continues |
| Keys / MIDI note input | Last-note-wins mono selected part; release does not resurrect older held notes |
| OCT− / OCT+ | Octave; releases old outgoing notes before transposing |
| MASTER | Smoothed master volume |
| HOME | Panic: silence selected note and stop groove |
| SAVE | Persist **only octave, part, Bluetooth state** |
| REC | Inherited Bluetooth controls: tap scan/connect/disconnect; hold on/off |

Other inherited buttons/knobs report their names but have no musical function.
MIDI CC120/123 and system reset panic the instrument. Pattern notes stay internal
(no sequencer MIDI output). Keys still send MIDI/pressure over inherited USB/BLE
routes. No external clock sync. Panic is consumed at the next audio render block.

In BASS mode, keys override the pattern bass; after release it resumes on the next
scheduled bass trigger. In VOICE mode, keys play the live carrier over the bass
groove. A new articulation restarts the phrase; legato changes pitch in place.
**No automatic voice reply:** the experiment still needs human listening approval.

## Thirty-second demo

1. Switch on, set MASTER conservatively, press ARP: machine beat and E bass groove.
2. At ~8 seconds press FX; hear bass alone. At ~12 seconds return the drums with FX.
3. At ~16 seconds play a short bass fill; K2 darkens/brightens the bass.
4. Switch PRESETS to VOICE; hold a key for the phrase, then release. Try another pitch.
5. Return to BASS. At ~30 seconds press HOME: silence and stopped transport.

Deterministic host rendition and isolated voice comparisons:

```sh
app/build/host/bgirl_host app/tests/demo.txt app/build/host
# app/build/host/demo-30s.wav (30.000 seconds, stereo PCM 44.1 kHz)
./tests/render_voice.sh # optional: requires ffmpeg
# Compare app/assets/phrase.wav with app/build/voice/vocoder-*.wav
```

## Verification and limits

`./tests/run_tests.sh` retains input, MIDI, Bluetooth, saved-state and screen-reader
checks and adds DSP timing/restart/mute/bounds/release tests and electro integration.
[Build status](docs/BUILD_STATUS.md) records exact evidence and local artifacts.
[Voice findings](docs/VOICE_SPIKE.md) separate signal checks from listening judgment.
[Asset provenance](app/assets/README.md) documents original speech and conversion.

Optional real-browser smoke (Playwright + Chromium installed in your environment,
local server running): `node tests/browser-smoke.cjs`. For a non-project install,
set `PLAYWRIGHT_MODULE` to its module path and optionally `CHROMIUM_PATH` to its binary.
The check measures AudioWorklet output and uses actual keyboard/pointer controls;
it is **not a human audition**. Screenshots/results go to `app/build/browser/`.

No device build, flashing, package identity allocation, storage-map review, target
CPU/RAM/xrun validation, releases or published site. The host executable's BSS
includes simulator storage; it is not a device RAM estimate. Basic oscillators are
not band-limited; tone and voice quality need listening/tuning. Tempo, tone, mute
and transport reset at boot. Built-in spoken screen-reader voice is unavailable
without a separate fm1-reader bundle; system screen-reader text is checked.
The required words URL is a reserved `.invalid` placeholder, not a live endpoint.

## Project notes

- [Initial build plan](docs/INITIAL_BUILD_PLAN.md)
- [Contributor instructions](AGENTS.md)
- [Pinned upstream provenance/licensing](docs/UPSTREAM.md)

The host/web-only wrapper and read-only host CI deliberately cannot publish or
flash. fm1-base and BSP pins are unchanged. Source files retain MPL-2.0 notices;
the original speech asset has its own documented dedication.

# Portable song snapshots · verification

2026-11-18 · [issue #37](https://github.com/track-forge/bgirl-fm1/issues/37) ·
[same draft PR #38](https://github.com/track-forge/bgirl-fm1/pull/38).
Browser/host evidence only; human listening and hardware validation not performed.

## Graphics rebase · 2026-11-19

Rebased the same PR/branch onto main `6768f96` (merged STUDIO graphics and
browser-shell overhaul). The original acceptance below is historical evidence;
the following results are fresh checks of the integrated runtime.

- Portable songs is an `intro-card` immediately below the panel/keyboard help,
  inheriting STUDIO colors, fonts, focus outlines and responsive button sizes.
- Retained upstream accessible panel startup focus/key release behavior and all
  upstream build/font copying and test gates.
- Updated CUT SONG's newly introduced visible renderer to use portable source
  metadata, not destination slot names/lengths/density. A native regression draws
  CUT SONG after populating different destination slots and checks the resolver.
- No schema/capability, engine/BSP pin, factory song, shared service or tailnet
  routing change. Existing v1 exports remain compatible.

### Rebased acceptance

- Frozen runtime: `19e722ba33cec2650c7799edfd26fba468bbed24`.
- PASS browser build (`sh web/emu/build.sh` after the root asset-generation build).
  WASM **1,081,017 bytes**, SHA256
  `7f9964d3a2109754c87cd0525acb3b513330217d6b13794ae00598ca4bf77314`.
- PASS complete root `./tests/run_tests.sh`, including all new main graphics
  matrices and snapshot regression; **111 audio goldens unchanged**.
- PASS **13/13** Chromium suites: splash-browser, browser-smoke, browser-shell,
  vocoder-browser, mega-browser, demo-browser, vocal-browser, dance-browser,
  crew-browser, six-browser, sh-style-browser, system-browser, song-browser.
  Same environment and loopback8813 recipe below; no hosted CI claim.
- PASS shared descriptor/schema checks, 29 JSON negatives, atomic native/WASM
  import, actual download/upload, fresh-profile import, edited-state refresh,
  preserved slots, dense FULL and two-source64-step audio audition.
- Browser shell keyboard/focus/release/contrast/reduced-motion checks pass.
  Mobile and CUT SONG screenshots visually inspected; zero page errors.
- Local logs `/tmp/bgirl-rebase-final-build.log`, `/tmp/bgirl-rebase-host.log`,
  `/tmp/bgirl-rebase-<suite>.log`. [Browser receipt](song-snapshots-rebase-results.json).
  [CUT SONG](img/song-snapshots-rebase-song.png) ·
  [Mobile shell](img/song-snapshots-rebase-mobile.png).
- Shared tailnet preview remains on its existing immutable artifact; this rebase
  does not deploy. The same PR remains open and retains its existing ready state.

## Original pre-rebase source and artifact

- Planning head: `0bfa0f30eeff740c6e7e44347ccd8b8af1cdac99`.
- Unchanged main/base: `c7116c5451bcfca9325ce71a71f7356ab1bd1b2c`.
- **Frozen runtime: `79ba1bf1a0ae276373f03c502ba1c4bd756b2aa2`.**
  Later test/example/verification changes do not change runtime files. The test
  harness avoids a duplicate codec run and auditions a full64-step section.
- Exact local WASM: **968,133 bytes**, SHA256
  `c3254840e028bfba296a8d65bb3665520052c3738692edb4ae9c516fdf1b534e`.
  `build-sha.txt` and exported producer identify the frozen runtime above.
  Compiler/platform-specific artifact, not a cross-toolchain reproducibility claim.
- Emscripten6.0.10 (`d6c521a7f05449857c76bd99e396895583cf2083`),
  GCC15.3.1, Node24.22.1, Playwright Core1.59.1, Chromium149.0.7827.55.
- Capability: `bgirl-six-clarity-127-v1-bb83fb2d0515bc0b`.
  Descriptor/factory identity, not an exact producer-SHA lock.

## Acceptance

| Check | Result |
| --- | --- |
| Root `./build.sh --web` at frozen runtime | PASS |
| Root `./tests/run_tests.sh` | PASS |
| Actual Chromium CI browser matrix, eleven suites | PASS (11/11) |
| Shared layout and generated schema check; public example validation | PASS |
| Live export → two-path agent edit → WASM import, no rebuild | PASS |
| Native SONG source names/lengths/density vs different named slots | PASS |
| `git diff --check` | PASS |

The root host runner retains storage, user-preset, input/debounce, MIDI parser,
codec, persistence, UI, MIDI/clock/drift, swing, audio, backup, vocoder, vocal,
dance/CREW, signature, six-track, editor, clarity, Mega and SH-STYLE gates. It also
runs the focused snapshot native/WASM/CLI checks. The existing20,000-record codec
matrix runs through upstream `project_test.c`, not a second time in the focused
snapshot script. No golden baseline was updated.

Observed unchanged audio coverage: **111 golden renders,0 changed/0 gone,
0 health failures,0 voice/routing failures,0 crashes**. The runner reports two
pre-existing extra renders outside its baseline; no new golden coverage is
claimed for them. Clarity and SH-STYLE each compare176,400 native/WASM frames;
Mega compares661,500 frames. Host instruction counters do not establish target
CPU/deadline/RAM fit.

### Focused proof

`tests/song_snapshot_test.c`, `tests/song-snapshot.mjs`, and
`tests/song-browser.cjs` cover:

- Live unsaved six-track sound/mix/step63/motion and custom FM6 state; no delayed
  flash export. Dense FULL data transfers intact beyond the3840-byte save limit.
- Two different source slots, several per-track/row references, destination slots
  containing different notes/lengths/names, unchanged named saves after import,
  and normal chain cache/playback. SONG overview uses portable source metadata;
  PROJECT still shows actual named saves.
- Real browser download/upload; normal panel edit before autosave; canonical
  agent note/parameter edits; fresh-profile roundtrip; normal edits after import
  checkpoint and survive refresh. Re-export compares normalized musical state.
- Shared JSON validator: **29 negative cases**, including type/range/integer,
  missing/duplicate IDs, counts, unsupported versions/capabilities, truncated and
  oversized files. Representative malformed raw-WASM fields are also refused
  without mutation, before narrowing/array traversal.
- Missing vocal identity refused; cancelled import preserves latched controls,
  current music and storage. Expired requests cannot apply late. Accepted import
  stops, clears pending MIDI/panel/encoder note-ons and refreshes the panel.
- New/reset detaches both source mapping and paired checkpoint. Destination saved
  slots/libraries remain unchanged through import, playback and refresh.
- Portrait390×844 and desktop1300×1100 controls; real download at mobile viewport;
  storage-denied mode warns SESSION ONLY and can still export. Zero page errors.
- Actual browser AudioWorklet/analyser signal during a complete64-step SONG
  section (including edited step63), bounded and
  nonzero (observed final peak0.09287261962890625). This is technical audio
  evidence, **not human listening approval**.

CLI proof changes only these normalized musical paths:

```text
song.tracks[4].steps[63].notes[0]: 0 -> 57
song.tracks[5].parameters.P_PAN: 0 -> 15
```

The browser test additionally makes an audible step, custom FM6/OWN and lock edit,
imports the file, plays it and compares untouched fields. No rebuild happens
between browser export, agent JSON edit and browser import/audition.

## Commands and local receipts

```sh
source /home/dev/.local/share/bgirl-emsdk/emsdk_env.sh
./build.sh --web
./tests/run_tests.sh
# Focused standalone entry (included by the full host runner):
./tests/run_song_snapshot_tests.sh
node tools/song-snapshots/song.mjs validate examples/song-v1.bgirl.json
node tools/song-snapshots/song.mjs diff \
  tmp/song-fixtures/export.bgirl.json tmp/song-fixtures/agent-edited.bgirl.json
```

Actual browser matrix against a loopback server (all existing CI suites plus the
new song file-flow suite):

```sh
export BGIRL_URL=http://127.0.0.1:8813
export PLAYWRIGHT_MODULE=/home/dev/.openclaw/npm/projects/askjo-camofox-browser-2f55486f67/node_modules/playwright-core
export CHROMIUM_PATH=/home/dev/.cache/ms-playwright/chromium-1228/chrome-linux64/chrome
for spec in splash-browser browser-smoke vocoder-browser mega-browser demo-browser \
  vocal-browser dance-browser crew-browser six-browser sh-style-browser song-browser
do
  node "tests/$spec.cjs"
done
```

Final logs: `/tmp/bgirl-snapshots-verified-build.log`,
`/tmp/bgirl-snapshots-verified-host.log`, and
`/tmp/bgirl-snapshots-verified-<suite>.log`. Browser artifacts/results:
`tmp/song-browser/` (ignored/local). The portable SONG screen was visually
inspected: SOURCE A, all six lanes and the correct12-bar source-derived timeline.
Desktop/mobile full-page screenshots were also inspected.
[Committed SONG screen](img/song-snapshots-portable-song.png) ·
[Final snapshot browser results](song-snapshots-browser-results.json).

Final review after initial acceptance found SONG's metadata display still read
destination slots. The scoped resolver fix and its
native names/lengths/cache-mode test were added, then the final runtime above was
rebuilt and the host/browser matrix repeated. Evidence is not silently carried
across that fix.

GitHub-hosted CI is tracked separately on PR38; local results are not a hosted
CI claim. Existing workflow gates stay enabled; `song-browser.cjs` was added.

## Local browser audition

The implementation build is in the existing worktree, not the shared preview:

```sh
cd /home/dev/.openclaw/workspace/codebase/bgirl-song-snapshots-plan
python3 -m http.server 8813 --bind 127.0.0.1 --directory felucca/build/emu
```

A loopback server on8813 was left running for this handoff. If using a remote
workstation, tunnel that port rather than changing Tailscale Serve/Funnel:

```sh
ssh -N -L 8813:127.0.0.1:8813 dev@YOUR_OPENCLAW_HOST
```

Open <http://127.0.0.1:8813>, Switch on, stop/release controls, Export song. Make
an edit using the [CLI example](../tools/song-snapshots/README.md), validate/diff,
Import song, confirm and press PLAY. For an arrangement, GLO → SONG → PLAY.
Stop/release and wait at least2seconds before refresh to capture later imported
session edits, or keep a newly exported file. Browser checkpointing is not
continuous crash recovery during playback.

## Remaining boundaries

No merge/deploy/release, hardware build/flash, engine/BSP pin change, factory demo
change, new offline audio/gallery generation, Ableton/MIDI device access or
existing tailnet-preview modification. Private user samples remain unresolved;
matching installed vocal identity/group is required. Imported source projects'
resources are checked conservatively even when their source sounds are not used
by SONG's current-instrument playback. Hardware persistence is not promised.
Portable arrangements use JSON, not named-slot SAVE. Musical/listening approval
remains Doug's separate decision.

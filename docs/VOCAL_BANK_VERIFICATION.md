> **2026-11-16 demo restoration:** the original issue-13 evidence below is
> historical. Default/public builds now include the authorized G6-only bank,
> not the three-entry original-speech fixture. The other five recordings remain
> private. Current reproduction and mapping: [VOCAL_BANKS.md](VOCAL_BANKS.md),
> [G6 provenance](../assets/vocals/g6/README.md).

## 2026-11-16 public G6 demo restoration

Root cause: PR #14 merged the right pattern, but it was guarded by the full
optional bank's group count. Clean/default/Pages builds repackaged the old speech
fixture, so the guard chose the four-bar phrase demo instead.

Default build now deterministically packages the authorized G6 original and six
unchanged supplied chops; no other supplied recordings are committed. The G6
label is independent of packed VIEW position. The twelve-trigger/two-bar groove
and octave-down CLOUD PAD carrier are shared by startup, explicit demo load and
wet 8/12-band auditions. Custom banks without G6 retain the rhythm on their first
group. Autosaves and unrelated saved projects are not reset.

- `./build.sh --web` and full `./tests/run_tests.sh`: PASS, including 98 unchanged
  golden renders, 88200 bit-identical native/WASM frames, 630 threaded vocoder
  comparisons, enabled/disabled bank checks, 9 packaging tests, arp dance and
  Battle Circuit compatibility checks.
- Real Chromium under `/bgirl-fm1/`: demo-specific startup/explicit G6 label +
  matching fingerprint, 9-second repeat playback, STOP silence, zero page errors;
  existing browser-smoke and vocal-browser save/load checks PASS.
- Additional native integration: full private six-group bank and original-speech
  fixture PASS with the same updated code; no generated header was reused in the
  default build.
- Public bank: 1 whole + 6 chops, 42294 encoded bytes, 84588 decoded samples;
  bank SHA256 `a0fce7527ad0f979f83be2f5cfc86fc29277db570bc0f870de20744dea9a3757`.
- Browser WASM: 902450 bytes; SHA256
  `599fb4770bb6397753f32999b4852609ea833e1f303d2b7391e6df403feaec9e`.

Reproduce from checked-in inputs with `./build.sh --web`, `./tests/run_tests.sh`,
then serve `felucca/build/emu` and run `node tests/demo-browser.cjs` (set
`BGIRL_URL`, `PLAYWRIGHT_MODULE`, `CHROMIUM_PATH` as needed). CI includes that
new browser check. This is browser/host evidence, not physical-device proof.
The original issue-13 results below describe the earlier fixture/private builds.


# Vocal-bank host/browser verification — issue #13

Runtime revision: `8ccf63bd1bf4f22fedb46bf030c8a9b909133d49`.
Harness completion-latency refinement: `7469596e34138231af50fb6508c68518b1745d38`.
Base freshly fetched and compared with `ls-remote`: main
`c3edd758d6bc12ffb74f0522b4904210816ec93f`. Contract committed first at `072aff6`.
Felucca/BSP/engine pins unchanged. This document's later evidence-only commit
contains no runtime changes. No device build, flash, merge, release or manual
deployment. Local loopback HTTP serves the browser preview only.

## Toolchain / commands

Host GCC15.3.1, Python3.14.5, Node24.22.1, Emscripten6.0.10; Chromium via Playwright.
Activate Emscripten as documented in README. On this host:

```sh
export PATH="$HOME/.local/share/bgirl-emsdk/upstream/emscripten:$PATH"
./build.sh --web
./tests/run_tests.sh
python3 -m http.server 8773 --bind 127.0.0.1 --directory felucca/build/emu

# Separate shell, with Playwright installed:
export BGIRL_URL=http://127.0.0.1:8773/
node tests/browser-smoke.cjs
node tests/vocoder-browser.cjs
node tests/vocal-browser.cjs
```

The existing CI job runs these same host/browser tests under the Pages project
path, always with the public fixture. Local custom `PLAYWRIGHT_MODULE` and
`CHROMIUM_PATH` can select an existing install; those are harness configuration,
not runtime dependencies. Default builds require none of the private inputs.

## Public fixture evidence

- Baseline build and complete host runner passed **before implementation**.
- Final default build and host runner: PASS. All98 existing golden renders
  unchanged;88200 native/WASM comparison frames remain identical. Existing
  storage, input, MIDI, UI, drift/swing, backup, autosave and legacy tests pass.
- Packaging:8 Python tests pass, including reproducibility, stable mapping
  despite manifest row ordering, inventory/duplicate/missing-path failures,
  malformed bounds/formats/durations, missing tails, unsafe paths and invalid
  float PCM. Default fixture comes only from retained original BGIRL speech.
- Host vocal test:3 fixture entries; exact start/end and all128 grid checkpoints,
  decoder comparisons against the full stream, tap-to-end durations, chromatic
  maps, queued engine-transition taps, newest/no held fallback, pitch/reverse/
  decay/loop overrides, captured source/view lifetime, STOP/panic/missing/invalid
  silence, identity-aware project/preset persistence, source automation identity
  retention. Run both vocoder enabled and `BGIRL_VOCODER=0` (source playback remains
  available dry; the disabled build does not synthesize a vocoder). All128 CC
  values preserve ordinary SLICE DIV scaling and correctly span vocal views.
- Sequencer: BPM60 /180 gives quarter-note trigger spacing44100 /14700 host
  frames (bounded block alignment), with unchanged one-shot decoded length/rate.
  Browser API buffers audio in128-frame halves; active-voice completion checks
  allow two half-buffer intervals and independently require the exact decoded
  end position, rather than treating buffered voice completion as stretched audio.
- New whole/chop two-core cases:112 bit-identical ordered source/carrier/core-mask
  comparisons plus8 each rejected/partial/completed-failed worker cases. Existing
  336 ordered-pair plus182 carrier-engine comparisons and36 failure cases pass.
  Total630 bit-identical comparisons; no double render. Scheduling unchanged.
- Actual Chromium: existing instrument and vocoder suites pass. New vocal suite
  passes all four audible modes, note-off tap, STOP-to-silence, normal EDIT knob,
  confirmation cancellation, full project SAVE > PROJECT and subsequent panel
  load after reload. Read-only firmware state4102 verifies SRC6, WHOLE, START0,
  matching identity (not just a nonempty browser storage object). Mobile has no
  horizontal overflow and all suites report zero page errors.

Public fixture bank SHA256:
`03838fd53df752004436ae92b594a30eff2dc97c3570385a8b7d2132ee2bd445`.

| Public resource | Exact bytes |
| --- | ---: |
| Original fixture +two chops, IMA stream |22423 |
| Generated C header (text, not runtime RAM) |77673 |
| JSON metadata |9852 |
| Default WASM |870913 |
| Baseline WASM |842141 |
| Default WASM increment |28772 |

Default WASM SHA256:
`142e0d1e35c826723e49fc005103715f910cbbd8f0239a11645d86cb63434144`.

Clean-checkout public CI at runtime/test head `7469596` passed both
[push run43774678244](https://github.com/track-forge/bgirl-fm1/actions/runs/43774678244)
and [PR run43774684004](https://github.com/track-forge/bgirl-fm1/actions/runs/43774684004),
including all three Chromium suites under `/bgirl-fm1/`. Pages jobs were skipped;
no private inputs or private WASM were available to those runs.

## Supplied private-input acceptance

The six originals and40 supplied chops were available. Explicit packaging
validated and hashed **all46 files**, excluding the separator-containing reel;
no detection/regeneration/retrimming was performed. Two independent output
packages compare byte-for-byte for header, encoded stream and JSON.

```sh
python3 tools/package_vocals.py --input-dir /tmp/vocals-for-doug \
  --manifest /tmp/vocals-for-doug/slices/manifest.json --output local/vocal-bank
python3 tools/package_vocals.py --input-dir /tmp/vocals-for-doug \
  --manifest /tmp/vocals-for-doug/slices/manifest.json --output local/reproduced-bank
cmp local/vocal-bank/bank.ima local/reproduced-bank/bank.ima
cmp local/vocal-bank/bank.json local/reproduced-bank/bank.json
cmp local/vocal-bank/bgirl_vocals.h local/reproduced-bank/bgirl_vocals.h
BGIRL_VOCAL_BANK="$PWD/local/vocal-bank" ./build.sh --web
./tests/run_tests.sh
mkdir -p local/acceptance
felucca/build/host/vocal_bank_test local/acceptance
node tests/vocal-browser.cjs
```

Bank identity:
`2cf02818a15bda381158d7270060b6d191204e28c2b5c14c815f887891c9856a`.
IMA stream SHA256:
`42271caceb6475392088251bf07180b74ab4f7971b9f9008ac222ceae1852e51`.
Input manifest SHA256:
`a05a7096d535cfc6e6ed6cf53f1628597629e8b476f753543186e23a8fef7a41`.
Individual source/chop hashes and all exact boundaries remain in ignored local
`bank.json`, not public listening artifacts.

| Local view | Entries | Runtime samples |
| --- | ---: | ---: |
| WHOLE |6 |290856 |
| G1 /T67_S006 |12 |78790 |
| G2 /T71_S010 |5 |51950 |
| G3 /T72_S009 |7 |42208 |
| G4 /T72_S010 |6 |36245 |
| G5 /T74_S001 |4 |39369 |
| G6 /T75_S008 |6 |42294 |
| Total |46 |581712 |

Whole durations:3.573243,2.356009,1.914195,1.643764,1.785442,1.918095 seconds
at22050Hz. Duplicated whole/chop views intentionally retain different fade
boundaries. Original total input coverage581710 frames at44100Hz is retained;
ceil rounding at each entry accounts for sample-count differences.

| Private package resource | Exact bytes |
| --- | ---: |
| IMA stream |290856 |
| Generated C header |1004841 |
| JSON metadata |32152 |
| Private local WASM (not distributed) |1137024 |
| Increment over baseline WASM |294883 |

Private acceptance PASS:

- Full host runner, including46-entry tests with vocoder enabled and disabled,
  legacy saves/default demo/ordinary SLICE,98 unchanged goldens,88200 identical
  native/WASM frames and630 split-render comparisons.
- All46 exact entry boundaries/checkpoints and all128 grid checkpoints checked
  against decoding from stream start. Every entry reached and played to its
  decoded end; all128 ordinary/vocal MIDI DIV/VIEW control values checked.
- Four8-second finite/nonzero bounded stereo16-bit44100Hz WAVs,1411244 bytes
  each, retained only in ignored `local/acceptance/vocal-0..3.wav` (whole dry,
  whole vocoded, sequenced chops dry, sequenced chops vocoded respectively).
- Actual Chromium on the private build:6 groups detected, all four modes pass,
  original short-tap playback continues, STOP silences, normal EDIT controls
  work, full project save/load restores SRC6/WHOLE/START0/matching fingerprint;
  mobile no overflow; zero page errors. No human listening approval inferred.

| Actual private browser mode | Observed peak |
| --- | ---: |
| Whole dry |0.068497 |
| Whole vocoded |0.139847 |
| Chops dry |0.068481 |
| Chops vocoded |0.139206 |

Private local WASM SHA256 (served bytes verified identical):
`37c6c3e906ab0bb53a04dd70b2911a5cd5008d11bfae3ad2da3ed755eadc101c`.
Loopback service `bgirl-vocal-audition.service` is active at port8773, explicitly
bound only to127.0.0.1; existing public Pages/private HTTPS previews are untouched.
The private worktree/package are retained for Doug's audition, not deployment.
Local logs are `/tmp/bgirl13-final-{public,private}-*.log`; private screenshots /
result JSON remain in `local/acceptance/private-browser/`.

See [local listening steps](VOCAL_BANKS.md#build-and-serve-locally).

## Working memory / bounds (host measurements only)

Incremental logical persistent working state: **72 bytes**:32 per-track expected
identity bytes +8 undo identity bytes +32 in the decoded project scratch.
Host symbol sizes confirm undo1528→1536 and project scratch4904→4936. Track stays
1856 bytes, vocoder DSP348, captures280; decoder/voice/reverse buffers are reused
without growth. UI descriptors are const, not allocated at runtime. Bank audio
and metadata are compiled read-only, not decoded into resident PCM.

Additional read-only source descriptor976 bytes; private entry table552 bytes,
group table14, identity8. START descriptor table224 bytes on64-bit host (public64);
WASM/target pointer-dependent metadata differs. Header text sizes are **not**
runtime resident sizes. Existing WASM32MiB linear-memory setting is unchanged.

Project wire extent stays3840 bytes, ID region bytes3277..3308 inclusive; MIDI
bytes3309..3311 and FM6 patches3312..3823 unchanged. One spare byte remains before
IDs. Preset extent192 bytes; packed identity offsets134..141 retain the existing
category byte143. `P_COUNT=126`, `P_E0=118` unchanged. FUNC/version6 prevent older
firmware from silently substituting factory speech. All previous readers remain.

No allocation, file/network I/O, logging or blocking added to render. Preparation
and validation occur offline; source selection/voice decode remain bounded.

## Limits / human acceptance

- Host/browser proof only; no target linker map, resource flash fit, CPU deadline,
  cache coherence, xruns, physical keys/MIDI or physical-device performance proof.
- Signal bounds, nonzero energy and screenshots are **not human intelligibility
  approval**. Doug must listen to his recordings in dry and vocoded modes.
- One optional immutable bank per build; another machine needs the same inputs /
  prepared package separately. Missing/mismatched bank is silent and preserved
  on resave. No hot uploader, slicing editor, bank resource manager or streaming.
- No warping, tempo-fit/repitch, synced loop or new dry-overlay feature. BPM only
  changes trigger timing. Newest shot may intentionally cut a previous long shot.
- Automatic main CI is the only public Pages delivery path; this PR does not
  publish private assets, merge itself or manually deploy anything.

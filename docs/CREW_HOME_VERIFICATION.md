# CREW HOME — browser / host review

2026-11-18. Runtime source **30198d9cf4df88439e8df7abad43c51f9ba2b827**;
subsequent documentation-only commit records evidence. Same draft
[PR29](https://github.com/track-forge/bgirl-fm1/pull/29), branch `plan/crew-home`.
Fresh remote main verified at `c51c53885fd18c1e84be676d1644143803bc2e0b` before
implementation. Original plan head5072b2c; no newer main changes were discarded.
Doug requested implementing this existing PR, superseding the handoff's suggested
new-PR workflow. PR28's Mega Jam arrangement was **not** absorbed.

## Delivered

- HOME IDs0/1/2 stay SCOPE/TRACKS/LEVELS; CREW appends3. Fresh settings default to
  CREW; imported choices, unknown-value SCOPE fallback and other preferences survive.
  No save/project format or frozen defaults change.
- Exactly one dancer at the selected track, with T-number, move and activity text.
  Six stable associations share five banks. ALGORITHM swaps source/identity on
  the next update, clears the previous silhouette and reads current activity.
  No miniature crew or background per-track sprite evaluation.
- Normal engine HOME knobs, HOME→MIXER→CLOCK, HOME-hold menu, GLO banks and inactive
  cells remain musical controls. Menus/dialogs/name/performance layers own the
  screen. LARGE uses full graph / bigger labels, not the62px strip.
- Core0 accepted starts/winning mono legato coalesce peak accents; one whole-word
  per-track snapshot is published **after render and CPU2 join/recovery**. Workers
  never write it; UI never reads mutable voices. Activity handles sustained gates,
  bounded250ms one-shots, one-quarter settle, freeze, mute/solo inhibition,
  deferred starts, stealing and reset. [Exact contract](CREW_ACTIVITY_CONTRACT.md).
- One Q32 phrase/four quarter notes. Internal samples/current BPM, external musical
  GRID units, START reset / STOP local live phase / CONTINUE preserve. BPM changes
  change slope, not phase; no event replay or musical RNG usage. At most one
  selected mask evaluation per graph paint, normal animations capped by67ms.
- All four new video sources retrieved with the supported Moltazine CLI/media
  resolver, decoded/inspected and converted. Named, deterministic96px packs,
  source hashes/metadata, permission record, contact sheets and standalone timing/
  seam/idle/mute previews are [committed offline](../assets/crew/README.md).
  Existing FLOORWORK/ARP DANCE remains unchanged, including CRISP/GHOST semantics.

## Reproduction

Tools observed: GCC15.3.1, Emscripten6.0.10, Node24.22.1, Python3.14.5,
FFmpeg7.1.5, Pillow12.1.1. Engine/BSP pins unchanged:
`d0260074ce100a47675814b7f38d61f11176c926` /
`5942e68b89ab9ccd23f2c541d712ca38fe635f11`.

```sh
git submodule update --init --recursive
source /home/dev/.local/share/bgirl-emsdk/emsdk_env.sh
./build.sh --web
./tests/run_tests.sh
# Included in root suite:
./tests/run_crew_tests.sh
# Offline art repeat, originals must match committed hashes:
python3 tools/convert_crew.py local/crew/originals /tmp/crew-repeat
```

A first baseline build lacked initialized gitlinks; initializing the **pinned**
submodules resolved it, not a dependency update. Authoritative baseline was a
frozen detached5072b2c worktree (`local/crew/baseline`): root web build and full host
suite PASS. This avoids counting a test reading concurrently edited sources as a
clean baseline. Receipts `/tmp/bgirl-crew-frozen-baseline-{build,tests}.log`.
Final source-frozen receipts `/tmp/bgirl-crew-final-{build,tests}.log`.

## Automated acceptance

Final root web build and **full host suite PASS (exit0)**.109 audio goldens unchanged,
zero gone/health/routing failures;88200 heavy-song and176400 clarity native/WASM
frames bit-identical. Applicable storage/persistence/UI/input/MIDI clock/drift,
six-track codec/motion/editor, signature/vocal/vocoder, disabled-vocoder and worker
failure/recovery matrices pass. Root runner also executes the focused CREW checks
below. Final source and WASM were frozen for this pass; no golden was regenerated.

Focused CREW, independently repeated:

- Fresh default, all four imported choices and unknown fallback, without resetting
  settings/projects. Normal editor menu protocol appends CREW and reports default3;
  the old test's three-name/default0 expectation was updated, not a wire ID change.
- Accepted/released/settling/frozen sustained activity; stopped live play; one-shot
  expiry despite held key; muted receipts, performance exclusion and no unmute-only
  rejoin; losing mono priority, winning legato, deferred request, stolen voice,
  panic and project reset. Other tracks cannot animate an inactive selection.
- Exact external GRID phase tests: quarter-quarter advances, BPM-independent units,
  START resets, stopped local phase advances, CONTINUE keeps phase. A development
  samples-versus-GRID error was caught/fixed before the final source revision.
- **1584 exact pixel compositions**:11 palettes×2 styles×6 tracks×12 poses, including
  replacement/old-pixel erasure. LARGE retains96px graph stage. Unchanged idle and
  hidden CREW produce zero additional CREW graph blits;1000 rapid publication/draw
  attempts over1s produce at most15 paints, not one paint per attack.
- **1,000,000 actual producer publication/reset iterations**, concurrent atomic
  snapshot reader; complete packed-word invariants hold. Host pthread stress,
  not a physical target memory-model/code-generation audit.
- **512000 stereo frames +4000 RNG/note snapshots byte identical** between compiled
  telemetry-off control and hidden, visible/rapidly selected, covered CREW under
  six-track playback plus RANDOM/probability/swing arp. No audio golden changes.
- Four mask/header/manifest integrity and crop-padding checks, source-order indices,
  bit order, bounded replacement inputs. Each of all four independent conversions
  matches **every generated file byte-for-byte**. No original FLOORWORK file changed.
- Focused ASan and `FELUCCA_CPU2=0` builds PASS. Track size1856B normal /1840B CPU2off,
  unchanged; wire3840B. ASan includes whole-word stress and exact pixel checks.

### Actual Chromium

Local HTTP8796 only, real AudioWorklet after Switch-on gesture:

```sh
python3 -m http.server 8796 --bind 127.0.0.1 --directory felucca/build/emu
export BGIRL_URL=http://127.0.0.1:8796
export PLAYWRIGHT_MODULE=/path/to/playwright-core
export CHROMIUM_PATH=/path/to/chrome
node tests/crew-browser.cjs
```

PASS: fresh CREW; six distinct track screens/stable move associations;24 further
selection switches; empty T6 stays inactive while the rest play; engine knob,
HOME/MIXER/CLOCK and mute/return; live notes while stopped; ARP/covered GLO paint
isolation; LARGE/LINE/alternate palette; all four HOME choices changed through the
normal menu, saved to emulated NOR/localStorage and restored after reload; desktop
and390×844 mobile, no overflow/page errors. Final measured audio peak0.2012557983,
nonzero/bounded. [Machine receipt](img/crew/results.json).

The preference test initially navigated PRESETS as if rows clamped, but the real
menu wraps. It now reads its current row before navigating HOME; no production
control change. The fresh default does not overwrite an existing origin's view.

Existing Chromium suites PASS: splash, browser-smoke, vocoder, demo, vocal, dance,
six, battle, signature and clarity. The clarity test uses a copied, byte-unchanged
historical local listening gallery; no gallery rerender or relabel. Four standalone
`file://` art previews also exercised playback, seam and idle/mute/accent controls
without page errors. CI runs CREW alongside its existing project-prefix tests.

## Visual review artifacts / limits

[Desktop](img/crew/desktop.png) · [Mobile](img/crew/mobile.png) ·
[LARGE / LINE](img/crew/large-line.png) · [Muted](img/crew/muted.png)

Sequential track screens (not a crew layout):
[T1 windmill](img/crew/track-1.png) · [T2 step/turn](img/crew/track-2.png) ·
[T3 wave-ish](img/crew/track-3.png) · [T4 worm](img/crew/track-4.png) ·
[T5 FLOORWORK](img/crew/track-5.png) · [T6 shared windmill](img/crew/track-6.png).

Actual240×240 screens inspected: controls/header/footer remain legible; stable
union framing avoids pose jitter and new crop clipping; upright arm/leg gaps and
floorwork shapes read, but thin fingers/hair simplify. Windmill poses occupy less
than the entire96px square because its all-frame bounds include extreme limbs.
Some original video frames already touch the source's top boundary; added output
padding cannot restore missing source pixels. Worm remains low to its floor, not
independently centered per pose. All seams are exposed hard cuts. No seamless-loop,
perfect motion reproduction or **Doug visual approval** is claimed.

## Measured resources (host/browser, not device fit)

Same GCC/O1 `tests/bgirl_test.c` binary, baseline5072b2c vs source30198d9:

| Region | Baseline | CREW | Delta |
| --- | ---: | ---: | ---: |
| New linked mask banks |0|55296B|+55296B read-only|
| Native text+rodata |826693B|884493B|+57800B|
| Native initialized data |1596B|1596B|0 aggregate, alignment may absorb fields|
| Native BSS |1654688B|1654848B|+160B|
| Logical new mutable fields |0|145B|24 snapshots+96 producer+4 phase+20 UI+1 default flag|
| Track / project wire |1856 /3840B|same|0|
| Existing canvas |59520B|same|0 new framebuffer|
| Changed graph transfer |58560B|same|full240×122 RGB565|
| WASM |916364B|948990B|+32626B|
| WASM initial linear memory |33554432B|same|unchanged fixed build allocation|

Existing13824B FLOORWORK bank is shared, not duplicated. Six track associations do
not multiply clip storage. At the15/s animation cap, graph payload alone can reach
878400B/s; forced user edits/selection and other independently changed UI regions
are additional. This is not a measured SPI bandwidth or device FPS promise.

Three alternating paired hidden-UI whole-process512000-frame runs: disabled
control median0.138777s, telemetry-on0.156953s (~13.1% higher). These include boot,
UI and `/dev/null` state writes, on a shared host; not isolated DSP/target deadline
measurements or a stable overhead guarantee. Raw local recipe/results:
`local/crew/host-timing.json`. Browser final CREW diagnostic self-load sample5.217%
under headless concurrent testing; not worst-case worklet latency or xrun proof.

Baseline WASM SHA256:
`6e6d3e8a9678dc2829c6d179813d3e2357009e771d37c1fd6232a456e08e5b9d`.
Final WASM SHA256:
`3d39d6bf7cec9c411e4919a5140fecc4540bd595a312e2ad1cd01702492d1e8f`.

## Demo and remaining gates

Build/serve locally as above. Fresh origin→Switch on→CREW→PLAY; ALGORITHM through
T1–T6, each replaces the last. Existing origins: HOME hold→DISPLAY→HOME→CREW.
Explicit six-track electro audition is also available; it is not a startup-demo
replacement. Select bass→HOME/MIXER mute→CLOCK→CREW, return/unmute; STOP and play
live keys; visit ARP DANCE and return; change HOME choice and reload.

**Unverified:** target atomic disassembly/linker fit/RAM, physical LCD/DMA tearing,
SPI payload timing, audio deadlines/xruns, physical USB/TRS and human art/seam
approval. No hardware build/flash, engine/BSP pin change, B6S1 expansion, paid
image/video generation, default-demo edit, merge, release/tag, manual public or
private preview deployment. Existing tailnet8448 preview/service remains untouched.

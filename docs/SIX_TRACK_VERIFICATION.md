# Six full tracks — host/browser verification

2026-11-17 · branch `plan/six-tracks` · draft [PR26](https://github.com/track-forge/bgirl-fm1/pull/26).
Baseline main: `4a5f1fbc414a32cd907b2b215060dca503004e6c`.
Core implementation: [`d6991eae6b945c7d5c57d02a7ef65400435a4191`](https://github.com/track-forge/bgirl-fm1/commit/d6991eae6b945c7d5c57d02a7ef65400435a4191).
Final PR head, browser acceptance follow-ups and CI receipts are recorded in the PR handoff.
No merge, deployment, release, device build, flashing or existing audition-service change.

## Delivered

Six unrestricted tracks ×64 steps; shared eight-voice admission/stealing budget.
T5/T6 have empty FM6/DRUM defaults, not light/MIDI-only engines. Original four
startup roles, Signature sounds/choke, vocal banks/identity, vocoder single-owner/
no-self-routing, arp dance, song chain, MIDI, recording, locks and undo remain.
The overview fits six rows; selected/mute/armed-record/playhead states are visible.

- ALGORITHM wraps through all six tracks. Selection follows level banks T1–4,
  then T5/T6/--/--. GLO and HOME LEVELS use those knobs; inactive knobs/keys
  cannot touch hidden tracks. GLO ALL/LEARN/FILL/TAP retain their original keys.
  Ordinary MIXER retains selected-track LEVEL/PAN/REV/MUTE editing, displaying
  the selected four-column bank. Sound pages retain four parameter knobs.
- MENU → CHANS has T1–T6 CHANNEL. MIDI IN → TRACK gives default channels1–6;
  the old explicitly four-channel blocks keep their behavior.
- Motion addresses use a checked nine-bit place in uint16, written LE16; T5/T6
  step63 cannot alias T1/T2. Shared event capacity stays128. MIDI learn retains
  16 bindings, now with three track bits; PER5 clears old incompatible bindings
  while preserving preferences/favorites. No project/autosave migration.
- Editor namespace `7D 46 36` rejects old `7D 46 4C` clients before dispatch.
  INFO/TRACK advertise six tracks. SONG rows use six slots + repeat; invalid
  track/count packets and old fixed-four rows are refused. MENU IDs33/34 append
  T5/T6 channels. Backup/import validates B6S1; full exports return failure.
- B6S1 retains the existing 3840-byte A/B object and four slots. PROJECT's SAVE
  card reports occupancy/FULL, and preflights before name entry. Manual save,
  rename, export and autosave never truncate. Unchanged full autosaves are
  blocked until editing changes the state. Native undo does not need packing.

[Exact wire format, capacity fixtures and clean-break/reset contract](SIX_TRACK_CODEC_SPIKE.md).
[Upstream pins, originating six-track commit, notices and adaptation map](UPSTREAM.md).

## Reproducible commands

Toolchain: Emscripten6.0.10, GCC15.3.1, Node24.22.1, Python3.14.5.
Pinned recursive submodules initialized without changing either gitlink.

```sh
source /home/dev/.local/share/bgirl-emsdk/emsdk_env.sh
./build.sh --web
./tests/run_tests.sh
python3 -m http.server 8792 --bind 127.0.0.1 --directory felucca/build/emu
```

For actual browser checks, set `PLAYWRIGHT_MODULE` to an installed Playwright
module directory and `CHROMIUM_PATH` to the Chromium executable if needed:

```sh
export BGIRL_URL=http://127.0.0.1:8792
for suite in splash-browser browser-smoke vocoder-browser demo-browser vocal-browser dance-browser six-browser signature-browser; do
  node tests/$suite.cjs
done
```

No public preview is published. Existing audition services/ports are untouched;
8792 was an isolated loopback-only acceptance server, not a deployment.

## Observed results

- Baseline root web build, complete host suite and actual Chromium smoke: PASS.
- Root host suite: PASS. **108 audio goldens unchanged, zero gone**, no health/
  routing/voice failures; **88200 native/WASM frames bit-identical**. Historical
  four-lane song probe retains `0x676a420d` over11026 blocks with T5/T6 idle.
  No golden files were regenerated.
- Existing storage/NOR interruption/retry/wear, user-preset, input, UART, UI,
  MIDI/clock/swing, backup, Signature, vocoder, vocals, dance, logo and visual
  settings checks retained. Affected old-project migration tests intentionally
  became rejection tests; old preset/bank/preferences support is not removed.
- Six-track runtime: all six solo signals nonzero through the real renderer;
  independent64-step patterns; every engine on T5/T6; motion/lock at step63;
  normal sound undo; inactive bank cells; new settings round trip/old learned
  map reset; channels1–6 ownership and note-offs; fixed-eight overload; live/RAM/
  NOR preservation on FULL; autosave no-retry behavior. Dense playback is bounded.
- USB/TRS external-clock fixture now explicitly covers all six tracks, including
  polymetric lengths31/48 on T5/T6: simulated20-minute phase/tempo/wrap tests PASS.
  This is not physical wire-clock validation.
- Worker tests:336 original vocoder split comparisons;180 extra T5/T6 ordered-
  pair/core-mask comparisons, plus rejected/partial/completed-failed worker jobs.
  144 whole/chopped identity-bound vocal comparisons also exercise T5/T6, with nonzero captured
  input; all eligible carrier engines retain182 extra comparisons. Signature hats
  retain28 bit-identical serial/threaded comparisons. No duplicate part render.
- Actual Chromium: all eight suites PASS, zero page errors. Six-track acceptance
  exercises six distinct screens/wrap, live audio after gesture, banked editing,
  inactive cells, accepted-template latched-control reset/cancel preservation,
  autosave fresh-instance restore, manual slot saving, unchanged
  stored flash after dense refusal, and mobile no-horizontal-overflow. Screenshots
  were visually inspected. Signature's ten sounds and BG-first browsing, default
  G6 vocal groove, vocoder and dance browser checks remain green.
- B6S1 exact codec round trips include dormant notes on REST steps, all flag bits,
  FM6 OWN bytes even on non-FM6 engines, vocal fingerprints, channels,16 chain rows
  and128 events. Exact3839/3840/3841-byte boundaries pass. Every shorter input
  length, wrong magic/version/count, bad checksum and malformed headers refuse
  without mutating output. **20000 checksum-valid mutations**:8663 accepted valid
  editable records,11337 atomically refused; no out-of-bounds access under ASan.
- `FELUCCA_CPU2=0` six-track runtime build/run: PASS. Root vocoder-disabled and
  dance-disabled preservation checks also pass. No thread-sanitizer or device
  deadline claim is inferred from the host thread harness.

Logs: `/tmp/bgirl-six-baseline-{build,tests,browser}.log`,
`/tmp/bgirl-six-tests-final.log`, `/tmp/bgirl-six-codec-asan.log`,
`/tmp/bgirl-six-single.log`, `/tmp/bgirl-six-browser-<suite>.log`.
Screens/results: `/tmp/bgirl-six-browser-<suite>/`.

ASan command uses the normal host include paths, plus
`-fsanitize=address -fno-omit-frame-pointer`, building `tests/six_codec_test.c`;
`ASAN_OPTIONS=detect_leaks=0:abort_on_error=1`. It checks memory bounds, not musical
quality or absence of all undefined behavior in inherited DSP.

## Capacity

| Fixture | Needed bytes /3840 | Result |
| --- | ---: | --- |
| Empty reset palette | 734 | saved |
| Existing startup electro/vocal demo | 1107 | saved |
| Six FM6 | 1236 | saved |
| Six FM6 +128 motion | 1748 | saved |
| New six-track sparse audition | 1358 | saved |
| Dense6 ×64 ×4 notes + flags/hits/chance/128 motion | 5972 | PROJECT FULL |
| Dense + every parameter nondefault | 6686 | PROJECT FULL |

The compact route is practical for these ordinary fixtures, not universal.
Filling every step of every track with every field can exceed a sector.

## RAM / build / timing findings — host, not target

Same native-check harness, GCC -O2, before/after `nm -S` and `size`:

| State | Four tracks (B) | Six tracks (B) | Delta |
| --- | ---: | ---: | ---: |
| `track_t` array (each1856) | 7424 | 11136 | +3712 |
| Engine union pools | 51552 | 77328 | +25776 |
| Chain state + four cached sources | 13096 | 19364 | +6268 |
| Slice buffers | 32768 | 49152 | +16384 |
| Decoded project scratch | 4936 | 7284 | +2348 |
| Serialized scratch | 3840 | 3840 | 0 |
| Four retained project slots | 15360 | 15360 | 0 |
| Motion store | 388 | 516 | +128 |
| Normal undo snapshot | 1536 | 1664 | +128 |
| Per-track insert histories | 2320 | 3480 | +1160 |
| Core mix accumulators (`mx`, two sets) | 1536 | 1536 | 0 |
| Vocoder captures/metadata | 280 | 280 | 0 |
| Vocoder DSP | 348 | 348 | 0 |
| Favorites/settings record | 520/572 | 524/576 | +4/+4 |
| Whole native harness BSS | 1594016 | 1654400 | **+60384** |

Whole harness mutable data additionally grows24 bytes. BSS includes emulator
NOR/framebuffer/ring state, not just firmware target RAM. Other growth includes
DRUM lane state, FM6 effective/OWN patches, motion bases, per-part RNG/modulation
and spread histories. `C1_NP` grows4→6 for per-part state; the core count stays2.
The performance FX history duration remains8192 frames despite larger slice
storage, preserving the existing FX behavior/goldens. No render-path allocation,
file I/O, logging or blocking was introduced.

Host stack-usage report: decoder752 bytes (chain116 + motion516 local objects),
packer128, capture48. Main decoded project scratch is static; two-pass validation
avoids a full decoded project on the decoder's stack. Target ABI/placement differ.

Final tested WASM919864 B versus baseline923565 B; browser memory setting
remains32 MiB. WASM SHA256 `6769a387de3d463f5a49bb2c00e6f630257f3f043e35641cf729ea7395b3f080`.
One native12-second WAV render took99.791 ms wall time, including output writing,
peak0.23458862, RMS0.020363941. Shared-host figures are not target CPU budgets,
ISR deadlines, xruns, physical RAM fit or flash compatibility proof.

## Audition recipe

1. Build/serve using the commands above; open localhost8792 and click **Switch on**.
2. **Six-track electro audition** → confirm → **PLAY**. At120 BPM, T1 kick,
   T2 snare, T3 closed hat, T4 driven sub, T5 iron stab, T6 chirp. Four bars,
   ordinary editable tracks. ALGORITHM reaches each; GLO levels follow banks.
3. STOP; use normal SAVE → PROJECT to save. Its SAVE card shows1358/3840 before
   edits. Reload only after ordinary autosave quiet/write-spacing rules, or
   explicitly load the saved project slot.
4. **Dense save-capacity stress fixture** → confirm. PLAY is an overload stress
   check, not the musical demo. STOP and attempt PROJECT SAVE: FULL, no truncated
   save and no destruction of the prior slot/session. Load the sparse audition
   or Blank electro template to resume.

Offline recording (ignored, local only): `local/six-tracks/six-electro.wav`,
12-second44.1kHz stereo PCM16; SHA256
`e33448d1986e3808443f62abedd1a8a4da810ee1a402b1259bee544f73819010`.
Reproduce with `tools/render_six.c`, same host include paths/flags, then
`render_six OUTPUT.wav`; optional final argument1–6 solos a track. Output is
not normalized. Human musical approval of the new arrangement remains pending.

## Explicit limits

No device compiler/build/flash, merge, public preview, release or deployment.
Physical RAM placement, real-time deadlines/xruns, physical MIDI/clock and hardware
flash compatibility remain a separately authorized milestone. No bigger storage
layout is proposed or silently assumed here. This is verified host/browser work.

### Browser closure follow-up

Final review reproduced a Shift-latched GLO remaining lit after a new six-track
fixture replaced the project. The new buttons now clear browser button/key maps
and latch state only after confirmation, matching the existing template actions.
A real Chromium regression proves cancel preserves the latch and acceptance
releases it. No DSP/codec change; WASM hash above remains unchanged.

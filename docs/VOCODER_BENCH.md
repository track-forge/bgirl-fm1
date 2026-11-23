# Vocoder timing build (#74)

This is an opt-in diagnostic and an experimental fixed-point kernel, not a
change to the normal render path or saved project format. A single build can
compare the floating-point reference, capture with the bank bypassed, and
the integer candidate. All three band counts keep their original coefficients
and routing, including the 24-band reference.

Build on a JieLi-equipped host:

```sh
BGIRL_HARDWARE_ASSETS=1 python3 tools/generate.py
BGIRL_HARDWARE_ASSETS=1 BGIRL_VOCODER_BENCH=1 ./felucca/build.sh --release 0.3-vocbench
```

Use the [MIDI bench loader and fixtures](https://github.com/track-forge/bgirl-fm1/pull/75).
After an attended install, verify boot and the bank CRC before importing the
intended Ticker fixture. Do not reflash between fixtures. This build retains
normal persistent boot: it does not resolve the separate #73 boot loop.
Preserve unsaved projects and disable Restore Last during controlled runs.

With the USB console open, use `vocbench bypass`, `vocbench float` or
`vocbench fixed`. These queue a mode and an interval reset at the next audio
half. Use `vocbench reset` to clear counters without changing the DSP state,
and plain `vocbench` to poll without clearing. Poll until `epoch` equals
`request`. The reported modes are 0 = float, 1 = bypass, 2 = fixed.

- `render_ticks` is the instrumented half-buffer render peak; divide by
  24 for microseconds or 24,000 for milliseconds. The deadline is 2.90 ms.
- `setup_ticks` is route lookup, buffer initialization and any route reset.
  `join_ticks` measures carrier absence/mute handling after mixing and before
  the bank. Neither includes tap copies embedded in track rendering; the
  whole-render OFF vs bypass comparison includes them.
- `wet_ticks` and `silent_ticks` time the bank itself on overlapping or
  non-overlapping blocks. Capture activity counts nonzero samples on each
  tap in the same block; this is not an audibility test. The overlap scan
  contributes to whole-render timing, not to the bank interval.
- `timer_ticks` is the peak of a back-to-back timer read pair. Do not
  subtract unrelated peaks. Also record `status` uptime, late blocks,
  voices shed and transport, and report late-count deltas within a boot.

Start with OFF and bypass, then use a short reference sweep only if the unit
remains responsive. The old float path can overload even while stopped.
Select bypass before loading a new fixture; arm the capture before PLAY.
Keep the build ID, mode, fixture hash, uptime, bank state and listening result
together. Confirm wet-only overlap and carrier removal, then repeat a
complete normal-mix loop and STOP → PLAY for the finalist. No hardware timing
gain or sound-quality claim follows from the host tests alone.

Host DSP check (after normal asset generation):

```sh
cc -O2 -w -ffp-contract=off -Ifelucca/firmware/hal \
  -Ifelucca/firmware/bsp/chip -Ifelucca/firmware/bsp/boards/fm1 \
  -Ifelucca/firmware/src -Ifelucca/build/gen \
  tests/vocoder_fixed_test.c -lm -o /tmp/vocoder-fixed-test
/tmp/vocoder-fixed-test
```

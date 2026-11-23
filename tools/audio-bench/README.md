# FM-1 audio regression bench

Use Linux USB MIDI to swap B6S1 projects into the running session, and the
USB serial console to sample audio counters. No reflash between variants,
no transport automation, and no firmware changes for the tools themselves.

## Connected setup

Enable USB SERIAL in firmware. Identify the correct ALSA raw MIDI and CDC-ACM
nodes with `cat /proc/asound/cards` and `ls -l /dev/serial/by-id/`; they can
change after reboot. Use an account with access to both nodes and close other
MIDI/serial clients. Use Python 3; no extra Python packages are needed.
In the examples below, substitute your device paths.

```sh
cd tools/audio-bench
midi=/dev/snd/midiC2D0
console=/dev/ttyACM0
python3 load-ticker.py probe "$midi"
(cd fixtures && sha256sum -c SHA256SUMS)
```

The loader uses editor PUT 67, object 0, and checks every ACK. It doesn't
directly write flash or overwrite named slots A–D. It **replaces the current
live project**, which Restore Last can autosave on regular firmware. Preserve
unsaved work first and disable Restore Last for a controlled comparison.
Regular loads require a CRC-ready Miami bank; the separate `--ram-probe`
load flag only accepts the exact diagnostic firmware, where Miami is not
mounted. Use the same bank state and firmware for all A/Bs.

## One matched run

Start stopped from a clean boot for each fixture. Live-reload the original
demo for the control so the control also goes through the importer. Example
on the RAM diagnostic, using Ticker at 8 bands:

```sh
python3 capture.py "$console" --label pre-import > run.csv
python3 load-ticker.py load "$midi" --ram-probe --song fixtures/ticker-sweep-8.b6s1
python3 capture.py "$console" --label post-import --no-header >> run.csv
sleep 5
python3 capture.py "$console" --label stopped-5s --no-header >> run.csv
python3 capture.py "$console" --label transport --samples 35 --no-header >> run.csv
```

Once transport capture starts, press PLAY for about 15 seconds, hit STOP,
then try PLAY and STOP again if responsive. Use the CSV's `playing` transitions
to identify the sampled boundaries; note actual audio/UI behavior and any
required power cycle separately. If the capture times out, preserve the
partial CSV. Don't assume an unsampled interval passed.

Record one row per run: fixture, pre/post-import and stopped/playing peaks,
new late blocks per interval, output quality, and STOP → PLAY behavior.
`audio_max_us` is a **boot high-water mark**, not an interval maximum.
Subtract `audio_late` readings only within the same boot; check
`uptime_ms` for resets. The deadline on the tested firmware is **2.90 ms**.
A zero late-count delta doesn't rule out audible breakup or an over-budget
render. This timer starts in the handler and doesn't measure IRQ-off restore
time.

## Fixtures and offline variants

The fixtures contain project settings and sequencer data, not audio.
`original-demo.b6s1` is the RAM probe's factory control. `ticker.b6s1` is the
exact packed Ticker from candidate `343b867`, with both vocal bank IDs mapped
for hardware. `ticker-sweep-off.b6s1` disables T3's vocoder carrier;
`ticker-sweep-8.b6s1` selects T5 ANALOG and 8 bands. Both keep Ticker globals.
On the bench, T5 at 24 bands reproduced the stopped overload (32.15 ms peak);
8 bands reached 13.31 ms during choppy playback despite zero late counts.
See [#73](https://github.com/track-forge/bgirl-fm1/issues/73) for the run history.

From the repository root, with pinned submodules and a C compiler:

```sh
BGIRL_HARDWARE_ASSETS=1 python3 tools/generate.py
tools/audio-bench/build-variant.sh
bench=tools/audio-bench
variant=felucca/build/bench-variant
"$variant" vocoder "$bench/fixtures/ticker.b6s1" 3 5 1 > /tmp/ticker-t5-12.b6s1
"$variant" split "$bench/fixtures/ticker.b6s1" "$bench/fixtures/original-demo.b6s1" 56 > /tmp/ticker-tracks-1-3.b6s1
```

`vocoder INPUT OWNER CARRIER [BAND_INDEX]` uses one-based track numbers;
carrier 0 means OFF, band indexes 0/1/2 mean 8/12/24, and omitting the index
preserves the band count. `split INPUT CONTROL REPLACEMENT_MASK` copies the
control globals and the tracks selected by the decimal bitmask (bit 0 is
track 1; 56 replaces tracks 4–6) with their steps, FM6, vocal IDs and MIDI
channels. Other project-level fields stay on the input. Both modes use the
firmware codec and verify a round trip before emitting a project. The loader's
`frames` action emits SysEx offline for comparison, not for blind replay.

## Bench-only firmware reference

[`ram-defaults.patch.txt`](ram-defaults.patch.txt) is the archived diagnostic
diff against `9b9b3e6`, not an applied or shipping change. It skips persistent
boot initialization and last-session restore. The package used for #73 was
built on that baseline with hardware assets and
`BGIRL_HARDWARE_ASSETS=1 ./felucca/build.sh --release 0.2-ramprobe`; it reported
`FELUCCA v0.2-ramprobe` over MIDI. The diagnostic also changes image layout:
a successful boot doesn't identify the cause of the separate baseline boot
loop. No stock firmware, flash backups, or firmware binaries are included here.

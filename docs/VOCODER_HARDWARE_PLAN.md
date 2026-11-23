# A real-time live vocoder for the M-VAVE FM-1

[Hardware failure #74](https://github.com/track-forge/bgirl-fm1/issues/74) · [bench runbook and commands](https://github.com/track-forge/bgirl-fm1/blob/e8da86b58096ff49786f817230f10e45945dc2d4/tools/audio-bench/README.md) · [bench tooling PR #75](https://github.com/track-forge/bgirl-fm1/pull/75)

**Target:** intelligible, live two-input vocoding in the full six-track Ticker project without missing a 2.90 ms half-buffer deadline. Keep the existing 24-band sound as the reference. At eight bands with an ANALOG carrier, the whole render peaked at 13.313 ms playing; at 24 it reached 32.152 ms stopped. These are boot maxima, not isolated DSP timings. Twelve bands is unmeasured. The separate [boot-loop investigation](https://github.com/track-forge/bgirl-fm1/issues/73) is not a prerequisite for profiling the running diagnostic.

## 1. Prepare before borrowing the operator

- Start from current main. Read the vocoder contracts and clarity verification in docs/, the capture/join in fx.c, and felucca/firmware/src/bgirl_vocoder.c. Keep the existing float kernel as an offline reference. Render the same overlapping speech/carrier phrase for comparisons; retain wet-only and matched-level reference audio.
- Use tools/audio-bench from main if landed; otherwise take the tools from #75 in a separate checkout. Verify fixture hashes, generate T3-owner/T5-ANALOG OFF/8/12/24 variants with identical globals, and stage them together. The fixtures contain project data, not audio samples. Locate the intended vocal assets before asking for the final listening pass.
- The operator's private bench note identifies the SSH host, last device nodes and existing logs. Re-enumerate MIDI and console, identify firmware, uptime, transport and project; do not assume the last state survived. The last known RAM-defaults diagnostic had Miami unmounted. Preserve the current project if needed before replacing it. Do not ship the RAM-defaults patch.

## 2. Measure without a reboot per result

In one diagnostic build expose interval-resettable RAM counters for total render, capture/join, absent/silent-input handling and bank processing. Count calls and input activity as well as elapsed time; export outside the audio IRQ. Measure timer overhead. Compare route OFF, route active with bank bypassed, then real wet processing. Bypass is a timing control only. Use the same passage and compare stopped, sounding, and tails.

The agent runs imports and CSV collection remotely. Use the runbook's acknowledged live MIDI loader; change project variants in RAM, not by flashing. Capture before import, after import, after five seconds stopped, and through transport. Arm capture first, then give the operator one cue: PLAY, STOP, PLAY, STOP. Pull audio observations and timings together. Use short sweeps for diagnosis; reserve a full loop for finalists. If a known failing case wedges, save partial CSV, check uptime and device identity, and resume at the next useful case rather than repeating the failure. Request a power cycle only when recovery needs it.

Preserve build SHA/options, fixture hash, route, asset availability, uptime, interval peak and late-count delta, wet audibility and transport behavior per run. Never subtract boot peaks to estimate a stage. Log new late blocks only within a boot. Group diagnostic switches into one image where practical; coordinate firmware changes with the operator, with the last working diagnostic available for recovery.

## 3. Choose the smallest change that buys measured headroom

1. If capture or missing-input handling dominates, fix that path first. Skip inactive work only if release tails, Freeze state and the first returning syllable remain correct. Repeat with both sources active.
2. If the bank dominates, inspect the optimized JieLi pi32v2 assembly. Move invariant gains/conversions out of the sample-band loop; compare the optimized float bank to a fixed-point multiply-accumulate prototype on target. Budget coefficient precision, accumulator headroom and saturation; test low bands, hot inputs and decay into silence. Pick by measured whole-render improvement, not desktop speed.
3. If still over budget, prototype a multirate bank: anti-alias before decimating lower bands, reconstruct and align their output, retain a high-rate consonant path. Try slower envelope updates with interpolated gains separately so changes in timing and sound can be attributed.
4. If 24 cannot meet the deadline with margin, compare purpose-voiced 12 and 8 against the reference at matched loudness. Report timing, intelligibility and character on the same phrase. Propose the winning mode and explicit handling of existing 24-band projects; do not silently relabel a smaller bank or close #74 as a 24-band pass.

## 4. Evidence for the implementation PR

Run existing vocoder/dual-input and render checks; verify wet/dry, missing carrier, Freeze/release, mode changes and STOP → PLAY on the changed path. On the physical unit, load Ticker with original globals, intended carrier and actual vocal material. Confirm wet-only output during overlap (and that removing the carrier removes the vocoded signal). Then play the normal mix through a full loop and restart. Require no breakup, no new late blocks, responsive transport and worst observed whole-render time below 2.90 ms. Report actual margin and repeat the winning configuration to check reproducibility. A silent run, an emulator render or zero late blocks alone does not pass.

Attach before/after measurements, fixture/build identifiers, listening result and any band-limit decision. Distinguish diagnostic-only proof from a verified shipping build. If assets, hardware or an operator are unavailable, finish offline work and identify the exact outstanding hardware test rather than calling it fixed.

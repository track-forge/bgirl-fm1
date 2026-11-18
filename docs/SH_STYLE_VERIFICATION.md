# SH-STYLE basics-round evidence — issue #31 / PR #33

> Historical initial implementation results below refer to pre-rebase02a4a2b.
> Current-main rebase and user-authorized preview: [SH_STYLE_PREVIEW.md](SH_STYLE_PREVIEW.md).

Implemented on `plan/sh-style-analog-bank`, stacked on **PR #32** branch
`plan/sh01a-patch-gathering` at `4a1cb94db67da1c0e3f6f105f650e3c5b4125583`.
Parent capture implementation is `27bbf5179cfc2c8d3707ce2156680485310a378d`;
its subsequent docs head has passing Python3.9/3.12 and broader browser-host CI.
The two real inputs are explicitly accepted; parent need not be merged. See PR
for the exact pushed implementation head and current hosted checks.

Concurrent main at implementation is `5a107f6ce772b9dac4bafdf1198aa4895e5238ed`
(Mega demo PR #28). It is not silently folded into this child: the required base
is PR #32, not main. This child changes no startup/demo file; the base-to-head
diff contains the Part2 plan/implementation, not the parent capture tooling.
No duplicate PR, force-push, merge, deployment, hardware build, flash, new MIDI
capture, DAW mutation, reference-audio capture or listening gate.

## Input and reproducibility evidence

Both private schema-v1 bundles passed raw-prefix hash/length validation and replay
using their original private session/port identities. Snapshots/logs are unchanged.
The child publishes only curated recipes, per-field disposition reports and
sanitized hash references. Conservative `partial` capture confidence is retained
separately from uncalibrated translation fidelity. No source redistribution
license is invented. Earlier “2-1” labels are historical: Doug confirmed **1-1**.
Wire-PC1 is **not** called 1-2 or assigned a guessed human slot.

| Source | Snapshot SHA256 | Raw SHA256 |
| --- | --- | --- |
| Confirmed proving 1-1 | `f30d053bc4c5381494365c5cdc16e0e2e37f76539b5f85805eb86c86432503e8` | `534c9e37e89c217df9c4ece6e7738cc08874e57d5730544e734386d31775f29b` |
| Distinct saved wire-PC1 | `9c0dee5a41dd5a5acd4fb775ba1bb71257b1cb4c8fe1d77f117616777f275a6c` | `9d605255a68f924343c2efbf330b199054923aa5f4f773e7b2c1b2fdba2f447f` |

Every report covers all36 source candidate fields, all127 target parameter fields,
and every source metadata key. No full raw parameter tables, source event logs,
port names, timestamps, machine usernames or session IDs appear in public assets.
The recipe itself is public authorized curation, not a full source snapshot.

Semantic recipe SHA256 = parent canonical sorted compact JSON + LF (not pretty
file bytes). Reports carry policy, translator and target descriptor hashes too.

| Recipe | IDs | Semantic recipe SHA256 |
| --- | --- | --- |
| SH 1-1 | ANALOG0 / preset13 | `d08447ab682aab3c3bacdb1f2330a856f4d7f94c10b4bef41fd2c3e23b9e623f` |
| SH WIRE1 | ANALOG0 / preset14 | `7aad631a2ec308d537fb11c65ae864c39556ea71e133dadbe40402750ab65113` |

Policy file SHA256: `a1bd94ba115dd3fd4ee9a07b083904dc788d8232a25984c7b4fb0bc91fbe9b14`.
Translator SHA256: `87d74cfd88f428011c895785f08f5b910175279a0a9164e5712bf3bcd99fdcb0`.
Pretty recipe file SHA256s: 1-1 `bc01a4df8f76c81d7961c8a923ef8242ad0a272e642ae3b19469ccbbfe96040c`;
wire1 `8fc45672afb5594fe30f7593dedca174e2cfe860bfb745d5d07bd904df234c93`.
No random values or wall-clock output fields.

## Losses and normal loading

Both have15 approximated source fields,21 explicitly unsupported source fields,
no missing source fields,9 common limitations plus1 patch-specific mixer loss.
All selector encodings remain unknown, not guessed from observed raw codes.

- **SH 1-1:** target SUB (saw + octave-down square), heuristic balance42. Source
  pulse dropped, stored source chord not synthesized. Cutoff uses the saved106,
  never the edited65 experiment. Filter LFO depth50 and filter ADSR depth12 are
  normalized target choices, not equivalent measured depths.
- **SH WIRE1:** target PWM, centered width. Source sub dropped; pulse/sub cannot be
  recreated together in that mode. Filter envelope depth55, cutoff25 and ADSR
  use normalized target codes, with no source physical-time/frequency claim.
- Both: LP SVF ≠ source filter; target mono retrigger ≠ decoded source assignment;
  source range/octave, portamento, bend/mod sensitivity, sub type, gate/envelope
  mode, chord state, PWM routing, LFO waveform/mode and noise color are not
  faithfully reproduced. One target amplitude ADSR also drives filter modulation.
  Target triangle LFO is an explicit assumption. No extra track or new DSP.

One generator owns ANALOG wrapper generation, so carrier and collection cannot
overwrite each other. Factory load applies explicit values within the existing
sound transaction/panic path; remaining sound fields use normal descriptor
defaults. Existing track-owned mixer/scale/ARP/chord-key/sequence state is kept.
SH-STYLE category10 appends after SIGNATURE9; existing IDs/categories are retained.
Host tests check byte-identical inherited presets0–11 and unchanged carrier12.
No engine/BSP gitlink, frozen default, startup/demo or persistence format changes.
The generic regression harness sees two new unbaselined renders: dedicated
normal-load native/WASM tests verify them; no old golden is rewritten.

## Commands and technical acceptance

See [offline reproduction and user instructions](../tools/sh_style/README.md).
The normal browser loader selects either sound on any of six full tracks; normal
PRESETS → ANALOG or LIST SH-STYLE offers both. ENV/LFO/EDIT, sequencer, locks,
MIDI and SAVE are ordinary instrument operations. Existing eight shared voices
and3840-byte B6S1 capacity/FULL atomic refusal remain the contract.

Hosted CI is reported independently on PR #33, never inferred from a local pass. No acoustic
similarity, human listening, physical controller, hardware-fit or CPU deadline
claim is made. Standalone editor mock DSP is not the acceptance instrument:
real firmware editor protocol tests cover names, loads and parameter GET/SET.

## Final local results (2026-11-18)

- Parent offline capture suite: **19 PASS**. Translation suite: **11 PASS**,
  including exact real-source recipe/report reproduction with no source-byte changes;
  public CI deliberately skips the private-source-only test. Determinism, parent
  schema/definition/range rejection, endpoints/clamps, missing/strict handling,
  unknown selectors, mixed-wave losses, bounded/rationale-bearing overrides,
  synthetic/stale/edit/hash rejection, sanitization and unchanged old recipes covered.
- Root `./build.sh --web`: **PASS**, Emscripten6.0.10. WASM917035 bytes,
  SHA256 `b864378d9d4db183e09b7522d922cf8cd710b283f27217a589ac677553cbf17d`.
- Full `./tests/run_tests.sh`: **PASS**. All109 inherited/signature/carrier golden
  entries unchanged (111 renders including2 unbaselined new ones), no health,
  voice/routing or crash failures. Upstream heavy native/WASM parity88200 frames;
  clarity parity176400 frames; new SH-STYLE parity**176400 frames bit-identical**,
  including note release, finite/bounded/nonzero outputs. Dance disabled/hidden/
  CRISP/GHOST audio preservation512000 stereo frames and4000 RNG/note snapshots.
- SH-STYLE host acceptance: both sounds × six full tracks; descriptor bounds;
  byte-identical old factory presets/carrier, category enumeration, retained
  sequence/mixer/scale, learned MIDI parameter, step lock and motion, user preset
  and B6S1 project roundtrips, real MIDI note dispatch on explicitly configured
  per-track channels, note-off across track changes, mono retrigger, preset-load
  panic, explicit panic and sequencer STOP: **PASS**. Combined six-track/eight-voice
  exercise passes; last measured project1405/3840 bytes.
- Real firmware editor framing `7D 46 36`: both preset names/IDs, load and parameter
  SET/GET on all six tracks **PASS** in the full editor suite.
- Actual Chromium: new SH-STYLE desktop/mobile selected-track loading on all six
  tracks, unchanged surrounding tracks, normal PRESETS ordering, ENV attack edits,
  MIDI signal/release, transport STOP, normal SAVE + fresh-page autosave restoration
  for both presets, no overflow and **zero page errors PASS**. Peaks0.0875549 and
  0.1004944, timing-dependent technical measurements, not loudness/fidelity proof.
- Surrounding actual-browser suites: splash, browser smoke, vocoder, demo, vocal,
  dance, six-track and Signature. Six-track acceptance includes3840-byte FULL
  atomicity and unchanged flash/live behavior. Individual logs stay ignored/local.

Initial failures were test/integration findings, not waived: category table length
needed the additive entries; a fixed traversal count needed to use the bank count;
normal browsing continues to BG-first FM6 rather than wrapping ANALOG; HOME has
track level knobs, so browser editing uses ENV; default MIDI block is still1–4,
so the all-six test explicitly selects per-track routing. Live held notes remain
held on STOP by design: tests distinguish MIDI note-off, panic and sequencer STOP.
No DSP was changed to satisfy those assertions; no golden was rewritten.

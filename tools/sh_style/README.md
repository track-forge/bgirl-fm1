# SH-STYLE basics-v1 (issue #31)

Pure offline translator, no MIDI/live dependencies. Two real saved-patch captures
are the accepted input set: confirmed proving **1-1** (supersedes the historical
2-1 arm label) and a distinct repeated **wire-PC1** source (human slot unknown).
The parent `tools/sh01a/snapshot.schema.json` is used unchanged. No source bytes,
ports, timestamps, usernames or raw logs ship in the collection. Public recipes
are authorized curated derivatives, not a redistribution license for the sources.

## Reproduce a recipe

From repository root, Python 3.9+:

```sh
python3 -m venv local/venv
local/venv/bin/pip install -r tools/sh01a/requirements-test.txt
local/venv/bin/python -m tools.sh_style translate /private/source/bundle \
  --name 'SH 1-1' --preset 13 --strict --output local/translation-1-1
# Same for the second bundle: --name 'SH WIRE1' --preset 14, a fresh output path.
```

A bundle is an existing immutable `patch.json` + `events.jsonl` pair. Translation
validates parent schema/version, CC identities/ranges, hardware evidence, baseline
state, raw hash/byte count, and raw replay. `partial` capture confidence is retained,
not promoted to acoustic confidence. The exact private input inventory/hashes are
referenced in `assets/sh-style/manifest.json`. Raw source filenames/identity labels
never get copied into reports. Outputs are deterministic readable JSON; semantic
recipe hashes use parent `canonical()` (sorted compact JSON plus LF).

`--overrides overrides.json` accepts only already-translated output parameters:

```json
{"P_E4":{"value":90,"reason":"Explicit brighter design variant; not calibration"}}
```

Ranges are rejected, not silently accepted; only normalized mappings clamp to
target bounds. Missing patch candidates default to raw0 with a per-field missing
loss; use `--strict` for the shipped sources. Unknown selector bins are never
inferred. Policy changes need a new reviewed implementation/version, not arbitrary
replacement coefficients. Source material is not available in public CI.

## Policy and losses

`assets/sh-style/basics-v1.json` defines normalized half-up/clamped mappings. The
pinned ANALOG descriptors and common descriptors are hashed in each report. No
source Hz/ms, selector thresholds, calibrated filter response or source oscillator
absolute gains are claimed. Target units shown by the normal UI remain **target**
units only. Both recipes are single-track ANALOG sounds, not a new engine.

- **SH 1-1**, engine0/preset13: dominant saw + target octave-down square SUB,
  source pulse omitted. Source stored chord not reproduced; host chord keys keep
  their existing selected-track state, as normal sound loading requires.
- **SH WIRE1**, engine0/preset14: dominant pulse → PWM with centered width,
  source sub omitted. No source voice-mode/poly/unison inference.
- Both use mono retrigger, LP SVF, shared amplitude/filter ADSR, triangle LFO,
  normalized filter/envelope/LFO mappings and no glide. Range, gate, sub type,
  PWM routing, noise color, bend/mod sensitivity, chord and LFO selector laws remain
  explicitly unsupported. All source candidate IDs receive a disposition, each
  target parameter has provenance/default/retained-state disposition, and all
  source metadata fields have a private/sanitized disposition. Reports do not
  reproduce source raw parameter tables.

Recipes describe a normal factory sound load: `values` are explicit assignments;
all other sound parameters use `TP` descriptor defaults, and `param_kept` values
(pattern/scale/ARP/mixer/chord keys etc.) remain the selected track's. Reports name
all127 target parameters individually and hash those descriptor definitions.
Dry FX sends are an explicit recipe choice; LEVEL/PAN/MUTE are not changed.

## Build and test

There is one wrapper owner, `tools/carrier_bank.py`: inherited ANALOG presets0–11,
unchanged BG CARR SAW12, SH-STYLE13–14. No engine/BSP pin changes. The hook runs
inside the normal factory load transaction, before audio render, with normal panic,
undo and persistence; no alternate DSP or saved-project format.

```sh
python3 -m tools.sh_style build-bank
python3 -m tools.sh_style build-bank --check
local/venv/bin/python -m unittest discover -s tools/sh_style/tests -v
# Private exact-source reproduction (never required in public CI):
SH_STYLE_SOURCE_ROOT=/private/capture-root \
  local/venv/bin/python -m unittest discover -s tools/sh_style/tests -v
# With Emscripten and build dependencies activated per root README:
./build.sh --web
./tests/run_tests.sh
# With the local README server, Playwright and Chromium:
BGIRL_URL=http://127.0.0.1:8798 node tests/sh-style-browser.cjs
```

The two new generic regression renders are intentionally unbaselined; all existing
109 golden entries stay unchanged. Dedicated normal-load native/WASM tests cover
both new recipes and release sample-for-sample. Generated technical signals are
not recordings of the hardware and are not fidelity/listening approval.

## Play normally

Switch on the browser instrument. Select T1–T6 with **ALGORITHM**; choose a sound
in **SH-STYLE** and **Load SH-STYLE on selected track**. On the panel use
**PRESETS → ANALOG → SH 1-1 / SH WIRE1**, or LIST **SH-STYLE**. Normal ENV/LFO/EDIT,
sequencer, parameter locks, MIDI and SAVE work. Source chords are not installed;
existing host chord settings are retained. Loading does not erase patterns or
saved slots, restart a demo, or consume extra tracks. Six full tracks share eight
voices. B6S1 3840-byte capacity and atomic FULL refusal are unchanged.

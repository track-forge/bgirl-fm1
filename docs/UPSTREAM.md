# BGIRL migration provenance

## Vocoder follow-up delta

The scoped follow-up implements [VOCODER_CONTRACTS.md](VOCODER_CONTRACTS.md) on this
same pinned tree: bundled original speech through SLICE, count-mapped controls,
post-join capture/processing, normal FX pages, bounded worker-failure recovery and
FUNB compact project writes with all legacy readers. No gitlinks, flash addresses,
engine numbers, release/install workflows or device identities change. New files
retain GPL-3.0-only; generated speech has its separate rights/provenance manifest.
The baseline-only statements below are historical; current evidence is in
[VOCODER_VERIFICATION.md](VOCODER_VERIFICATION.md).


## Selected source and import

- Felucca **v1.8.1**, annotated tag object `71f1cf70ee2e732f34671c47cc6e83f5104803f3`,
  resolved source commit **`f4df2964649f301ba5abd0ec6000110068672f25`**.
- Source: <https://github.com/hugelton/Felucca/tree/v1.8.1>.
- Engines: `d0260074ce100a47675814b7f38d61f11176c926` (MIT).
- BSP: `5942e68b89ab9ccd23f2c541d712ca38fe635f11` (MPL-2.0).
- Both are upstream's exact gitlinks, not newly selected dependency versions.
- v1.8.1 includes/documented browser support. Inspected v1.8.2
  `6162de864075ad9edb8cd35e0e45ba75faa5d0db` removes it and documents FM-1 only.
  This overrides the planning note's unverified stable-1.8.2 assumption.

Imported full v1.8.1 tree under `felucca/` with `git merge -s ours --no-commit
--allow-unrelated-histories v1.8.1` then `git read-tree --prefix=felucca/ -u v1.8.1`.
Merge commit `59fb7ad` retains both parent histories. Root submodule paths were
prefixed deliberately. Upstream's internal relative dependency/build paths stay intact.
The unmodified baseline WASM and Chromium checks passed before BGIRL source edits.

Prototype checkpoint `4cc1e11e50ffb1d2a06a50547780b9e828a8b15f` is an ancestor.
Its app/base/tests were removed from the active tree to avoid two competing apps;
original vocoder, speech, rights/conversion notes and render harness remain there.
Historical reports: PROTOTYPE_BUILD_STATUS.md, PROTOTYPE_UPSTREAM.md, VOICE_SPIKE.md.

## Review map

The full upstream import exceeds GitHub's 20,000-line aggregate PR-diff limit.
Review the import by its tree identity (unmodified `59fb7ad:felucca` equals
`f4df2964649f301ba5abd0ec6000110068672f25^{tree}`), then review implementation
commit [`1bc9d29`](https://github.com/track-forge/bgirl-fm1/commit/1bc9d29ebeae69c8bb045e7ca36c541616245fc1).
The [compact upstream delta](BGIRL_DELTA.patch) contains all five changed upstream
files (84 insertions, 9 deletions), generated with:

```sh
git diff f4df2964649f301ba5abd0ec6000110068672f25^{tree} HEAD:felucca
```

It is a review artifact, not a build-time patch or second source tree. Regenerate
it after any upstream-source changes. Root wrappers, tests, CI and docs are in the
implementation commit, separate from the import.

## Delta against v1.8.1

- `firmware/src/engines.c`: separate `BGIRL_TRK_DEF` palette; no factory preset,
  pattern, engine numbering or `TRK_DEF` edits.
- `firmware/src/main.c`: startup uses the palette plus ordinary editable demo
  steps before upstream autosave restore. No change to render/transport/MIDI/clock.
- `web/emu/felucca_web.c`: explicit blank/demo operation via upstream project_new,
  engine/preset loaders and sequencer data. No new flash format or project slot overwrite.
- `web/emu/worklet.js`: handles that explicit action outside process/render, clears
  queued panel events, returns success/failure. Audio render itself unchanged.
- `web/emu/index.html`: BGIRL name, header colors/role labels, confirmed project
  buttons, separate browser storage key, source links instead of installer links.
  Inherited panel geometry, controls, keyboard, Web MIDI handler, screen/UI retained.
- Root-only browser build/generator entry point, host subset runner, focused delta
  and Chromium acceptance tests, build-only CI, documentation/license entry points.

No DSP changes, vocoder, speech/sample additions, animation, protocol or file-format
changes. Legacy formats using "default sound" still get original Felucca defaults;
explicit modern projects preserve their sounds. Native CLEAR ALL remains upstream;
the separate Blank electro template button provides BGIRL roles without demo steps.
Browser key isolation does not automatically import stock Felucca/prototype data.

To compare upstream without prefix noise:

```sh
python3 - <<'PY'
import subprocess
up = subprocess.check_output(['git','rev-parse','f4df2964649f301ba5abd0ec6000110068672f25^{tree}'], text=True).strip()
local = subprocess.check_output(['git','rev-parse','HEAD:felucca'], text=True).strip()
subprocess.run(['git','diff','--stat',up,local], check=True)
PY
```

## Notices and assets

Combined derivative GPL-3.0-only; all upstream headers, credits, LICENSING.md,
LICENSES/, engine/BSP licenses retained. Fonts: bundled Inter Tight, Material
Symbols and DotGothic16 keep SIL OFL notices; Fukiai's separate notice retained.
Samples generated by upstream tooling retain upstream CC0 pack and bundled
Versilian CC0 attribution under assets/. No commercial-record samples or new
external sound dependency. Ported DSP retains MIT/Apache/LGPL notices as enumerated
by upstream LICENSING.md. Optional ADPCM is not enabled by this milestone.
No claim of trademark endorsement or completed device/distribution review.

The current review-aid patch normalizes whitespace-only context lines after
`git diff`; `git apply --numstat docs/BGIRL_DELTA.patch` validates its diff syntax.
This prevents a diff-of-the-diff from reporting blank context markers as newly
introduced trailing whitespace. It is not a runtime/build dependency.

## Six-track compact format and control study (2026-11-17)

- JIANT: <https://github.com/juanjiant-bit/JIANT/tree/da8baa1eed7c4e75f1ed9bcf433797b3aeace107>,
  GPL-3.0-only. Inspected `firmware/src/project.c` (`proj_packed_size`,
  `proj_step_empty`, `proj_step_x/size`, `proj_fm_kept`, `proj_pack`,
  `proj_unpack_c`, `proj_full`) and `tests/project_test.c` six-track round-trip,
  dormant-note, damaged-record and full-section tests; also persistence tests.
  Originating six-track commit:
  [`ee130ccac084e85ff660a7ae21dfcee5ea91335f`](https://github.com/juanjiant-bit/JIANT/commit/ee130ccac084e85ff660a7ae21dfcee5ea91335f).
  Adapted sparse bitmap/conditional-field/patch and preflight-before-write ideas
  into BGIRL's `project.c`, plus exact-round-trip/full/fuzz test ideas in
  `tests/six_codec_test.c`. Retained Leo Kuroshita and Juan Ignacio Antonucci
  portions copyright/SPDX. New explicit LE records, B6S1 magic, frozen defaults,
  full dormant flags/notes, wider motion and two-pass atomic decoder are
  BGIRL-specific. No JIANT migration readers, engine IDs, extensions or storage
  addresses imported. Its FUNC magic is deliberately not reused.
- Skiff: <https://github.com/illuminus5/skiff/tree/b257ddf6a15a21f6c5956abc9c19c0571daa8e15>,
  GPL-3.0-only, Leo Kuroshita copyright retained in our existing derivative files.
  Studied `firmware/src/ui.c` six-track selection/four-knob conventions and
  `firmware/src/project.c` (`proj_old`, explicit OLD FORMAT refusal).
  Adapted the explicit-refusal UX principle, not its converter or pattern pool.
  BGIRL uses two selection-following banks, six ordinary engines/track, and
  eight shared voices; no Skiff light-track restrictions or voice budgets.
- `project_defaults.h` is a frozen snapshot of the existing pinned BGIRL
  parameter descriptors and generated FM6 INIT. No dependency pin, sample,
  artwork, branding, workflow installer or device identity was imported.

The obsolete FUN1–FUND project converters are intentionally removed for this
explicitly authorized clean break. Earlier provenance sections describe the
historical four-track versions, not current project-format compatibility.
[Capacity decision](SIX_TRACK_CODEC_SPIKE.md) · [verification](SIX_TRACK_VERIFICATION.md).

## Clarity tranche local adaptation

`bgirl_carrier.h` is generated by `tools/carrier_bank.py` from the pinned MIT
ANALOG presets/descriptor, appending BG CARR SAW. Imported engine source/gitlinks
are unchanged; inherited presets retain their IDs and samples. Firmware-only
format IDs 14/15 add vocoder-ms display without editing the engine API.
[Additive count/default contract](VOCODER_CLARITY_CONTRACT.md) preserves B6S1
126-count saves while writing 127-count records; frozen default values unchanged.
See [host/browser evidence and target limits](VOCODER_CLARITY_VERIFICATION.md).

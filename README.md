# BGIRL FM-1 — Felucca electro edition

**Felucca electro defaults + a sample-fed vocoder listening checkpoint.**
Independent GPL-3.0-only derivative of Felucca, not an official Hügelton build.
Browser development milestone only: **do not install on hardware**.

![BGIRL browser edition](docs/img/browser.png)

## Run locally

Requires Git, Python 3.11+, C compiler/libm, Node 24, Emscripten **6.0.10**.
Pillow must have FreeType/Raqm support (ordinary Linux wheels do).

```sh
git clone https://github.com/track-forge/bgirl-fm1.git
cd bgirl-fm1
git checkout feat/sample-fed-vocoder # unmerged review, stacked on planning PR #5
git submodule update --init --recursive
python3 -m pip install Pillow==12.1.1 fonttools==4.68.0
# Install/activate Emscripten 6.0.10 using emsdk; then:
source /path/to/emsdk/emsdk_env.sh
./build.sh --web
./tests/run_tests.sh
python3 -m http.server 8767 --bind 127.0.0.1 --directory felucca/build/emu
```

Open <http://127.0.0.1:8767> and click **Switch on**. Audio needs this user gesture.
The wrapper calls Felucca's own asset generators and `web/emu/build.sh`; no JieLi
SDK, device package, installer, paid service or network-fetched sound is needed.
Build-time fonts and sample sources are bundled upstream with their notices.

If Emscripten is not installed, create a separate SDK checkout (do not clone over
an existing installation):

```sh
git clone https://github.com/emscripten-core/emsdk.git /path/to/emsdk
git -C /path/to/emsdk checkout 708dcbcc9ee23bc7d2dad44fc90db377e38b771c
/path/to/emsdk/emsdk install 6.0.10
/path/to/emsdk/emsdk activate 6.0.10
source /path/to/emsdk/emsdk_env.sh
```

## Sample-fed vocoder / early listening demo

Switch on, then **Load dry speech**, **Load8-band vocoder** or **Load12-band
vocoder** below the panel; confirm replacing the current unsaved project. Press
PLAY. The wet setups keep the electro demo drums/bass and use a **track3 sustained ANALOG saw
pad** as track4 speech's carrier, with dry pad monitoring OFF. The dry-speech
button stays isolated. Saved slots are not erased; startup defaults remain.

- Track4 chromatic keys wrap four phrases, at original pitch, one-shot/retrigger.
- Select track3 (ALGORITHM) and play pad/carrier notes without restarting speech.
- Track4 FX pages2–4 (immediately after FX): VOCODER (carrier, bands, wet, dry monitoring), VOC INPUT
  (live HOLD/Freeze, trims, consonants), VOC COLOR (three character groups/release).
  Tap FX **slowly**, or use SELECT after opening FX; quick double-tap locks the
  inherited performance layer, whose FRZ is a different effect.
- MON OFF hides the carrier's direct output/sends while retaining its audio feed.
  Ordinary mute silences the feed. STOP/panic ends speech and live Freeze.
- Expand **reference renders** for dry/8/12/consonant/chorus comparisons.
  These original isolated ANALOG references are not the live pad groove.
  Unnormalized levels differ. Doug reports the overall vocoder experiment is a win;
  a final band preference/intelligibility curation decision remains open.
- Save projects normally. Old saves load with vocoder OFF. New FUNB saves keep
  bank/routing/controls but load unfrozen; old firmware rejects this new format.
  Sound presets clear project-specific carrier references.

[Contracts](docs/VOCODER_CONTRACTS.md), [verification / exact commands and limits](docs/VOCODER_VERIFICATION.md),
[speech provenance](assets/speech/README.md). No physical-device performance proof.

## Starting palette and demo

| Track | Sound | Pattern |
| --- | --- | --- |
| 1 | DRUM / TIGHT KIT | Original 16-step kick, snare+clap, hats and bell |
| 2 | ANALOG / SQR BASS | Upstream OFFBEAT, mono legato bass |
| 3 | FM6 / BELL | Upstream sparse BELL, metallic/icy reply |
| 4 | VOICE / VOX LEAD | Original sparse reply; **synth placeholder, no words/vocoder** |

Fresh browser state loads this editable demo **stopped at 120 BPM**. A valid
last-session autosave takes precedence. All engines/presets remain selectable;
this is a curated starting palette, not a restricted instrument. Shared upstream
voice limits still apply. Sounds have automated signal evidence, not human taste approval.

**Load electro demo** and **Blank electro template** replace only the current
unsaved project, after confirmation. Both use Felucca's project reset/sound-load
machinery, stop transport, release voices and preserve saved slots/user presets.
The blank template has the four sounds and no steps. Normal Felucca MENU > SYSTEM
> CLEAR ALL retains its original factory defaults; it is not the BGIRL template button.

### Short demo recipe

1. Switch on fresh (or explicitly Load electro demo). Start with low MASTER; press PLAY.
2. Turn **ALGORITHM** (keyboard ←/→) to select each of the four tracks.
3. Play Z–' / Q–O keyboard notes; release them. Drag/scroll the labeled knobs;
   focused knob arrows also work. SELECT browses pages, PRESETS browses sounds.
4. Stop with PLAY. Edit a sound and use Felucca **SAVE**, **OCT+**, **OCT+** to
   confirm a user preset. Use its normal SAVE > PROJECT page to save/load a project.
5. Use Blank electro template, edit or load saved material, then optionally reload
   the demo. Autosave restores changed music after at least 10 quiet seconds and
   the inherited 60-second write-spacing limit; a page reload immediately after an
   unsaved edit is **not** a guaranteed save.
6. Stop, release keys and let effect tails decay. No prototype HOME-panic/ARP-start
   remappings: these are Felucca's normal controls. Read its [manual](https://github.com/hugelton/Felucca/wiki/Home).

Portrait uses upstream's tall responsive panel. Desktop/mobile screenshots and
verification details: [build status](docs/BUILD_STATUS.md).

## Architecture, history and licensing

- Active instrument: `felucca/`, ancestry-preserving import of **v1.8.1**. Upstream
  v1.8.2 removed browser support; it is intentionally not this baseline.
- Exact pins, license inventory and small upstream delta: [provenance](docs/UPSTREAM.md).
- Browser flash key `bgirl-felucca-v1-flash` isolates this edition from stock
  Felucca and the retired prototype. Same Felucca data formats; no device/prototype migration.
- Prototype source, original speech, vocoder and tests remain reachable at
  [`4cc1e11`](https://github.com/track-forge/bgirl-fm1/tree/4cc1e11e50ffb1d2a06a50547780b9e828a8b15f).
  [Prototype build evidence](docs/PROTOTYPE_BUILD_STATUS.md) and
  [voice findings](docs/VOICE_SPIKE.md) are historical, not shipping features.
- GPL-3.0-only combined derivative: [LICENSE](LICENSE), upstream
  [credits](felucca/CREDITS.md), [licensing](felucca/LICENSING.md) and
  [component texts](felucca/LICENSES). Engines retain MIT; BSP retains MPL-2.0;
  fonts and CC0/other assets retain their notices. `LICENSE.upstream` is the old
  prototype's MPL notice, **not** the license of this combined application.
- Vocoder sound approval/target validation, [silhouette dance page #2](https://github.com/track-forge/bgirl-fm1/issues/2),
  device identity/storage review and physical MIDI/clock/performance testing remain follow-ups.

No release/site workflow is active. Root CI only builds/tests. Workflows retained
under `felucca/.github/` are imported source, not GitHub Actions entry points.
No merging, flashing, releases or site publication is part of this milestone.

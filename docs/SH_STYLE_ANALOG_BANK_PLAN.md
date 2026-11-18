# SH-STYLE bank via ANALOG — basics round implementation plan

Status: plan only; implementation is BLOCKED on https://github.com/track-forge/bgirl-fm1/issues/30.
Own issue: https://github.com/track-forge/bgirl-fm1/issues/31. Continue this PR when assigned after Part 1 delivery.
Planning baseline: main `c51c53885fd18c1e84be676d1644143803bc2e0b`.

## Product decision and scope

Round one turns captured SH-01A patches into a named **SH-STYLE** collection using
BGIRL's existing ANALOG engine. This is an approximation bank + deterministic
translator, **not a new synthesis engine**, not FM conversion, and not an exact
Roland emulator. Keep ANALOG available under its current name/ID; SH-STYLE names
the collection. No new DSP/filter topology, poly/unison emulation or SH control
panel is required. No reference audio, acoustic analysis, calibration or listening
acceptance gate in this round. Technical generated-signal checks remain appropriate.

Use the real captures produced by Part 1, including patch 2-1 and at least one
other distinct saved patch. Do not substitute invented presets for missing inputs.
The goal is playable, editable approximations and honest reports of their limits.
Do not alter startup/demo arrangements, user saves, existing preset identities,
upstream engine/BSP pins, service previews, or device state.

## Dependency gate and branching

Before implementation, record Part 1's delivered commit, schema version, capture
inventory, source hashes and known gaps. Confirm at least two real captures are
available and accepted as the input set. Schema readiness alone is insufficient.
If hardware work is blocked, this implementation stays blocked too.

Both planning PRs initially target main independently; a plan-only merge does not
satisfy the dependency. Prefer starting runtime implementation after Part 1's
implemented delivery is merged, then update this branch from main. If Part 1 is
accepted but unmerged, explicitly stack this PR on its implementation branch and
record the dependency; do not copy/diverge the schema or merge it without approval.
Recheck current source and IDs, since concurrent BGIRL work may have landed.

## Current architecture and integration points

Active firmware is `felucca/`, not the historical `app/` prototype. Relevant sources:
- `felucca/firmware/engines/src/eng_analog.c`: existing oscillator pairs, SUB/PWM
  modes, noise, drive, state-variable filter and factory preset definitions;
- `felucca/firmware/src/engines.c`: engine table and normal preset integration;
- `tools/carrier_bank.py`: local generated ANALOG wrapper appending BG CARR SAW;
- `felucca/firmware/src/bgirl_carrier.h`: generated wrapper used by engine table;
- `assets/signature/manifest.json` and Signature bank integration: recipe/provenance
  and normal panel + browser selected-track loading precedent;
- `felucca/firmware/src/project.c` and frozen `project_defaults.h`: persistence;
- root `build.sh`, tests and browser/editor code: build and parity surfaces.

ANALOG has waveform modes, not independent simultaneous saw + pulse + sub gains.
Its resonant SVF is not a calibrated SH-01A filter. A matching numeric 0..127 range
does not mean equivalent frequency, time, gain or resonance. Mono note priority,
legato/retrigger and glide may also differ. Inventory actual parameter descriptors
and modulation destinations before choosing mappings.

## Translation architecture

Keep conversion offline in host tooling, e.g. `tools/sh_style/`; no JSON parsing,
allocation, file IO or fitting in the realtime render loop. Reuse Part 1 schema
validation; do not import its live MIDI dependencies to convert files.

Pipeline:
1. Validate immutable source snapshot/version/ranges/provenance and missing fields.
2. Decode canonical values from Part 1; retain unknowns and source references.
3. Apply versioned mapping policy plus explicitly recorded per-patch overrides.
4. Emit complete ordinary ANALOG preset recipes and a loss/approximation report.
5. Generate an additive bank from checked-in curated manifests reproducibly.

Outputs under a proposed `assets/sh-style/` include source references/hashes,
translator version, mapping policy, recipe IDs/names, output hashes, reports and
original patch provenance. Full computer-local capture metadata stays out of public
assets. No runtime dependency on the Mac, hardware, MIDI listener or a network.

CLI example (final names documented by implementation):

```sh
python -m tools.sh_style translate capture.json --policy basics-v1 --output out/
python -m tools.sh_style build-bank assets/sh-style/manifest.json
```

Identical source + policy + overrides must produce identical recipe/report content;
wall-clock build time must not break reproducibility. Unknown schema versions,
invalid CC ranges and malformed input fail with actionable messages. Missing
parameters can use a documented default only if prominently reported as assumed;
never silently fill from a different patch. Permit strict mode to reject assumptions.

## Approximation policy: explicit, bounded, useful

Every source field receives one disposition: mapped, approximated, unsupported,
performance-only/ignored, or missing/defaulted. Each output field records source
parameter(s), transform, rounding/clamp and confidence. Separate capture confidence
from translation fidelity. Estimated values/ranges are design heuristics, not
statistical confidence intervals or measured Roland specifications.

| Source concept | Round-one treatment |
| --- | --- |
| Saw/pulse/sub/noise levels | Select best existing SAW/SQR/PWM/SUB mode using a documented deterministic rule; map attainable mix/noise and report dropped components |
| Pulse width and PWM | Use existing shape/modulation only where routing supports it; preserve manual versus LFO/env distinction when known, otherwise mark assumed/unsupported |
| VCF cutoff/resonance | Monotonic bounded normalized mapping to ANALOG descriptors; declare response-law and filter-topology mismatch |
| Filter envelope depth/key follow | Map supported destinations/sign/range; report clipped or unsupported routing, never silently reinterpret |
| ADSR | Convert to target envelope domain with documented monotonic heuristic; preserve zero/maximum and sustain semantics; do not label ms without justified units |
| VCA gate/envelope | Use corresponding host behavior if representable, otherwise explicit approximation |
| LFO waveform/rate/destination | Map supported waveform/routing; rate may be normalized or estimated; no fabricated Hz |
| Range/transposition/glide/trigger | Preserve playable pitch where known, use existing mono behavior; report priority/legato/glide mismatches |
| System/poly/unison/performance state | Out of scope unless already representable without new behavior; report omissions separately |

Implementer freezes a concrete policy table/formula after inspecting real inputs
and target descriptors. For an unknown law, a simple normalized linear mapping is
acceptable as a labeled initial heuristic. Where useful, store an estimated target
range plus a deterministic chosen value (e.g. midpoint) and override rationale.
Do not generate arbitrary random values within uncertainty ranges.

Mixed-wave patches cannot always be represented; choose one single-track ANALOG
approximation, not extra tracks or a new oscillator algorithm. Two oscillator
mix controls must not be called independent SH mixer gains. Same envelope used for
multiple destinations must be handled consistently with target architecture.

## Bank and user experience

Minimum shipped set: two named translations backed by two real captured patches;
2-1 is a required member. Name additional optional exploratory variants distinctly
and link them to their one source; variants do not satisfy the two-source minimum.
Names must fit actual firmware limits with deterministic collision handling.

Expose **SH-STYLE** through normal preset browsing and browser selected-track
loading, following the existing Signature collection pattern. Each loads an
ordinary ANALOG sound editable with normal controls, sequencer, locks, MIDI and
save operations. Preserve unrelated selected-track pattern/mixer state according
to existing sound-loading semantics; document any intentional factory level.
Do not automatically replace the current demo or erase autosave/user slots.

Append new factory presets after existing ANALOG presets including BG CARR SAW;
never renumber current engine/preset/program identities. Consolidate/extend the
existing wrapper generator as needed so two scripts cannot overwrite each other's
bank. Preserve copyright/license notices and pinned dependency source unchanged.
Validate preset-count widths, MIDI program mapping, editor lists and name lengths.
Prefer no storage change: translations are ordinary existing parameter values.
If a format extension proves necessary, stop that expansion and document why;
do not change frozen defaults or silently reinterpret saves to force it through.

## Implementation sequence and verification

1. Verify dependency delivery and inspect schema/target descriptors; freeze mapping
   table and source inventory. Show attainable versus dropped features per patch.
2. Implement pure deterministic translator, schema validation and useful reports;
   test endpoints, clamping, missing/unknown selectors and mixed-wave fallback.
3. Generate additive factory bank + normal panel/browser access, no new DSP.
4. Verify existing behavior and deliver commands, exact commit, manifest and limits.

Tests/evidence:
- same input/policy produces byte-identical recipes; source snapshots unchanged;
- all parameters in legal bounds, no unknown parameter silently discarded;
- at least two real inputs traced to outputs and all omissions/assumptions surfaced;
- old factory and BG CARR SAW IDs/recipes unchanged; no golden rewrites to hide drift;
- root web build and appropriate full host suite, native/WASM rendering parity;
- generated signal finite/bounded/nonzero, note-off/STOP/panic and mono retrigger;
- normal selection on all six tracks, same eight shared voice budget, concurrent
  vocoder/other-engine regression and no duplicate rendering;
- save/project/autosave/editor roundtrips preserve new sounds and existing content;
  existing B6S1 3840-byte FULL behavior remains atomic;
- actual Chromium selects each bank sound, edits, plays, saves/reloads and checks
  desktop/mobile access with no errors; no human listening claim required.

Technical checks may render temporary signals for automated measurements. They do
not require recording/listening to the SH-01A or producing a listening gallery.
Report browser/host CPU/memory changes only as measured; physical FM-1 feasibility
and acoustic similarity are unverified, not blockers disguised as passed tests.
No device compile/flash, deploy, merge or release under this implementation brief.

## Acceptance and handoff

Deliver same PR with translator, versioned policy, at least two provenance-backed
patches, per-patch approximation reports, additive accessible bank, tests/results
and short user instructions. Label the bank approximate, not Roland-equivalent.
This completes the basics round without acoustic calibration. Future audio-based
calibration or dedicated SH-style DSP is separate work, not an unfinished MVP gate.

### Implementation-agent brief

Continue `plan/sh-style-analog-bank` only after the Part 1 delivery gate above.
Read repository instructions, this plan and the delivered capture contract. Keep
ANALOG engine identity/DSP and existing presets intact. Implement translator and
SH-STYLE collection in reviewable commits, prove persistence/browser behavior,
report every approximation, and push the same PR. Do not start new hardware capture,
rewrite the demo, deploy, merge, flash, or expand into a new synth engine.

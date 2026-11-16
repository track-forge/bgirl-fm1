# BGIRL FM-1 — Signature Electro Bank build spec

Date: 2026-11-16. Status: **implemented on the existing PR; final verification in progress; Doug listening pending**.

## 1. Purpose and authority

The bank is a core part of BGIRL's instrument identity: a deliberately designed,
small electro palette with useful performance ranges, coherent levels and musical
defaults. It is not a collection of faithful Ableton preset ports. Learn from
reference synthesis, envelopes, layering and arrangement roles; author original
BGIRL recipes. No wavetable synthesis or purchased preset/sample redistribution.

This document is the implementation contract. The PR starts with documentation
only; the implementer must add implementation and evidence to **this same branch
and PR**, not merge the stub or create a replacement PR. Exact patch bytes and
final names are provisional until rendered and auditioned. Do not claim musical
approval from meter activity or automated tests.

Planning baseline: `5b2587014376e26d64d97b7082d67be65fdabef3` on main. Its tree
matches the research baseline `a1bfcb64e14f151c1222206bdebd03574819c755` (empty
Git diff). Recheck drift when implementing. Active source is **felucca/**, not the
historical app/base prototype at `4cc1e11`.

Read current `AGENTS.md`, `README.md`, `docs/BGIRL_BASELINE_PLAN.md`,
`docs/UPSTREAM.md`, `docs/BUILD_STATUS.md`, `docs/VOCODER_CONTRACTS.md` and
`docs/VOCAL_BANK_CONTRACT.md`. Follow their current build and persistence rules.
Do not advance engine/BSP pins, build/flash devices, merge, publish releases/tags
or deploy a preview as part of this handoff. Local host/browser work is in scope.

## 2. Evidence and supplementary references

The local research notebook is:
`/home/dev/.openclaw/workspace/obsidian-agent-tasks/Creative/Music/BGIRL FM-1 - Electro Patch Bank Spec.md`
(notebook commit `04d13c8`). Related notes are `Studio Brootle Mega Lab.md` and
`Studio Brootle SB61 - Electro Bundle for Ableton.md` in the same directory.
Private receipts: `/home/dev/.openclaw/workspace/tmp/bgirl-electro-patch-audit/`.
These local resources supplement this self-contained build contract; access to
purchased source files or the running DAW is not required to begin implementation.

Audit: 14 racks observed live, 14 preset files / 15 Operators inspected offline,
96 MIDI notes read. Live 12.4.6, 50 tracks, 907 scenes. Nested Operator/FX values,
mapping graphs and algorithm/waveform labels were not live-verified. Set title,
save state and listening remain unknown. File-derived values are recipes, not
current live truth; numeric algorithm, fine tuning and envelope domains are not
portable by identity. Missing details block reproduction claims, not our design.
No need to manipulate the existing Ableton set for this work.

## 3. Required v1 roster

Logical IDs below are stable design identities, **not array indexes**. Final
short display names must fit the actual UI. Existing kit and liked demo sounds
remain accessible under their existing identities.

| ID / working name | Musical contract | First implementation route | Controls to expose through existing UI |
| --- | --- | --- | --- |
| sub-clean / SUB CLEAN | Stable mono fundamental; punchy articulated or held bass | FM6 single carrier or ANALOG sine | level, release, restrained tone; no default pitch sweep |
| sub-drive / SUB DRIVE | Same stable foundation with audible upper harmonics | FM6 carrier/modulator or clean core + existing drive | brightness, decay/release, drive, level |
| bd-round / KICK | Rounded body, short downward pitch transient, controllable onset | DRUM voice via dedicated-voice selection | tune, decay, attack/click if supported, drive |
| sd-snap / SNARE | Crisp noise attack with body, distinct from clap | DRUM snare | tone/body, decay, snap if supported, level |
| cp-layer / CLAP | Single-trigger clap with supporting snare/body layer | DRUM clap + snare lanes behind one trigger | layer balance, decay, tone, level |
| hh-closed / CLOSED HAT | Short, articulate high-frequency timekeeper | DRUM closed hat | color/tune, decay, level, accent |
| hh-open / OPEN HAT | Complementary open tail, defined choke behavior | DRUM open hat | color/tune, decay, level, accent |
| stab-iron / IRON STAB | Inharmonic metallic punctuation with useful brightness sweep | FM6 serial pair plus existing drive | brightness, release, drive, delay send |
| chord-electro / ELECTRO CHORD | One-root minor chord hit, controlled tail and voice use | ANALOG + existing chord mode; FM6 alternative | brightness/filter, release, drive, space |
| chirp / CHIRP | Short pitch accent, useful without ambience | FM6 carrier + pitch envelope | sweep depth, decay, level, delay send |

Ten required identities. Optional second hollow stab and siren are excluded from
v1 completion criteria; add only if listening establishes a distinct role. Do not
fill slots with near-duplicates. Existing 808-style kit is retained; current demo
TIGHT and DRUM model 80 are distinct, so preserve both rather than assuming which
one the user meant. Preserve SQR BASS, BELL, CLOUD PAD and vocal demo access.

The control column states desired musical affordances, not a promise that every
engine already supports four named macros. Use existing controls where possible;
record unsupported controls and the smallest necessary app-side extension. Do not
invent ineffective knobs or redesign the entire parameter UI.

## 4. Playable behavior contracts

### Bass

SUB CLEAN must have one stable fundamental, no detune/chorus/noise/pitch modulation
or default reverb/delay. Target sounding MIDI 28–52, especially 32–43; explicitly
test MIDI 24 as an extended low case. MIDI numbers, not octave-name conventions,
are authoritative. Default mono retrigger, with existing legato available if
supported. Start with 3–8 ms attack / 40–150 ms release as audition targets, not
literal DX rate bytes. SUB DRIVE adds harmonics without a moving second fundamental
or large loudness jump. Test short/held/overlapping notes and rapid retriggers.
Keep phase/retrigger choice explicit; deterministic phase is not inherently click-free.

### Standalone percussion (required, not merely an audition workaround)

Selecting KICK makes the track a kick instrument. **Every playable MIDI note
triggers that selected identity**, not a different GM lane. Default fixed drum
pitch across keys; TUNE changes pitch. Chromatic key tracking is optional and off
by default, not required for v1. Clamp tune ranges initially to kick ±5 semitones,
snare/clap ±3, hats ±5 around curated centers; document actual mapping.

All five drum presets use decay-defined one-shots: note-off must not truncate the
normal tail, while STOP/panic and sound/project replacement must clear safely.
Velocity controls level with useful ghost notes and accents, not identity. A
new hit retriggers its selected lane; overlapping tails/stealing must be documented.
CLAP takes **one note-on**, firing its internal clap/body layers together with a
controllable balance; two sequenced notes do not meet this contract.

Retain full-kit GM behavior unchanged. A kit restricted to note 36/38/42 is an
intermediate prototype, not completed standalone support. Prefer an app-level
persisted voice-selection/trigger mode over a new engine or dependency fork.
Verify both live keyboard/MIDI and sequencer trigger paths; do not duplicate
musical logic in the UI or lose note-off ownership after track switches.

For v1, standalone CLOSED/OPEN HAT tracks share an explicit **opt-in choke group**;
the curated hat presets opt into the same group. A closed hit terminates earlier
open tails in that group across tracks. Open hits do not kill closed hits. Same-
frame open+closed resolves to closed regardless of track processing order. Other
tracks, nonmember hats and full-kit internal choke behavior remain unaffected.
No blanket cross-track choke. Persist membership and clear runtime relationships
on load/panic. This is a proposed product choice to implement, not existing engine
capability; document a blocker instead of silently dropping cross-track support.

Starting decay targets: kick 80–450 ms, snare 70–250 ms, clap 100–350 ms,
closed hat 30–100 ms, open hat 120–450 ms. Calibrate actual DSP envelopes.

### Stab, chord and chirp

Stab: useful MIDI 48–72 and 50/125/250 ms gates; short controlled release. Start
with a two-op FM core, not a copied algorithm index. FM6's existing brightness
control is not a resonant post-FM filter. Audition the reduced voice first. If the
missing filter defeats the musical role, document and implement only a bounded
app-side extension compatible with pinned dependencies; broad DSP redesign is out
of scope. Record its cost and output behavior separately.

Chord: one input root produces at most four tones, initially root/minor third/
fifth/lower octave, subject to verifying existing chord-mode voicing. Avoid
accidental chord expansion of already-polyphonic demo input. Document actual voice
count, input stacking, release and stealing. Chirp: clearly defined sweep direction,
start/end pitch and repeatable 60–250 ms envelope; initially no more than 24
semitones of sweep. Do not require long echo to make either voice useful.

## 5. Factory delivery, identity and compatibility

- Provide a discoverable factory Signature Electro collection using normal sound
  selection/loading; no manual user-slot import or hidden debugging command.
- Store original recipes and a versioned manifest in the repo: logical ID, display
  name, engine, parameter data, behavior mode, role, performance ranges, reference
  rationale and expected resource cost. Choose the smallest existing data mechanism.
- Append rather than reorder legacy engine/preset/bank indexes. Preserve old names
  and meanings; never overwrite the 32 user-preset slots or saved projects.
- Preserve `TRK_DEF` (legacy migration uses it). Keep fresh startup, restored
  autosave, Blank electro template, explicit demo loads and legacy imports separate.
- Existing fresh vocoder demo remains intact in v1. Add an explicit Signature
  Electro template and demo using normal project operations with replacement
  confirmation. Template loads stopped, empty sequences; demo loads stopped with
  editable patterns. Fresh users must be able to discover the bank/demo readily.
- New behavior fields need explicit old-data defaults: legacy DRUM remains kit
  mode with existing choke behavior. Reuse compatible extension mechanisms only
  after checking reserved fields; if a version change is essential, document it,
  preserve readers and define safe rejection by older firmware. No silent byte reuse.
- Round-trip factory selections and edited sounds through project, user preset,
  autosave and FM6 OWN paths as applicable. Preserve vocal-bank fingerprints,
  mismatch refusal, routing and sound-preset clearing of project carrier references.

## 6. Musical arrangements and resource bounds

Use two small loadable/editable demos so four tracks need not pretend to show ten
sounds simultaneously: (A) full retained kit + clean/driven bass + stab + chord or
chirp; (B) standalone kick + clap/snare + hat + bass. Include a short separate
closed/open cross-track choke audition. Keep the existing vocal demo available as
a coexistence regression, not overwritten by these examples.

Baseline has four tracks and eight shared voices; a four-note chord + bass +
kick/snare/hat already reaches eight before release overlap or a layered clap.
One six-operator FM note is one voice, not six, but its render cost differs.
Document actual voice allocation, stealing priority and tails for each preset and
stress arrangement. Protect intelligibility of bass/drum transients; no stuck
voices, runaway feedback, nonfinite output or unexplained clipping at defaults.
Use conservative gain staging; test mono summing and bass/kick coexistence.
No allocations, logging, blocking or file I/O in rendering. Hardware headroom,
RAM fit, deadlines and xruns remain unmeasured; browser/host timing is not target proof.

## 7. Implementation sequence on this PR

1. **Baseline and architecture map.** Read current instructions; initialize pinned
   submodules; build `./build.sh --web`, run `./tests/run_tests.sh`. Record exact
   revisions/tools. Inspect `felucca/firmware/src/eng_drum.c`, `eng_fm6.c`,
   `engines.c`, `project.c`, `upreset.c`, `up_fm6.c`, `main.c`, `category.c`, `drum_voice.c` and note-dispatch
   call sites. Inspect pinned engine source and normal factory selection before
   deciding storage and dispatch. Do not use historical app/ instructions.
2. **Bank foundation + bass vertical slice.** Add manifest/data/loading through
   existing machinery; implement and render clean/driven subs; prove one edited
   sound and project round-trip before adding the whole roster.
3. **Standalone percussion.** Implement shared keyboard/MIDI/sequencer selection
   semantics, single-trigger layered clap, explicit choke group, old-save defaults
   and persistence. Prove legacy kit unchanged before tuning the five voices.
4. **Character voices + curation.** Implement stab/chord/chirp, render dry and
   restrained-wet variants; calibrate levels/ranges. Scope filter work only from
   an identified musical deficiency. Keep exact engine decisions in the manifest.
5. **Factory integration and demos.** Add collection/template/demo discovery,
   replacement confirmation and reproducible short audition patterns. Preserve
   existing startup/autosave/vocoder/UI behavior and inherited sequencing/MIDI.
6. **Verification and listening handoff.** Complete the matrix below, update README
   and `docs/SIGNATURE_ELECTRO_BANK_VERIFICATION.md`, update this PR description
   and push reviewable commits. Keep draft while required gates are incomplete.
   Doug's taste review remains explicit; do not self-declare human approval.

## 8. Required acceptance evidence

| Gate | Required proof |
| --- | --- |
| Build/regression | Exact commands and source SHA; web build + root test suite; affected upstream tests; `git diff --check` |
| Bank | Ten distinct required identities load normally; manifest matches shipped data; no dependency on private assets |
| Drum input | Low/middle/high MIDI notes all trigger selected drum via keyboard/MIDI/sequence; legacy kit still maps notes |
| Articulation | Velocity 32/64/96/127, short/held gates, retrigger rolls, tune/decay extremes; clap uses one trigger |
| Choke | Cross-track grouped closed→open behavior, same-frame ordering, nonmember isolation, load/panic cleanup |
| Bass/character | Sustained 8-second sub notes, target register, overlap/retrigger; sweep contours; chord count/stealing documented |
| Persistence | Edited sound/project/autosave round-trips; legacy fixtures; factory/bank/demo identities; user slots not overwritten |
| Audio robustness | Finite/bounded dry/wet mix, stop/panic tails end, maximum relevant overlap, shared effects/vocoder regression |
| Browser | Actual normal selection, controls, template/demo confirmation/cancel and reload tested; desktop/mobile usable |
| Listening | Labeled dry/wet recordings and context demos, fixed MIDI and level context; Doug's verdict pending/approved/revise per family |

Use focused behavioral tests, not assertions that merely mirror preset constants.
Do not rewrite upstream golden outputs to hide accidental changes. Record expected
new outputs separately; explain deliberate changed behavior. For each audition,
retain recipe/version, MIDI, gain/effects context, render command and artifact path.
Loudness-match comparisons while retaining unnormalized originals. Test contexts
at 90/120/140 BPM. Provide a playable browser recipe and a compact comparison set.

No new Live mutations or hardware actions are necessary. If listening or an
essential design extension blocks completion, report that exact gap in this PR;
continue independent implementation and never label a kit-note workaround complete.


## 9. Implementation decisions / progress

- Baseline root web build and full host suite passed at8e3ad05; main had no drift.
- Original FM6 OWN recipes appended at presets8..12; DRUM presets4..8.
  New PRESETS category SIGNATURE contains exactly ten entries. Legacy IDs and
  F1..F8/B1..B32 SLOT numbering stay fixed. MIDI PC8..39 retains the FM6 bank;
  new Signature FM6 presets append at PC40..44. DRUM PC0..8 keeps its kits;
  new selector modes append at9..15.
- Standalone KIT12..18 is the smallest persisted behavior mechanism. Grouped
  hats are15/16 (group1);17/18 FREE opts out. No new common parameter IDs.
  FUND guards projects containing these selections or DRUM KIT motion; it
  carries FUNC fingerprints too. Presets use7/8 (notes/grid). Old readers stay.
- CLAP mixes its clap and snare states inside one shared allocation; existing
  LANES SNARE level is the body blend, factory32/127. No dependency fork.
- FM6 chord is the permitted alternative: one live root → MIN/+OCT (four tones),
  explicit four-tone sequencer steps. No sequence double expansion. FM6 caps
  at six voices per track; global eight-voice stealing is inherited.
- No post-FM resonant filter added without an identified listening deficiency.
  Existing MLVL is modulation brightness, not a filter. OP ENV owns amplitude.
- Explicit browser template, kit-context demo, standalone demo and hat exercise
  use normal project reset/load paths, confirmation and stopped transport.
  Existing vocal startup/demo, autosave precedence, kits and user slots retained.
- Technical findings, exact final commands/revision and listening artifacts live
  in SIGNATURE_ELECTRO_BANK_VERIFICATION.md. All sound names/levels remain
  reviewable; **Doug listening verdict pending for every family**.

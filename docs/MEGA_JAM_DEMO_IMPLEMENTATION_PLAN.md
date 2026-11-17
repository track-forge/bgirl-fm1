# Mega Lab jam → four-bar default electro demo

Date: 2026-11-17. Status: **plan only; implementation and audition pending**.
Base: `c51c53885fd18c1e84be676d1644143803bc2e0b` (main, including PR27).
Branch: `plan/mega-jam-demo`. Continue this branch and its draft PR; do not open a replacement PR.

## Goal and authorization

Doug approved adapting his Mega Lab jam into the default BGIRL electro demo,
retaining and refining the vocoder. Deliver an editable, groovy four-bar loop,
not four copies of a one-bar pattern. This stub commits only this plan.
The implementation handoff targets a tested browser preview at:
https://openclaw-fedora.tail549b77.ts.net:8448/

No implementation or service change is performed by this planning PR. The next
agent should implement on this PR, validate, push, and update that existing
private tailnet preview. No merge, public Pages/Funnel deployment, release,
device build/flash, dependency pin changes, or Ableton edits are in scope.

## Evidence and reference identity

Read-only AbletonMCP inspection of Live12.4.6: 128 BPM,4/4,50 tracks,914 scenes.
Global transport was **stopped**, song position103.183286 beats. Retained launch
indices and clip is_playing flags are not evidence of audible playback.
Scene slot68 (human scene69) retained these clips; Doug accepted the proposed
adaptation after being given this combination. No audio listening or external
SH-01A patch dump was performed. Set root/scale UI says D Minor; that is not
proof of the heard key, rack transposition or external synth tuning.

| Live zero-based track/slot | Reference | Loop |
| --- | --- | --- |
| 16/68 | SB45 Noise Drone Pad, `45 \| Drone Pad (fav) 1` | 16 beats /4bars |
| 35/68 | 808 Core Kit, `ARTICLE 4 \| Bouncy + simple` | 8 beats /2bars |
| 37/68 | External SH-01A, `38 SH-01a 11` | 8 beats /2bars |
| 49/68 | recorder14 | Muted; not a musical source to import |

Do not require the Mac to still have this set open. The bounded transcription
below is enough to implement. If inspecting Live again, use ableton-live skill,
read only, and re-identify names/slots first; never launch, stop, save, select,
replace clips or change hardware patches incidentally.

### Pad: pitch, onset, duration, velocity (beats, raw MIDI numbers)

```
50  0.000000  7.024036  34
60  6.951958  1.301043  33
64  8.203610  7.279214  31
```

D→C→E movement, with slight overlap at transitions. Approximate onsets near
steps0,28,33 and use ties/gates; document deviations rather than claiming a
lossless MIDI import. The low velocities interact with the Ableton rack;
translate loudness by ear, not by blindly matching raw values across engines.
Observed pad macros: unison77%, drive15%, crunch64%, BP413Hz/mix37%, LFOmix4.9%,
reverb2.30s/mix34%. These are rack macro displays, not a full synth recipe.
Use an existing suitable engine to suggest its textured, evolving character;
exact reproduction of the rack is not required.

### SH-01A riff: pitch, onset, duration, velocity

```
45 0.00 .250000 40
45 0.25 .106716 87
55 1.50 .250000 84
55 1.75 .139379 61
57 2.00 .250000 45
57 2.25 .130964 73
45 4.00 .249977 74
55 4.25 .250000 84
55 4.50 .012798 41
57 4.50 .250000 62
57 4.75 .154490 53
45 5.50 .250000 19
45 5.75 .008936 54
55 5.75 .250000 60
55 6.00 .057554 48
53 6.25 .235285 65
55 6.75 .250000 35
55 7.00 .218600 112
```

Preserve A/G/F pitch material and rhythmic identity first; pitches fit without
mandatory global transposition. Use a suitable existing synth (ANALOG is a
starting candidate); external SH-01A timbre/settings are unknown. Two onsets
have simultaneous notes; for a mono interpretation prefer the longer main note
and explicitly document the omitted grace note. Step velocities are shared,
not arbitrary per-note velocities; gates/ties/locks must use existing machinery.
Retain meaningful accent contrast while adapting near-zero durations sensibly.

### Drums: raw MIDI pitch → onset beats in two-bar loop

```
36 kick:   0 .5 1.5 2 2.5 3.5 4 4.5 5.5 6.5 7.75
37 rim:    7.25 7.5 7.75
38 snare:  1 3 5 7
39 clap:   1 3 5 7
42 closed: 0 .25 .5 1.5 2 3.5 4 4.25 4.5 5.5 6.5 7.25 7.5 7.75
46 open:   1 1.25 2.5
49 cymbal: 4.5 5.5
```

Kick velocities in listed order:120,100,120,104,120,104,120,100,120,120,100.
Snare/clap120; rim100; hats/cymbal104. Most gates .25beat; open at2.5=.5beat;
cymbal at4.5=.75beat and5.5=.5beat. Existing DRUM lane notes are
36/38/39/42/46/45/37/56: pitch49 needs a deliberate replacement or omission,
not a raw numeric import that triggers an unintended sound. Use DRUM grid
hit/accent masks for simultaneous kit hits. Prefer the BG signature kit sounds
where they fit; do not split all drums into separate tracks. Explain any shared
velocity/accent approximation. Preserve snare+clap layering or use the existing
combined-body sound deliberately; avoid accidental double layering.

## Existing implementation constraints

- `felucca/firmware/src/core.h`: six full tracks,64steps each, eight shared voices.
  At1/16rate,64steps is4bars in4/4, not a whole-song limit; chain can go longer.
- `main.c`: `bgirl_demo_steps` / `bgirl_demo_vocoder` construct editable normal sequencer data.
  Current drums/bass repeat shorter patterns; carrier and G6 chops loop32steps.
- Current carrier is TRIO/FAT BASS (engine6/preset0), not old GRAIN/saw defaults.
  Default vocal route T3→T4, MONoff,12bands; clarity supports24bands and log REL.
- Read AGENTS.md, README.md, UPSTREAM.md, BUILD_STATUS.md, six-track verification,
  VOCODER_CLARITY_CONTRACT.md and VOCODER_CLARITY_VERIFICATION.md. Historical docs
  may describe older defaults; source at the pinned base wins.
- Keep normal sequencer/transport, step editing, presets, motion, MIDI and saves.
  No alternate player, new engine, new wire format or independent voice pool.
- Project capacity remains3840bytes with atomic FULL refusal. Measure actual
  packed demo size with margin; four-bar content must roundtrip without loss.
- Do not change frozen project_defaults.h or inherited engine/preset IDs.

## Arrangement and sound-design brief

| BGIRL track | Intended role |
| --- | --- |
| T1 | Two-bar reference drum groove expanded to a four-bar arrangement |
| T2 | SH-01A riff interpretation; preserve accents and rhythmic identity |
| T3 | TRIO/FAT BASS carrier, sustained enough for intelligible speech |
| T4 | Existing public G6 vocal hook + sparse chopped answers |
| T5 | Four-bar textured pad movement from the three-note reference |
| T6 | Optional sparse punctuation, or intentionally empty for user jamming |

Start at128BPM. Use64step phrases where the composition needs4bars; shorter
repeating tracks are fine if the overall arrangement genuinely develops.
Suggested shape (taste decisions to audition, not a required extra DSP feature):

1. Establish drum/riff identity with a recognizable vocal hook.
2. Vocal answer and a restrained existing drum turnaround.
3. Repeat the hook against the changed pad pitch; vary one accent/texture.
4. Short fill/answer and a breath before the loop restarts.

Do not pile every sound on each downbeat. Preserve the groove's negative space.
Use existing voice accounting to avoid steals, especially snare+clap+hat with
pad/riff/carrier/vocal tails. Reduce density/release or combine parts before
considering architectural changes. Never infer eight voices means eight tracks.
Do not make unnecessary four-note pad chords.

### Vocoder refinement

Keep T3→T4 routing and existing authorized G6 source; no new speech generation,
private/commercial samples, audio import from Doug's set or sample-bank rebuild.
Audition24bands with TRIO, MONoff, log release about100–150ms, wet about92%
(small original-speech blend). These are starting values, not listening approval.
Use whole-phrase/recognizable hook material and fewer chopped replies than the
current constant chop groove; time the phrase into available space. Confirm the
selected G6 view/chop mapping by identity, not brittle full-bank indices.
No36band/DSP redesign, carrier-engine modifications or new effects in this task.
Keep existing8/12/24 support and isolated clarity auditions working.

## Default/demo behavior

Fresh initialization and explicit **Load electro demo** must load the same new
arrangement, stopped. Existing autosave restores after initialization and wins;
never replace saved music automatically or erase slots. Confirm-before-replace
and cancel semantics remain. Blank electro template stays blank. Retain existing
standalone signature/six-track/clarity audition behavior unless a necessary
shared-default dependency is documented and tested. Update stale role labels,
README demo recipe and relevant assertions; do not rewrite unrelated goldens.

## Implementation sequence and acceptance

1. Inspect branch/status and current main; stay on this PR. Establish browser
   build/full host baseline using root wrappers; initialize pinned submodules
   as needed without advancing pins. Record exact revisions and tool commands.
2. Implement readable named pattern data/helpers and the chosen mapping using
   existing engine/load/sequence machinery. Record pitch/time/velocity/timbre
   deviations and public sample provenance. Keep edits in logical commits.
3. Test fresh vs explicit loader equality,128BPM/meter/lengths, four-bar variation,
   all roles/routing and deterministic repeat. Exercise play/stop/restart, tails,
   panic, voice steals/overload and bounded output over several loops. Check
   packed size, project save/reload, autosave priority, blank/cancel preservation.
4. Run `./build.sh --web`, `./tests/run_tests.sh` and affected real Chromium
   suites. Cover six tracks/selection, vocal identity, vocoder enabled/disabled,
   worker/native/WASM consistency and unaffected sound goldens as applicable.
   Intentional arrangement render changes get separate documented evidence,
   not silent replacement of unrelated golden expectations.
5. Actual browser: user-gesture audio, new default/demo sound, normal edits,
   save/reload, STOP, desktop/mobile visual checks and zero page errors. Render
   a short complete-loop WAV if useful for review; no mandatory gallery rebuild.
6. Push tested commits to this same draft PR. Update plan status and concise
   verification docs/PR body with results, exact head, mapping deviations,
   known limitations and listening recipe. Verify hosted CI at the final head.
7. Deploy the exact tested head privately as below; verify endpoint identity and
   interaction. Report technical pass separately from Doug's musical approval
   and untested hardware RAM/deadlines/physical MIDI. Do not merge the PR.

## Existing tailnet preview delivery (implementation agent)

Target: **https://openclaw-fedora.tail549b77.ts.net:8448/**.
Use the existing service, not a new public endpoint or an unrelated port.

Last recorded topology (re-inspect; these are discovery hints, not live proof):
Tailscale HTTPS8448 → loopback8786; user `bgirl-signature-local.service` has a
runtime `clarity-preview.conf` drop-in under
`/run/user/1001/systemd/user/bgirl-signature-local.service.d/`.
It served an immutable `local/previews/28fa3c49d88edc5fff5e60f8aafa44d79fea9173`
artifact from the clarity worktree. HTTP8771 redirect and other Serve/Funnel
entries must remain unchanged. Do not assume the service still uses that path.

- Inspect `systemctl --user cat/show` and `tailscale serve status --json`; record
  effective service/drop-ins and current artifact identity before modification.
- Build at exact pushed PR head, freeze a dedicated immutable preview directory
  (including needed runtime assets), and record build.json/WASM SHA256. Preserve
  prior artifact and service configuration for rollback; do not serve a mutable
  development build that later edits can silently change.
- Update only this service's necessary runtime launch/path configuration and
  restart through its existing mechanism. Preserve loopback binding and existing
  HTTPS mapping; do not create Funnel or change unrelated tailnet services.
- Compare Serve configuration before/after. Read served build.json and hash
  downloaded WASM; prove they match tested commit/artifacts. Test actual HTTPS
  AudioWorklet/audio and demo controls in Chromium, not only HTTP200.
- Old browser autosave may restore old music: reviewer recipe is refresh →
  Switch on → Load electro demo → confirm replacement → PLAY. Never clear
  browser storage to force a demo update without the user's explicit request.
- On failed rollout restore the saved effective service config/artifact, verify
  previous preview, and report the blocker. Do not claim deployment from a local
  build alone. Deliver exact URL/head, recipe and remaining listening questions.

# SH-01A patch gathering — basics round implementation plan

Status: implementation staged on this same PR; real patch captures pending.
See tools/sh01a/README.md and SH01A_PATCH_GATHERING_VERIFICATION.md.
Issue: https://github.com/track-forge/bgirl-fm1/issues/30
Next: https://github.com/track-forge/bgirl-fm1/issues/31 (SH-STYLE ANALOG translation; blocked on this delivery).
Planning baseline: main `c51c53885fd18c1e84be676d1644143803bc2e0b`.

## Goal and boundaries

Capture Doug's saved SH-01A patches as portable, evidence-bearing JSON and track
subsequent edits using received MIDI CC. Start with saved bank 2, patch 1. Supply
real source material for the next issue. This is the **basics round**: no reference
audio recording, audio analysis, listening, or acoustic calibration requirement.
Unknown physical units may remain unknown or explicitly approximate; never invent
captured MIDI values. No firmware changes belong in this issue.

Read-only means opening MIDI INPUT only: no CC writes, Program Changes, SysEx
requests, patch writes, output routing or feedback loops. User manually operates
the synth. Before switching away, ensure the desired patch is saved; do not lose
unsaved edits to force a baseline. Do not automate Ableton or alter its routing.
No merge, deployment, device build/flash, or release is included.

## Evidence and first experiment

The proposal reports CCnd enabled and patch 2-1 saved. Treat this as user-reported
setup, not a hardware result. Check official documentation before coding mappings:

- https://static.roland.com/assets/media/pdf/SH-01A_MIDI_Imple_Chart_eng01_W.pdf
- https://static.roland.com/assets/media/pdf/SH-01A_eng04_W.pdf

Record source document/version/page for each mapping. Verify CCnd patch-load CC
output, transmit versus receive columns, and supported controls. Do not assume
SysEx, a dump-request command, all CCs transmitting, discrete selector thresholds,
or Program Change/bank ordering based on other Roland devices.

Inspect the available Mac/Python/CoreMIDI environment first. Prefer Python with
mido + python-rtmidi, installed in an isolated environment; keep transport replaceable
and decoding/replay usable without MIDI dependencies. Document prerequisites and
exact install/run commands. No mandatory Ableton, MCP, HTTP service, or cloud.

Experiment, with user present for controls:
1. Enumerate input ports; select exact port and MIDI channel explicitly.
2. Start timestamped raw capture before manually loading another saved patch then 2-1.
3. Repeat loads, move cutoff and several sliders/switches, and record observed CCs,
   timing, ordering, channel and optional Program Change/bank messages.
4. Test concurrent Ableton access without changing routing. If unavailable, report
   the blocker and a manual capture-only session procedure; do not close or rewire
   the DAW automatically.
5. Record any controls not transmitted. Do not send test MIDI to determine echo;
   external-input echo behavior can remain unverified and a stated limitation.

If hardware is unavailable, implement/test the utility with explicitly synthetic
streams, but mark hardware acceptance and downstream real-capture gate blocked.

## Repository layout and separation

Proposed ownership under `tools/sh01a/`, with its own README/dependency metadata:
- transport: enumerate/open input, timestamps, disconnect/reconnect;
- definitions: versioned CC table, discrete semantics, transmit/receive direction;
- decoder: pure message-to-parameter observations, retain unknown messages;
- tracker: baseline/capture lifecycle, current values, changes and confidence;
- storage/schema: versioned JSON + append-only JSONL raw events;
- CLI: monitor/status/snapshot/diff/replay, no write-to-synth commands;
- tests: deterministic synthetic and sanitized real replay fixtures.

Use one foreground monitor process per session directory. It owns live state and
atomically replaces `current.json` and `status.json` (temp file + rename). Other
commands read those files, not a second MIDI connection or a nonexistent shared
in-memory singleton. Include run/session ID, sequence number, heartbeat, timestamps
and shutdown status. Readers label an old/dead monitor stale using a documented
configurable heartbeat threshold. Prevent two monitor writers for one directory.
Raw event append and atomic snapshots are outside any firmware audio path.

Illustrative CLI (implementation may refine spelling, document actual commands):

```sh
sh01a list-ports
sh01a monitor --port 'exact input name' --channel 1 --session ./capture-session
sh01a status --session ./capture-session --json
sh01a snapshot --session ./capture-session --output ./patch-2-1.json
sh01a diff --baseline ./baseline.json --current ./patch-2-1.json
sh01a replay ./events.jsonl --output ./replayed.json
```

## Export contract v1 — owned here, consumed by Part 2

Publish a machine-readable schema and documented example. Keep these concepts:
- `schema_version`, unique capture ID, device model, capture/session time;
- source port/channel, tracker/definition version, raw-event file and SHA256;
- patch label and optional bank/program identity with `manual` or `observed`
  provenance; keep MIDI wire program separate from human bank/number;
- confidence state and reasons, baseline ID, missing expected fields, observation
  coverage and disconnect/overflow indicators;
- immutable raw CC observations (0..127), canonical parameter IDs, decoded value
  or null, decoding provenance, and observed timestamp/sequence per parameter;
- baseline and latest state, modified-since-load, and typed parameter diffs;
- unknown messages, unsupported/transmit-unknown parameters and limitations.

Canonical IDs should describe the instrument, not BGIRL: e.g. `vcf.cutoff`,
`envelope.decay`, `mixer.saw_level`. Inventory the entire documented mapping and
separate patch settings from performance controls (notes, pitch bend, sustain,
mod wheel, etc.) and system settings. These must not pollute patch completeness.
Keep raw values authoritative. Unknown selector meanings stay unknown rather than
becoming percentages. Optional physical estimates need range/unit/assumption
metadata; unknown exact units do not invalidate a captured raw value.

Archive snapshots as immutable files; monitor state is mutable. Do not publish
machine usernames/port identifiers or entire live logs automatically: curated
fixtures contain only relevant MIDI/provenance. Include license/provenance for
original patch exports; this request covers Doug's supplied patches, not copying
third-party commercial banks. Full local logs can stay ignored.

## State machine and completeness

`uninitialized` -> `capturing` -> `ready` or `partial`; disconnect/restart or evidence
of loss -> `stale`. Ready means complete for a named **verified transmitted-parameter
set**, not all internal synth state. Track partial raw observations before baseline.

Burst quiet time identifies a candidate boundary only, never proves completeness.
Derive expected-set profiles from documentation plus repeated observations and
record that evidence. Handle duplicate/out-of-order messages and interleaved
clock/note traffic. Use MIDI channel and source filtering throughout.

Patch transition attribution must follow the measured ordering. Invalidate the old
baseline on a detected change, never carry absent fields across patches. If no
reliable delimiter exists, offer an explicit user-armed capture window (e.g. monitor
interactive capture action) and label identity manual. Do not claim automatic patch
reset correctness in an unobservable case. Ignore late prior-capture data when it
can be identified; otherwise mark ambiguous capture partial and request recapture.

Undetectable dropped MIDI messages cannot be magically diagnosed. Confidence is
best effort with explicit limitations. Never mark a reconnected listener ready
without a new baseline. A net diff compares current versus baseline (moving a knob
back clears its net change); event history remains separate.

## Implementation steps and acceptance

1. Documentation/port discovery + input-only raw logger. Prove manual patch-load
   burst on the real instrument; keep evidence and transcript of actions.
2. Full parameter inventory/decoder + schema and offline replay; unknown retention.
3. Tracker and atomic monitor-reader CLI; snapshot and baseline/net diff.
4. Real captures and sanitized fixtures, documentation and Part 2 handoff.

Required tests: channel filtering, partial startup, duplicate CCs, interleaved
performance traffic, observed Program Change ordering, patch transitions/no field
carryover, missing fields, disconnect/reconnect, stale reader, malformed log/schema,
unknown CCs/selectors, deterministic replay, atomic snapshot reader and diff reverts.
Prove no MIDI output opens/sends with a transport spy. Do not use synthetic fixtures
to claim the real hardware acceptance criteria passed.

MVP evidence: discover input, capture 2-1 baseline, slider update, correct net diff,
snapshot export, honest partial/stale state and verified/manual patch boundary.
Export **at least two distinct real saved patches**, including 2-1, with each
capture's raw log hash and coverage report. User selects another saved patch; never
write new hardware patches. Two identical captures or invented variants do not
satisfy the set. Incomplete fields are allowed if explicit and Part 2 can apply its
approximation policy; absent captures are not.

## Serial handoff / definition of done

Deliver on this same PR: utility, schema v1, official mapping references, sanitized
replay fixtures, actual commands/results, two source captures or portable artifact
locations/hashes, and a concise capture inventory with missing/uncertain fields.
Record the exact source commit. Part 2 must consume this delivered schema and real
inventory, not freeze its own competing schema. If publication of a specific patch
is unresolved, keep it local and make that access requirement explicit.

Part 1 is not complete while real capture is blocked. Part 2 may read its plan but
must not implement the bank until the capture deliverable is accepted and available.
No audio files, acoustic fidelity score, listening sign-off, or calibrated units
are required for this basics-round acceptance.

### Implementation-agent brief

Continue this PR/branch `plan/sh01a-patch-gathering`; read repository AGENTS.md and
this plan, recheck main and dependencies, implement in small reviewable commits.
Remain input-only. Arrange the manual hardware capture interaction instead of
claiming a synthetic demonstration is real. Publish verification and the Part 2
handoff on the PR; do not merge, deploy, flash or start Part 2 automatically.

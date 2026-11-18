# Part 1 implementation — staged for first hardware patch switch

Issue #30 / same PR #32. **Not complete; no real saved patch captured yet.**
No implementation of Part 2. No merge, firmware/web build, preview deployment,
Ableton mutation, MIDI output, audio capture, hardware writes, or flashing.

## Observed 2026-11-18

- Mac SSH reachable; macOS Python 3.9.6. Ableton Live 12 Suite running.
- Isolated operator-local environment installed pinned mido/python-rtmidi.
- CoreMIDI input enumeration includes SH-01A. Exact input opened successfully
  alongside Live. Five-second passive probe received 239 MIDI clocks, no channel
  messages. Thus transmit channel and load burst are **not yet verified**.
- Official Roland chart/manual read, entire named control table recorded with
  direction/page/version. Raw selectors deliberately not decoded by guessing.
- 19 focused synthetic/spy tests PASS on host. Includes schema range rejection,
  deterministic replay, atomic readers, input-only backend, immutable export/hash,
  channel filter, startup/partial, duplicates, performance/clock interleave,
  PC before/after burst, manual reset, missing fields, stale/disconnect/reconnect,
  unknown controls/selectors, malformed bytes/logs, sequence rejection, net revert.
- Runtime syntax compatible with Mac Python 3.9; final staging/verification below
  is recorded on the PR after commit to identify exact source bytes.

## Deliberate staging limitations

No verified transmitted-set profile yet: state never promotes to `ready` merely
because all candidate CCs arrived. PC invalidation is conservative and recorded as
ambiguous; measured ordering will drive any refinement. Manual arm boundaries are
required. Passive clock-only evidence is not a saved patch capture. No capture
fixtures are labeled real; committed example is explicitly synthetic.

## Downstream capture inventory

| Source | Evidence | Status |
| --- | --- | --- |
| saved 2-1 | user-reported saved patch | awaiting operator load after arm |
| second distinct saved patch | operator selects slot | awaiting operator selection/load |
| synthetic fixture | hand-authored CC74=42→50 | test only; forbidden as bank source |

Two distinct real saved patches, repeated burst/profile evidence, control updates,
net diff, immutable exports/hashes and sanitized replay evidence remain required
before issue #30 completion or Part 2 bank implementation. No acoustic/listening
gate is required this round. Monitor staging is temporary and input-only, not a
service deployment. Exact local paths/commands are given privately to the operator.

## Superseding hardware results — 2026-11-18 13:24 UTC

The earlier “no real saved patch” staging status above is historical. Real captures
are now delivered privately. Doug corrected the original 2-1 request: the loaded
sound is the intended proving patch, subsequently confirmed returned to **1-1**.
Do not treat the earlier 2-1 arm label as an observed slot identity.

Source/transport/tested revision remains `27bbf5179cfc2c8d3707ce2156680485310a378d`;
this follow-up changes evidence documentation only. Host, Mac Python3.9, dedicated
Python3.9/3.12 push+PR CI and both broader browser-host runs PASS at that code head.
Pages deployment skipped. No firmware or transport code changed after testing.

### Actual input observations

- Verified receive **MIDI channel3**, alongside running Live; no routing changes.
- Four observed saved-patch loads: wire programs **1, 0, 1, 0**. Each Program Change
  preceded exactly the same **36 distinct documented patch CC candidates**. No
  conversion from wire program to human bank/slot is assumed.
- Proving patch loads at 13:09:05 and 13:23:09 UTC have identical raw parameter
  values. Second patch loads at 13:07:16 and 13:23:07 also match exactly.
- Distinct sources differ in **31/36 raw parameter values**. The second source's
  human slot remains unspecified; identify it by observed wire program1 and raw
  artifact hash, not a guessed slot or name.
- Operator moved FREQ to about middle. CC74 on channel3 changed **106→65**, with
  exactly one typed net parameter difference. Reload restored106 and all other
  captured raw parameters, yielding an empty diff against the original export.
  This is a reload/restoration comparison, **not** a claim that a slider was moved
  back to106 while preserving the same in-memory baseline across Program Change.
- All five archived bundles pass hash/length checks and offline replay of current
  parameters, frozen baseline, and net diff. JSON schema validated for exports.

### Portable private artifact inventory

Artifacts are intentionally ignored/private, not embedded in public source.
In the implementation worktree, portable bundles reside under
`local/sh01a/hardware-20261118/`; each directory has schema-v1 `patch.json` plus its
exact-prefix `events.jsonl`. Mac originals remain under the operator-local
`~/.local/share/sh01a-capture/exports/`. No redistribution license was invented.

| Bundle | Raw events SHA256 | Meaning |
| --- | --- | --- |
| `first-switch-slot-unconfirmed` | `d00b84d94a3ff5d8a3a39cf827dc4106cc1d7e0d87a898c62983815a57dd1cd2` | first wire1 saved patch |
| `proving-patch-probable-1-1-baseline` | `55e36b27a02c762fcc05af1802d076306bc94f2b001360521a0e1ed1478aca2a` | original proving baseline; location initially uncertain |
| `proving-patch-cutoff-middle` | `0424342954b853597a14c0ea9a1ad1f5f072c4b3eef3d683f946006ff77df10d` | only cutoff changed to65 |
| `second-saved-patch-wire1-repeat` | `9d605255a68f924343c2efbf330b199054923aa5f4f773e7b2c1b2fdba2f447f` | repeat wire1 source, same36values |
| `proving-patch-1-1-reloaded` | `534c9e37e89c217df9c4ece6e7738cc08874e57d5730544e734386d31775f29b` | repeat proving source, operator-confirmed1-1 |

Replay with the original `session_id` and `source.port` from each JSON, CLI channel3
and explicit `--evidence hardware`; do not compare export/identity annotations
byte-for-byte with replay's raw-derived tracker metadata. Original wrong-label
snapshots were retained separately; corrected derived exports record the operator
clarification while preserving raw event bytes/hash and observation values.

### Remaining boundaries and serial handoff

The observed set is empirically repeatable for these two patches, not all internal
synth settings. Default runtime still deliberately reports conservative partial
state; no generic automatic boundary or ready-profile behavior was enabled from
four observations. No clock/audio fidelity or physical units were measured. Live
unplug/replug was not requested; disconnect/reconnect remains synthetic-test proof.
Only cutoff was physically moved; other switch thresholds remain unverified.

The two distinct real source captures are available locally for review/Part2 after
acceptance. A different machine/agent must obtain these private artifacts; the
public repository alone is **not** the complete input dataset. No public real-patch
fixtures or redistribution grant were added. Human slot labeling for wire1 is
still unknown, but its original/repeat data are uniquely identified and preserved.
Part2 remains unstarted, pending acceptance of this delivery. No merge or flash.

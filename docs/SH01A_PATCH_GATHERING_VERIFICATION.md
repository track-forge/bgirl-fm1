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

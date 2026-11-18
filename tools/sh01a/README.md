# SH-01A input-only patch gathering (Part 1, issue #30)

Python 3.9+; live transport is mido 1.3.3 / python-rtmidi 1.5.8. No Ableton
integration or routing changes. No output port, CC/PC send, SysEx request, audio
capture, firmware change, or network service. Run from repository root:

```sh
python3 -m venv ~/.local/share/sh01a-capture/venv
~/.local/share/sh01a-capture/venv/bin/pip install -r tools/sh01a/requirements-live.txt
# Abbreviate this interpreter as $PY below; it is not a required environment var.
PY=~/.local/share/sh01a-capture/venv/bin/python
$PY -m tools.sh01a list-ports
$PY -m tools.sh01a inventory
$PY -m tools.sh01a monitor --port 'Boutique SH-01A' --channel 1 --session ./local/sh01a/session-01
```

Monitor is a foreground process; keep its terminal open (or use an explicit
operator-owned nohup invocation). It creates the session directory; parents must
be writable. SIGINT/SIGTERM closes input and marks state stale. Do not reuse a
session containing events: start a new directory after restart. A flock prevents
concurrent writers. No automatic restart service is installed.

In a second terminal, from the same repo and using the same interpreter:

```sh
$PY -m tools.sh01a arm --session ./local/sh01a/session-01 --label 'saved 2-1'
$PY -m tools.sh01a status --session ./local/sh01a/session-01
# Wait for last_command.id to match the returned arm id, result accepted.
# MANUALLY load saved 2-1 only after acceptance. See safety/experiment below.
$PY -m tools.sh01a baseline --session ./local/sh01a/session-01
# Wait for baseline command acknowledgement before moving a slider.
$PY -m tools.sh01a snapshot --session ./local/sh01a/session-01 --source-revision "$(git rev-parse HEAD)" --output ./local/sh01a/baseline-2-1
# Move cutoff, then export another unique output directory.
$PY -m tools.sh01a snapshot --session ./local/sh01a/session-01 --source-revision "$(git rev-parse HEAD)" --output ./local/sh01a/edited-2-1
$PY -m tools.sh01a diff --baseline ./local/sh01a/baseline-2-1/patch.json --current ./local/sh01a/edited-2-1/patch.json
$PY -m tools.sh01a replay ./local/sh01a/baseline-2-1/events.jsonl --channel 1 --session-id ORIGINAL_SESSION_ID --port 'Boutique SH-01A' --evidence hardware --output ./local/sh01a/replayed.json
```

`replay` defaults to **synthetic** evidence, never opens MIDI, and produces tracker
state (not a new provenance-bearing export). Supply original session ID/port to
reconstruct identical capture IDs. Raw bytes/time/sequence stay authoritative.
Different receive channels can be replayed from the same full raw log. Wire
channels are 0-based in status bytes; CLI channels are **1..16**. A wrong channel
produces missing data, not a fabricated baseline. `diff` compares parameter raw
values in two exported patch JSONs, including typed added/removed/changed entries.
The state's `changes` compares against the explicitly frozen baseline; returning
all knobs to baseline clears the net diff but never erases event history.

## First proving-patch experiment (operator required)

1. Keep desired unsaved edits safe before switching. Do not save/overwrite anything
   solely for this experiment. Select a different **already saved** patch manually;
   tell the agent its bank/slot. This first switch identifies channel, CC burst,
   and PC/bank ordering. Enable CC Output Mode only manually if not already on.
2. Agent inspects raw received events and arms a new window; operator loads saved
   **2-1** after acknowledgement. Repeat this load for stable coverage/order proof.
3. Agent freezes baseline; operator moves cutoff and several chosen sliders/switches,
   noting which. Capture diffs; move cutoff back to test net revert.
4. Arm separately before the second distinct saved patch. Explicitly label its
   bank/slot manually; wire PC is separate, not an assumed bank/slot encoding.
5. Export exact raw prefixes with hashes. Assess repeated transmitted-set coverage,
   sanitize only relevant events/labels for any public fixtures, and record operator
   provenance/redistribution permission before publishing patch material.

Clock reception and an open input alongside Live prove neither a patch-load burst
nor audible behavior. Do not close Live or alter routing to make this work.
Concurrent patch/control CC reception still requires the operator experiment.
If no CC arrives, report it; do not send a dump request or MIDI test messages.

## Contract, storage, limitations

- `snapshot.schema.json` defines portable v1 exports; Part 2 consumes this contract.
  `patch.json` + `events.jsonl` in each unique export directory is the artifact.
  Snapshot copies the exact byte prefix referenced by mutable `current.json` and
  checks SHA256, so later live events cannot change an export's evidence. Export
  directories are never overwritten. Wait until patch loading settles before
  exporting. A failed filesystem write can leave an incomplete directory: discard
  that directory manually or choose a fresh name; do not treat it as a capture.
- `current.json` and `status.json` use fsync + atomic rename; readers do not open
  MIDI. Heartbeat every 0.5s; default stale threshold 5s (`--stale-after` overrides).
  An export from a stopped/stale monitor is explicitly labeled stale. Baseline
  command errors are reported by `status.last_command`, not silently accepted.
- `events.jsonl` is append-only with monotonic sequence and UTC receive times;
  includes arm/baseline/lifecycle markers, **all channels** and all received messages
  (clock, note, unknown and SysEx included). Known observations retain source CC,
  raw value, time/sequence, category, null decoded physical value and rationale.
  In-memory unknown summaries are bounded at 128 entries and PC at 64; the full
  log is authoritative. No transport can prove absence of undetectable drops.
- Startup does not treat knob wiggles as a saved patch. Explicit arm drains pending
  old messages before clearing state. Arm clears all old fields and baseline.
  If PC arrives, clear old fields/baseline again and retain a sticky ambiguity.
  PC-before-burst yields candidate partial data; PC-after-burst clears it. Neither
  ordering is called correct automatically. Late old messages without an observable
  boundary remain ambiguous; manual recapture is necessary. Disconnect or reconnect
  disables capture until a new arm; old values may remain visible only as stale.
- **All captures remain partial in this staged version**, even with every candidate
  present. There is not yet a hardware-verified transmitted-set profile. Quiet time
  and the manual baseline action never promote to ready. Full physical internal
  state cannot be inferred from CC alone. No acoustic approximations are introduced.
- Definitions inventory: **42 named CCs**, including 3 receive-only controls
  (11/64/65). 1 is performance; 76/77 are conservatively classified system here.
  There are 36 documented patch candidates. Chart marks 71–87 transmitted but
  gives no name for **84**; it remains unknown rather than invented. This system/
  patch categorization is utility policy, not a claim about saved hardware scope.
- Port names and full logs are private by default. Store under ignored `local/`.
  Nothing auto-publishes. New exports default to **no redistribution grant**;
  operator-owned patch licensing is a separate recorded decision. This utility
  follows repository GPL-3.0-only; official manuals remain Roland copyright.

## Mapping sources

Read directly 2026-11-18; page 1 of each:

- [Roland SH-01A MIDI Implementation Chart, 2017-08-08, v1.00](https://static.roland.com/assets/media/pdf/SH-01A_MIDI_Imple_Chart_eng01_W.pdf):
  names, transmit/receive direction, channel, PC 0..63, no SysEx. Selector thresholds
  and physical response laws are NOT specified. `definitions.py` records each row's
  URL/version/date/page; `inventory` publishes the full table.
- [SH-01A Owner's Manual eng04](https://static.roland.com/assets/media/pdf/SH-01A_eng04_W.pdf):
  CC Output Mode ON: “Each time you switch patches, all parameters are output as CC.”
  This is documentation, not empirical proof of exact set/order/timing.

## Tests / staged handoff

```sh
python3 -m venv /tmp/sh01a-test
/tmp/sh01a-test/bin/pip install -r tools/sh01a/requirements-test.txt
/tmp/sh01a-test/bin/python -m unittest discover -s tools/sh01a/tests -v
```

Tests use explicitly synthetic messages and an input-only backend spy, not a real
patch dump. Schema validation uses jsonschema; live/offline runtime does not.
See [verification](../../docs/SH01A_PATCH_GATHERING_VERIFICATION.md) for observed
hardware facts and the still-open two-capture gate. No firmware/browser tests are
needed for this Python-only change; no firmware source or build pins changed.

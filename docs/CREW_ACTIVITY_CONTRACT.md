# CREW publication contract

Core0 owns all writes: normal accepted voice starts / winning mono legato set a
per-track pending peak velocity; engine-deferred requests, muted receipt and losing
mono priority do not. No worker writes telemetry. `crew_publish` runs after render
and the CPU2 join/recovery, inspecting admitted active, non-stolen voices there
only (never in UI). A failed/retired voice cannot sustain activity. Unison and
chords coalesce to one peak, not extra phase increments. UI reads one relaxed
lock-free aligned uint32 per selected track; no seqlock/retry or mutable voice walk.

Word: bits0–7 phrase phase (one turn/four quarters);8 active;9 settling;10 muted;
11–12 accent strength;13 has ever had activity since reset. All visible fields
belong to that same word. Producer-owned sample countdowns saturate, never use
absolute wrapping timestamps. A global Q32 phrase phase advances by musical elapsed
samples / (4 * beat_samples); BPM edits change slope, not phase. External playback
uses the existing interpolated external-clock GRID units (24000/quarter), not
audio samples; stopped live play uses
local audio samples and measured/current BPM. START resets phase, STOP/CONTINUE
preserve it. Skipped UI frames sample current phase; no event history or replay.

Sustained activity means active gated non-oneshot admitted voices, not physical
keys. Every admitted synth onset also gets a short90ms pulse, so note-on/off in
the same block still accents; it does not require the gate to remain high. One-shots (including vocal phrases) get a bounded 250ms attack envelope
regardless of a held key, then one quarter-note settle. Released sustained gates
get the same one-quarter settle. Inactive phase freezes; panic/project/init reset
all transient state. Muting/performance exclusion parks immediately, discards
pending accents, and inhibits rejoin until a genuine accepted attack; unmute alone
does not resume held voices. Level0/carrier routing are not calibrated sound-level
measurements. Accent lasts90ms, strength has a minimum and is max-coalesced.

The UI maps only the selected track phase to a22-step ping-pong walk through
12 masks:0..11,10..1, then0 again. One full forward/back cycle spans four beats;
endpoints are not held twice. Activity still freezes/settles the same phase. At most15 graph paints/s
while active (latest snapshot, not a queue); selection/mute/reset changes are
immediate. Hidden/covered CREW has no animation timer, phase evaluation or painting;
there are no six background dancer loops. Existing ARP telemetry is independent.

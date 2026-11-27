/* SPDX-License-Identifier: GPL-3.0-only */
/* One packet-loss insert, owned by the audio renderer. UI publishes whole
 * control words; the worker sees a stable owner and settings for each block. */
#define DROP_PACKET (FS / 50u)
#define DROP_FADE 32

static struct {
    int16_t packet[2][DROP_PACKET];
    uint32_t rng, phase;
    unsigned fill, read, bank, left;
    int gain, last, seam;
    uint8_t owner, valid, held, fired, forced;
} drop = {.rng = 0x83a14297u};

static uint8_t drop_loss, drop_mix, drop_start;
static int drop_grid, drop_synced;

static void dropout_begin(void)
{
    uint32_t base = drop_base, hold = drop_hold;
    uint32_t cfg = hold ? hold : base;
    unsigned owner = cfg & 7u;
    unsigned step = crew_phase >> 28;
    drop_grid = song.playing && (drop_start || step != drop.phase);
    drop_start = 0;
    drop.phase = step;
    /* Retain the old owner long enough to fade a released wet signal. */
    if (!owner && drop.gain)
        owner = drop.owner;
    if (!owner)
        owner = drop_monitor;
    if (owner != drop.owner) {
        drop.owner = (uint8_t)owner;
        drop.fill = drop.read = drop.left = 0;
        drop.gain = drop.valid = drop.fired = drop.forced = 0;
    }
    if (!hold && drop.held) {
        if (drop.forced) drop.left = 0;
        drop.fired = drop.forced = 0;
    }
    drop.held = hold != 0;
    drop_loss = (cfg >> 3) & 127u;
    drop_mix = (cfg >> 10) & 127u;
    drop_synced = song.playing && (cfg >> 17);
    if ((!drop_loss && !hold) || !drop_mix) drop.left = 0;
}

static void dropout_track(track_t *t, int32_t *b, int32_t *side, uint32_t n)
{
    if (drop.owner != (unsigned)(t - trk) + 1u)
        return;

    for (uint32_t i = 0; i < n; i++) {
        int32_t dry = b ? b[i] : 0;
        int opportunity = drop_synced ? i == 0 && drop_grid : drop.fill == 0;
        if (opportunity && (drop.valid || drop.held) && !drop.left && !drop.gain && drop_mix && (drop_loss || drop.held)) {
            unsigned loss = drop.held ? (drop_loss < 108u ? 108u : drop_loss) : drop_loss;
            drop.rng = drop.rng * 1664525u + 1013904223u;
            int force = drop.held && !drop.fired;
            if (force || (drop.rng >> 25) < loss) {
                drop.left = DROP_PACKET * (1u + ((loss * (1u + ((drop.rng >> 21) & 3u))) >> 7));
                drop.read = 0;
                drop.last = drop.valid ? drop.packet[drop.bank ^ 1u][DROP_PACKET - 1u] : 0;
                drop.forced = drop.held;
                drop.fired |= drop.held;
            }
        }
        /* Live input keeps advancing. Capture only while not replaying, so
         * a dropped/partial packet can never replace the last good packet. */
        if (drop.left || drop.gain) {
            unsigned pos = drop.read % DROP_PACKET;
            /* Consecutive missing packets decay. With no history, conceal with silence. */
            int wet = drop.valid ? drop.packet[drop.bank ^ 1u][pos] >> (drop.read / DROP_PACKET) : 0;
            if (!pos) drop.seam = drop.last;
            if (pos < DROP_FADE)
                wet = (wet * (int)pos + drop.seam * (DROP_FADE - (int)pos)) / DROP_FADE;
            drop.last = wet;
            int target = drop.left && drop_mix ? drop_mix + 1 : 0;
            drop.gain += clamp(target - drop.gain, -4, 4);
            if (b)
                b[i] = dry + wet * drop.gain - (dry >> 7) * drop.gain;
            if (side)
                side[i] -= (side[i] >> 7) * drop.gain;
            drop.read++;
            if (drop.left) drop.left--;
            drop.fill = 0;
        } else {
            drop.packet[drop.bank][drop.fill++] = (int16_t)clamp(dry >> 7, -32768, 32767);
            if (drop.fill == DROP_PACKET) {
                drop.fill = 0;
                drop.bank ^= 1u;
                drop.valid = 1;
            }
        }
    }
}

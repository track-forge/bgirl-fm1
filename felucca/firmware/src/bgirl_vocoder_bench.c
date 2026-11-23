/* SPDX-License-Identifier: GPL-3.0-only
 * Opt-in hardware timing. The audio core owns counter writes; the console
 * can only queue a new interval/mode at a half-buffer boundary. */
enum { VOC_RENDER, VOC_SETUP, VOC_JOIN, VOC_WET, VOC_SILENT, VOC_TIMER, VOC_N };
enum { VOC_FLOAT, VOC_BYPASS, VOC_FIXED, VOC_OPT };
static volatile struct voc_profile {
    uint32_t epoch, mode, missing, overlap;
    uint32_t peak[VOC_N], calls[VOC_N];
} voc_bench;
static volatile uint32_t voc_request;

static void voc_bench_request(uint32_t mode)
{
    voc_request = ((voc_request + 4u) & ~3u) | mode;
}

static void voc_bench_sample(uint32_t stage, uint32_t start)
{
    uint32_t elapsed = fm1_ticks() - start;
    if (elapsed > voc_bench.peak[stage])
        voc_bench.peak[stage] = elapsed;
    voc_bench.calls[stage]++;
}

static void voc_bench_begin(void)
{
    uint32_t request = voc_request;
    if (request != voc_bench.epoch) {
        if ((request & 3u) != voc_bench.mode) {
            bg_vocoder_reset();
            if (bg_bus.owner >= 0)
                trk[bg_bus.owner].p[P_VHOLD] = 0;
        }
        for (uint32_t i = 0; i < VOC_N; i++)
            voc_bench.peak[i] = voc_bench.calls[i] = 0;
        voc_bench.overlap = voc_bench.missing = 0;
        voc_bench.mode = request & 3u;
        voc_bench.epoch = request;
    }
    voc_bench_sample(VOC_TIMER, fm1_ticks());
}

/* A worst half plus four recent events, each with the two preceding halves.
 * The IRQ owns the records. Off freezes them for main-loop inspection. */
struct voc_half {
    uint32_t id, total, nested, parts, capture, bank, calls, fx, tick, voices, flags;
    uint8_t step[NPART];
};
static volatile uint32_t voc_trace_request;
static uint32_t voc_trace_ack, voc_event_next;
static struct voc_half voc_current, voc_prior[2], voc_worst, voc_events[4][3];

static void voc_trace_begin(void)
{
    if (voc_trace_request != voc_trace_ack) {
        if (voc_trace_request & 1u) {
            memset(voc_prior, 0, sizeof voc_prior);
            memset(&voc_worst, 0, sizeof voc_worst);
            memset(voc_events, 0, sizeof voc_events);
            voc_event_next = 0;
        }
        voc_trace_ack = voc_trace_request;
    }
    if (voc_trace_ack & 1u)
        memset(&voc_current, 0, sizeof voc_current);
}

static void voc_trace_end(uint32_t id, uint32_t elapsed, uint32_t nested, uint32_t flags)
{
    if (!(voc_trace_ack & 1u) || (!song.playing && !flags)) return;
    voc_current.id = id;
    voc_current.total = elapsed;
    voc_current.nested = nested;
    voc_current.tick = song.tick;
    voc_current.flags = flags;
    for (uint32_t p = 0; p < NPART; p++) {
        voc_current.step[p] = trk[p].seq_idx;
        for (uint32_t v = 0; v < NVOICE; v++)
            voc_current.voices += trk[p].v[v].active != 0;
    }
    if (elapsed > voc_worst.total)
        voc_worst = voc_current;
    if (flags) {
        struct voc_half *event = voc_events[voc_event_next++ % 4];
        event[0] = voc_prior[0];
        event[1] = voc_prior[1];
        event[2] = voc_current;
    }
    voc_prior[0] = voc_prior[1];
    voc_prior[1] = voc_current;
}

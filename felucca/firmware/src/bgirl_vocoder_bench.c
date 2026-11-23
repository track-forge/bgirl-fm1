/* SPDX-License-Identifier: GPL-3.0-only
 * Opt-in hardware timing. The audio core owns counter writes; the console
 * can only queue a new interval/mode at a half-buffer boundary. */
enum { VOC_RENDER, VOC_SETUP, VOC_JOIN, VOC_WET, VOC_SILENT, VOC_TIMER, VOC_N };
enum { VOC_FLOAT, VOC_BYPASS, VOC_FIXED };
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

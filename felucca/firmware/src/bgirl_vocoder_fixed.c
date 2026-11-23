/* SPDX-License-Identifier: GPL-3.0-only
 * Bench-only integer bank. Signals/envelopes use Q20; filter coefficients
 * use Q31 except the feedback term near 2, which uses Q30. */
#include "bgirl_vocoder_qcoeff.h"

#ifdef BGIRL_VOCODER_FIXED_TEST
static bg_fixed_t bg_fixed;
#endif

static int32_t bg_q31(int32_t x, int32_t q)
{
    return (int32_t)(((int64_t)x * q + (1ll << 30)) >> 31);
}

static int32_t bg_qinput(int32_t x, int32_t trim)
{
    /* Four bits of overload headroom at the input. */
    return clamp(x, -262144, 262144) * trim / 2;
}

static int32_t bg_qfilter(bg_qbq_t *s, const int32_t *c, int32_t x)
{
    int32_t feed = bg_q31(x, c[0]);
    /* Bound recursive terms even if a hot transient overshoots the input. */
    int32_t y = clamp(feed + s->z1, -(1 << 25), 1 << 25);
    s->z1 = (int32_t)(((int64_t)y * c[1] + (1ll << 29)) >> 30) + s->z2;
    s->z2 = -feed - bg_q31(y, c[2]);
    return y;
}

static inline __attribute__((always_inline))
int32_t bg_qfilter_local(bg_qbq_t *s, int32_t b0, int32_t a1, int32_t a2, int32_t x)
{
    int32_t feed = bg_q31(x, b0);
    int32_t y = clamp(feed + s->z1, -(1 << 25), 1 << 25);
    s->z1 = (int32_t)(((int64_t)y * a1 + (1ll << 29)) >> 30) + s->z2;
    s->z2 = -feed - bg_q31(y, a2);
    return y;
}

static int32_t bg_fixed_output(int32_t x, int32_t c, int64_t y, const int16_t *p)
{
    bg_fixed.hp += (x - bg_fixed.hp) / 4;
    y >>= 3;
    if (c > 104 || c < -104)
        y += ((int64_t)(x - bg_fixed.hp) * p[P_VCONS] * 7) / 2540;
    y = p[P_VWET] == 127 ? y / 32 :
        (x * (127 - p[P_VWET]) + y * p[P_VWET]) / (127 * 32);
    return y > 49152 ? 49152 : y < -49152 ? -49152 : (int32_t)y;
}

static void bg_fixed_reset(void) { memset(&bg_fixed, 0, sizeof bg_fixed); }

static void bg_fixed_run(int32_t *mod, const int32_t *car, uint32_t n, const int16_t *p, int optimized)
{
    uint32_t mode = p[P_VBANDS] >= 2 ? 2 : p[P_VBANDS] > 0 ? 1 : 0;
    uint32_t bands = mode == 2 ? 24 : mode ? 12 : 8;
    const int32_t (*coef)[3] = mode == 2 ?
#ifdef BGIRL_VOCODER_NARROW24
        BG_QCOEF24 :
#else
        BG_QCOEF24_WIDE :
#endif
        mode ? BG_QCOEF12 : BG_QCOEF8;
    if (bg_fixed.mode != mode) {
        bg_fixed_reset();
        bg_fixed.mode = mode;
    }
    if (p[P_VHOLD] && !bg_fixed.hold)
        for (uint32_t j = 0; j < bands; j++)
            bg_fixed.held[j] = bg_fixed.env[j];
    bg_fixed.hold = p[P_VHOLD] != 0;
    int32_t release = (int32_t)(2147483648.f / (44.1f *
        (p[P_VRANGE] ? BG_REL_MS[clamp(p[P_VREL], 0, 127)] : p[P_VREL] + 1)));
    if (optimized && bands == 8 && n <= CTL) {
        int32_t x[CTL], c[CTL];
        int64_t sum[CTL] = {0};
        for (uint32_t i = 0; i < n; i++) {
            x[i] = bg_qinput(mod[i], p[P_VMTRIM]);
            c[i] = bg_qinput(car[i], p[P_VCTRIM]);
        }
        /* Bands are independent. Keep coefficients and recurrence state local
         * across the control block, preserving rounding and band sum order. */
        for (uint32_t j = 0; j < bands; j++) {
            int32_t gain = j < bands / 3 ? p[P_VLOW] : j < 2 * bands / 3 ? p[P_VMID] : p[P_VHIGH];
            const int32_t b0 = coef[j][0], a1 = coef[j][1], a2 = coef[j][2];
            bg_qbq_t analysis = bg_fixed.analysis[j], synthesis = bg_fixed.synthesis[j];
            int32_t env = bg_fixed.env[j], applied = bg_fixed.applied[j];
            const int32_t held = bg_fixed.held[j];
            const uint32_t hold = bg_fixed.hold;
            for (uint32_t i = 0; i < n; i++) {
                int32_t a = bg_qfilter_local(&analysis, b0, a1, a2, x[i]);
                if (a < 0) a = -a;
                int32_t delta = a - env;
                env += delta < 0 ? (int32_t)(((int64_t)delta * release) >> 31)
                                 : bg_q31(delta, 53687091);
                applied += bg_q31((hold ? held : env) - applied, 21474836);
                int32_t carrier = bg_qfilter_local(&synthesis, b0, a1, a2, c[i]);
                int32_t contribution = (int32_t)(((int64_t)carrier * applied) >> 20);
                sum[i] += (int64_t)contribution * gain;
            }
            bg_fixed.analysis[j] = analysis;
            bg_fixed.synthesis[j] = synthesis;
            bg_fixed.env[j] = env;
            bg_fixed.applied[j] = applied;
        }
        for (uint32_t i = 0; i < n; i++)
            mod[i] = bg_fixed_output(x[i], c[i], sum[i], p);
        return;
    }
    int32_t gains[24];
    for (uint32_t j = 0; j < bands; j++)
        gains[j] = j < bands / 3 ? p[P_VLOW] : j < 2 * bands / 3 ? p[P_VMID] : p[P_VHIGH];

    for (uint32_t i = 0; i < n; i++) {
        int32_t x = bg_qinput(mod[i], p[P_VMTRIM]);
        int32_t c = bg_qinput(car[i], p[P_VCTRIM]);
        int64_t y = 0;
        for (uint32_t j = 0; j < bands; j++) {
            int32_t a = bg_qfilter(&bg_fixed.analysis[j], coef[j], x);
            if (a < 0) a = -a;
            int32_t delta = a - bg_fixed.env[j];
            /* A sub-LSB delta must still decay, rather than sustain a noise floor. */
            bg_fixed.env[j] += delta < 0 ? (int32_t)(((int64_t)delta * release) >> 31)
                                         : bg_q31(delta, 53687091);
            int32_t target = bg_fixed.hold ? bg_fixed.held[j] : bg_fixed.env[j];
            bg_fixed.applied[j] += bg_q31(target - bg_fixed.applied[j], 21474836);
            int32_t carrier = bg_qfilter(&bg_fixed.synthesis[j], coef[j], c);
            y += (((int64_t)carrier * bg_fixed.applied[j]) >> 20) * gains[j];
        }
        mod[i] = bg_fixed_output(x, c, y, p);
    }
}
#define bg_fixed_process(mod, car, n, p) bg_fixed_run(mod, car, n, p, 0)

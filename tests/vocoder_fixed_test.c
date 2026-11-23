/* SPDX-License-Identifier: GPL-3.0-only */
#define BGIRL_VOCODER_FIXED_TEST
#include "../felucca/web/emu/felucca_web.c"
#include <assert.h>

int main(void)
{
    int16_t p[P_COUNT];
    for (uint32_t k = 0; k < P_COUNT; k++) p[k] = TP[k].def;
    p[P_VWET] = 127;
    p[P_VRANGE] = 1;
    p[P_VREL] = bg_release_value(150);
    for (int mode = 0; mode < 3; mode++) {
        p[P_VBANDS] = mode;
        bg_vocoder_reset();
        bg_fixed_reset();
        double energy = 0, fixed_energy = 0, error = 0;
        for (uint32_t frame = 0; frame < 44100; frame += CTL) {
            int32_t ref[CTL], fixed[CTL], car[CTL];
            for (uint32_t i = 0; i < CTL; i++) {
                double t = (frame + i) / 44100.;
                ref[i] = fixed[i] = (int32_t)(14000 * sin(6.2831853 * (100 * t + 350 * t * t)));
                car[i] = (int32_t)(12000 * (2 * fmod(t * 103, 1.) - 1));
            }
            bg_vocoder_process(ref, car, CTL, p);
            bg_fixed_process(fixed, car, CTL, p);
            for (uint32_t i = 0; i < CTL; i++) {
                double diff = fixed[i] - ref[i];
                energy += (double)ref[i] * ref[i];
                fixed_energy += (double)fixed[i] * fixed[i];
                error += diff * diff;
                assert(fixed[i] >= -49152 && fixed[i] <= 49152);
            }
        }
        printf("mode %d: level difference %.3f dB, error %.1f dB below reference\n",
               mode, 10 * log10(fixed_energy / energy), -10 * log10(error / energy));
        assert(energy > 0 && error < energy * 0.01);
        int32_t zero[CTL] = {0}, signal[CTL];
        p[P_VHOLD] = 1;
        memset(bg_fixed.synthesis, 0, sizeof bg_fixed.synthesis);
        for (uint32_t i = 0; i < CTL; i++) signal[i] = 12000;
        bg_fixed_process(signal, zero, CTL, p);
        for (uint32_t i = 0; i < CTL; i++) assert(signal[i] == 0);
        assert(bg_fixed.hold);
        p[P_VHOLD] = 0;
        for (uint32_t b = 0; b < 1000; b++) {
            for (uint32_t i = 0; i < CTL; i++) signal[i] = i & 1 ? INT32_MIN : INT32_MAX;
            bg_fixed_process(signal, signal, CTL, p);
            for (uint32_t i = 0; i < CTL; i++) assert(signal[i] >= -49152 && signal[i] <= 49152);
        }
    }
}

/* SPDX-License-Identifier: GPL-3.0-only */
/* Usage: render_six OUTPUT.wav [solo 1..6]. Native browser renderer; no device build. */
#include "../felucca/web/emu/felucca_web.c"
#include <assert.h>
static void le16(FILE *f, uint32_t x) {
    fputc(x & 255, f);
    fputc((x >> 8) & 255, f);
}
static void le32(FILE *f, uint32_t x) {
    le16(f, x);
    le16(f, x >> 16);
}
int main(int argc, char **argv) {
    assert(argc == 2 || argc == 3);
    FILE *f = fopen(argv[1], "wb");
    assert(f);
    unsigned frames = FS * 12;
    fwrite("RIFF", 1, 4, f);
    le32(f, 36 + frames * 4);
    fwrite("WAVEfmt ", 1, 8, f);
    le32(f, 16);
    le16(f, 1);
    le16(f, 2);
    le32(f, FS);
    le32(f, FS * 4);
    le16(f, 4);
    le16(f, 16);
    fwrite("data", 1, 4, f);
    le32(f, frames * 4);
    web_nor_erase();
    web_boot();
    assert(!web_bgirl_six_project(0));
    for (unsigned i = 0; i < 4096; i += 128)
        web_render(128);
    if (argc == 3) {
        unsigned solo = (unsigned)atoi(argv[2]);
        assert(solo >= 1 && solo <= 6);
        for (unsigned t = 0; t < 6; t++)
            trk[t].p[P_MUTE] = (t + 1) != solo;
    }
    double energy = 0;
    float peak = 0;
    uint64_t start = now_ns();
    seq_start();
    for (unsigned n = 0; n < frames; n += 128) {
        unsigned count = frames - n < 128 ? frames - n : 128;
        web_render(count);
        assert(voices_busy() <= 8);
        for (unsigned i = 0; i < count; i++) {
            float v[2] = {out_l[i], out_r[i]};
            for (unsigned c = 0; c < 2; c++) {
                assert(isfinite(v[c]) && fabsf(v[c]) <= 1);
                if (fabsf(v[c]) > peak)
                    peak = fabsf(v[c]);
                energy += v[c] * v[c];
                le16(f, (uint16_t)(int16_t)lrintf(v[c] * 32767));
            }
        }
    }
    seq_stop();
    fclose(f);
    printf("12s six-track 120BPM: bytes=%u peak=%.8g RMS=%.8g render_wall_ms=%.3f tracks=%u voices=%u\n",
           project_bytes(), peak, sqrt(energy / (frames * 2)), (now_ns() - start) / 1e6, NTRK, NVOICE);
}

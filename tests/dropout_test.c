/* SPDX-License-Identifier: GPL-3.0-only */
#include <assert.h>
#define main hostsim_main
#include "../felucca/tests/hostsim.c"
#undef main

static int32_t render(track_t *track, int32_t sample, unsigned n)
{
    int32_t b[CTL];
    assert(n <= CTL);
    for (unsigned i = 0; i < n; i++) b[i] = sample;
    dropout_begin();
    dropout_track(track, b, 0, n);
    return b[n - 1];
}

int main(void)
{
    drop_monitor = 1;
    song.playing = 1;
    drop_controls[0][2] = 1;
    for (unsigned i = 0; i < DROP_PACKET; i++)
        assert(render(&trk[0], 16384, 1) == 16384);
    assert(drop.valid);

    drop_hold = dropout_controls(0);
    assert(render(&trk[0], 32768, CTL) == 32768);
    drop_hold = 0;
    crew_phase += 1u << 28;
    assert(render(&trk[0], 32768, CTL) == 32768); /* cancelled */

    drop_hold = dropout_controls(0);
    assert(render(&trk[0], 32768, CTL) == 32768); /* waits */
    crew_phase += 1u << 28;
    assert(render(&trk[0], 32768, CTL) == 16384); /* repeats last good packet */
    assert(render(&trk[1], 8192, CTL) == 8192);   /* other track untouched */
    drop_hold = 0;
    assert(render(&trk[0], 32768, CTL) == 32768); /* recovery fade */
    puts("Mr. Dropout: bypass, grid, cancellation, isolated replay and release OK");
    return 0;
}

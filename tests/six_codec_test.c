/* SPDX-License-Identifier: GPL-3.0-only */
/* B6S1 replaces conversion tests intentionally: legacy migration is out of scope.
 * Retains field/bounds, failure atomicity and exact editable-state coverage. */
#include "../felucca/web/emu/felucca_web.c"
#include <assert.h>
static project_t original, decoded, sentinel;
static project_store_t wire, kept;
static void roundtrip(void) {
    assert(proj_pack(&wire, &original));
    assert(proj_import(&decoded, &wire, sizeof wire));
    original.sum = proj_sum(&original);
    assert(!memcmp(&original, &decoded, sizeof original));
}
static void bad(void) {
    decoded = sentinel;
    assert(!proj_import(&decoded, &wire, sizeof wire));
    assert(!memcmp(&decoded, &sentinel, sizeof decoded));
}
static void hash(void) { proj_wr(wire.raw + PROJ_END, proj_hash(wire.raw, PROJ_END), 4); }
int main(void) {
    web_nor_erase();
    web_boot();
    web_render(128);
    project_capture(&original);
    memset(&sentinel, 0xA5, sizeof sentinel);
    roundtrip();
    kept = wire;
    for (unsigned n = 0; n < PROJ_STORE_SIZE; n++) {
        decoded = sentinel;
        assert(!proj_import(&decoded, &wire, n));
        assert(!memcmp(&decoded, &sentinel, sizeof decoded));
    }
    for (unsigned i = 0; i < PROJ_STORE_SIZE; i += 13) {
        wire = kept;
        wire.raw[i] ^= 0x80;
        bad();
    }
    for (unsigned i = 10; i < 20; i++) {
        wire = kept;
        wire.raw[i] = i==12 ? 125 : 127;
        hash();
        bad();
    }
    for (unsigned n = 1; n <= 13; n++) {
        wire = kept;
        proj_wr(wire.raw, 0x46554e30u + n, 4);
        hash();
        bad();
    }
    wire = kept;
    wire.raw[8] = 255;
    wire.raw[9] = 255;
    hash();
    bad();
    wire = kept;
    for (unsigned t = 0; t < NTRK; t++) {
        original.mch[t] = (uint8_t)(t + 10);
        original.t[t].p[P_LEVEL] = 70 + t;
        original.fm6[t][90] = t + 12;
        original.vocal_id[t][7] = t + 40;
        for (unsigned i = 0; i < 64; i += 11) {
            step_t *s = &original.t[t].step[i];
            *s = (step_t){{60, 0, 127, 52}, 1, ST_REST, 255, 101, 255, 128, 110};
        }
    }
    original.sel = 5;
    original.chain.count = 16;
    for (unsigned i = 0; i < 16; i++) {
        original.chain.row[i].repeat = i + 1;
        for (unsigned t = 0; t < 6; t++)
            original.chain.row[i].slot[t] = (t + i) % 5;
    }
    original.motion.count = 128;
    original.motion.on = 63;
    for (unsigned i = 0; i < 128; i++)
        original.motion.event[i] =
            (motion_event_t){(i % 6) * 64 + i / 6, P_LEVEL | ((i & 1) ? MOTION_LOCK : 0), (int8_t)(i - 64)};
    roundtrip();
    puts("codec exact six-track editable fields, dormant notes/rests, FM6 OWN, vocal IDs, all MIDI routes, 16 chain "
         "rows,128 motion PASS");
    /* Recomputed-checksum mutation corpus, after JIANT's compact-codec test.
     * Rejection is atomic; any accepted record must be a valid editable state. */
    kept = wire;
    uint32_t seed = 0xB651;
    unsigned accepted = 0;
    for (unsigned i = 0; i < 20000; i++) {
        wire = kept;
        seed = seed * 1664525u + 1013904223u;
        unsigned at = 20u + seed % (PROJ_END - 20u);
        wire.raw[at] ^= (uint8_t)(1u << ((seed >> 24) & 7));
        hash();
        decoded = sentinel;
        if (proj_import(&decoded, &wire, sizeof wire)) {
            accepted++;
            assert(proj_ok(&decoded) && proj_values_ok(&decoded));
        } else
            assert(!memcmp(&decoded, &sentinel, sizeof decoded));
    }
    printf("codec 20000 checksum-valid damaged records: %u accepted valid, remaining atomically refused PASS\n",
           accepted);
    /* Find and prove exact boundary, with one-byte parameter additions. */
    assert(!project_new());
    project_capture(&original);
    for (unsigned t = 0; t < 6; t++)
        for (unsigned i = 0; i < 64; i++) {
            original.t[t].step[i] = (step_t){{60, 64, 67, 72}, 4, ST_NOTE, 255, 127, 255, 255, 100};
            if (proj_packed_size(&original) > 3650)
                goto tune;
        }
tune:;
    unsigned hit[3] = {0};
    for (unsigned t = 0; t < 6; t++)
        for (unsigned p = 0; p < P_COUNT; p++) {
            unsigned n = proj_packed_size(&original);
            if (n >= 3839 && n <= 3841) {
                kept = wire;
                int ok = proj_pack(&wire, &original);
                assert(ok == (n <= 3840));
                if (!ok) {
                    assert(proj_full);
                    assert(!memcmp(&kept, &wire, sizeof kept));
                } else {
                    assert(proj_import(&decoded, &wire, sizeof wire));
                }
                hit[n - 3839] = 1;
            }
            if (original.t[t].p[p] == proj_default(original.t[t].engine,p))
                original.t[t].p[p] = original.t[t].p[p] == 127 ? 126 : original.t[t].p[p] + 1;
            if (n > 3841)
                goto finished;
        }
finished:
    assert(hit[0] && hit[1] && hit[2]);
    kept = wire;
    original.t[0].p[0] = 128;
    assert(!proj_pack(&wire, &original) && !proj_full && !memcmp(&kept, &wire, sizeof kept));
    puts("codec 3839/3840/3841-byte boundaries, untouched destination, invalid values, every truncation, "
         "header/count/version/hash and legacy rejection PASS");
    return 0;
}

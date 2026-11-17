/* SPDX-License-Identifier: GPL-3.0-only */
#include "../felucca/web/emu/felucca_web.c"
#include <assert.h>
static void capacity(const char *name, project_t *q) {
    project_store_t wire;
    printf("capacity %-24s %u / %u pack=%d\n", name, proj_packed_size(q), PROJ_STORE_SIZE, proj_pack(&wire, q));
}
static void tick(unsigned n) {
    for (unsigned i = 0; i < n; i += CTL)
        web_render(CTL);
}
static void runtime(void) {
    assert(!web_bgirl_six_project(0));
    tick(4096);
    assert(project_bytes() < 3840);
    for (unsigned t = 0; t < 6; t++) {
        assert(trk[t].p[P_SLEN] == 64);
        unsigned notes = 0;
        for (unsigned i = 0; i < 64; i++)
            notes += !!trk[t].step[i].n;
        assert(notes);
    }
    /* Every part separately audible using the actual shared renderer. */
    for (unsigned t = 0; t < 6; t++) {
        for (unsigned k = 0; k < 6; k++)
            trk[k].p[P_MUTE] = k != t;
        seq_start();
        double energy = 0;
        for (unsigned i = 0; i < FS * 3; i += CTL) {
            web_render(CTL);
            for (unsigned j = 0; j < CTL; j++) {
                assert(isfinite(out_l[j]) && fabsf(out_l[j]) <= 1);
                energy += out_l[j] * out_l[j];
            }
        }
        seq_stop();
        tick(4096);
        printf("six solo T%u energy %.6g\n", t + 1, energy);
        assert(energy > .00001);
    }
    for (unsigned t = 0; t < 6; t++)
        trk[t].p[P_MUTE] = 0;
    /* Extended address never aliases track0/1, including the last step. */
    memset(&motion, 0, sizeof motion);
    assert(!motion_set_event(&trk[4], 63, P_PAN, 25));
    assert(!motion_set_event(&trk[5], 63, P_REV, 99));
    assert(motion_count(&trk[0]) == 0 && motion_count(&trk[1]) == 0);
    assert(motion_count(&trk[4]) == 1 && motion_count(&trk[5]) == 1);
    assert(motion.event[0].place == 319 && motion.event[1].place == 383);
    motion_store_t bad = motion;
    bad.event[0].place = 384;
    assert(!motion_valid(&bad));
    assert(!motion_set_lock(&trk[5], 63, P_LEVEL, 30));
    project_t q, d;
    project_store_t w;
    project_capture(&q);
    assert(proj_pack(&w, &q) && proj_import(&d, &w, sizeof w));
    assert(d.motion.event[0].place == 319);
    /* Normal sound undo is independent of save capacity, including track6. */
    track_select(5);
    int16_t level = trk[5].p[P_LEVEL];
    set_engine(0);
    undo_swap();
    assert(trk[5].p[P_LEVEL] == level);
    tick(2048);
    /* Banked GLO / HOME LEVELS: no edits from the two inactive cells. */
    track_select(4);
    assert(mix_base() == 4);
    ui_home_view = HV_LEVELS;
    int16_t before[6];
    for (unsigned t = 0; t < 6; t++)
        before[t] = trk[t].p[P_LEVEL];
    ui.ly = LAYER_GLO;
    ui.ly_t0 = LY_OPEN;
    fm1_in.buttons = ly_bit(LAYER_GLO);
    layer_knob(2, -10);
    layer_knob(3, -10);
    for (unsigned t = 0; t < 6; t++)
        assert(before[t] == trk[t].p[P_LEVEL]);
    layer_knob(0, -1);
    assert(trk[4].p[P_LEVEL] != before[4]);
    layer_knob(1, -1);
    assert(trk[5].p[P_LEVEL] != before[5]);
    ui.layer = 0;
    ui.ly = 0;
    fm1_in.buttons = 0;
    /* Preserve all16 learned bindings with a 17-bit, three-bit track field. */
    memset(favorites.factory[14], 0, 32);
    favorites.learn_hi = 0;
    for (unsigned t = 0; t < 6; t++) {
        assert(ml_learn(20 + t, t, P_E7) == 1);
        assert(ml_cc_of(t, P_E7) == 21 + t);
    }
    assert(favorites.learn_hi);
    persist_t p;
    settings_export(&p);
    memset(&favorites, 0, sizeof favorites);
    assert(settings_import(&p, sizeof p));
    for (unsigned t = 0; t < 6; t++)
        assert(ml_cc_of(t, P_E7) == 21 + t);
    p.magic = 0x50455234u;
    p.favorites.user = 123;
    p.favorites.factory[0][0] = 13;
    assert(settings_import(&p, sizeof p - 4));
    assert(!ml_count() && !favorites.learn_hi && favorites.user == 123 && favorites.factory[0][0] == 13);
    /* Every channel1..6 has independent ownership; note-offs go to original owner. */
    assert(!web_bgirl_six_project(0));
    tick(2048);
    for (unsigned t = 0; t < 6; t++) {
        web_midi(0x90 | t, 60 + t, 100);
        tick(1024);
        assert((midi_notes[t][60 + t] & 127) == t + 1);
    }
    track_select(5);
    for (unsigned t = 0; t < 6; t++) {
        web_midi(0x80 | t, 60 + t, 0);
        tick(1024);
        assert(!midi_notes[t][60 + t]);
    }
    /* Sound engines are unrestricted on both extra tracks. */
    for (unsigned t = 4; t < 6; t++)
        for (unsigned e = 0; e < NENGINES; e++) {
            if (e == ENGI_DIGITAL)
                continue;
            set_engine_of(&trk[t], e);
            apply_preset_to(&trk[t], 0);
            tick(1024);
            assert(trk[t].engine == e);
            trk_note_on(&trk[t], 60, 100);
            tick(512);
            trk_note_off(&trk[t], 60);
            trk_all_off(&trk[t]);
        }
    assert(!web_bgirl_six_project(0));
    tick(2048);
    for (unsigned round = 0; round < 64; round++) {
        unsigned t = round % 6;
        trk_note_on(&trk[t], 48 + round % 24, 110);
        tick(CTL);
        assert(voices_busy() <= 8);
    }
    for (unsigned t = 0; t < 6; t++)
        trk_all_off(&trk[t]);
    tick(FS);
    assert(!voices_busy());
    /* Explicit overflow keeps live state and both RAM/NOR committed copies. */
    assert(!web_bgirl_six_project(0));
    tick(2048);
    assert(!project_save(0));
    project_store_t old = proj_slot[0], flash;
    assert(!web_bgirl_six_project(1));
    tick(2048);
    project_capture(&q);
    assert(project_save(0) == 2 && proj_full);
    project_capture(&d);
    assert(!memcmp(&q, &d, sizeof q));
    assert(!memcmp(&old, &proj_slot[0], sizeof old));
    assert(st_load(OBJ_PROJECT0, &flash, sizeof flash) == sizeof flash && !memcmp(&flash, &old, sizeof old));
    /* Autosave tries FULL once per state, not every idle polling interval. */
    memset(&as, 0, sizeof as);
    as.saved = 0;
    as.seen = autosave_sig();
    as.t = fm1_ms;
    fm1_ms += AS_IDLE_MS + 1000;
    autosave_poll();
    assert(as.full_blocked && !as.writes);
    unsigned last = as.last;
    fm1_ms += AS_RETRY_MS * 10;
    autosave_poll();
    assert(as.last == last);
    seq_start();
    for(unsigned frame=0;frame<FS*2;frame+=CTL) {
        web_render(CTL);assert(voices_busy()<=NVOICE);
        for(unsigned i=0;i<CTL;i++)assert(isfinite(out_l[i])&&isfinite(out_r[i])&&fabsf(out_l[i])<=1&&fabsf(out_r[i])<=1);
    }
    seq_stop();for(unsigned t=0;t<NTRK;t++)trk_all_off(&trk[t]);tick(FS);assert(!voices_busy());
    assert(!web_bgirl_six_project(0));
    tick(1024);
    puts("six runtime audio/64-step tracks, engines, T5/T6 motion, undo, banks, MIDI learn/reset, channels1-6, "
         "ownership, eight-voice stress, full-save atomicity, autosave throttle PASS");
}

int main(void) {
    web_nor_erase();
    web_boot();
    web_render(128);
    project_t q;
    project_capture(&q);
    capacity("startup electro", &q);
    assert(NTRK == 6 && NVOICE == 8);
    assert(proj_packed_size(&q) < PROJ_STORE_SIZE);
    assert(!project_new());
    project_capture(&q);
    capacity("empty", &q);
    for (unsigned t = 0; t < NTRK; t++) {
        set_engine_of(&trk[t], ENGI_FM6);
        apply_preset_to(&trk[t], 0);
    }
    project_capture(&q);
    capacity("six FM6", &q);
    for (unsigned i = 0; i < MOTION_MAX; i++) {
        q.motion.event[i] = (motion_event_t){(i % 6) * 64 + i / 6, P_LEVEL, (int8_t)i};
    }
    q.motion.count = MOTION_MAX;
    q.motion.on = 63;
    capacity("six FM6 +128 motion", &q);
    for (unsigned t = 0; t < NTRK; t++)
        for (unsigned i = 0; i < NSTEP; i++) {
            step_t *s = &q.t[t].step[i];
            s->n = 4;
            s->time = ST_NOTE;
            s->flags = 255;
            s->vel = 127;
            s->hit = s->acc = 255;
            s->probability = 100;
            for (unsigned j = 0; j < 4; j++)
                s->note[j] = 60 + j;
        }
    capacity("dense six x64 x4", &q);
    for(unsigned t=0;t<NTRK;t++)for(unsigned i=0;i<P_COUNT;i++)q.t[t].p[i]=BG6_DEFAULTS[q.t[t].engine][i]==127?126:127;
    capacity("dense + all params", &q);
    printf("RAM track_t=%zu tracks=%zu engine_pool=%zu project=%zu motion=%zu chain=%zu wire=%zu\n", sizeof(track_t),
           sizeof trk, sizeof eng_mem, sizeof(project_t), sizeof(motion_store_t), sizeof(chain_config_t),
           sizeof(project_store_t));
    runtime();
    return 0;
}

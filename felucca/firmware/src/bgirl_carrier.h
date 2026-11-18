/* SPDX-License-Identifier: GPL-3.0-only; generated local wrapper around MIT engine presets. */
static const preset_t BG_ANALOG_PRESETS[] = {
    {"SAW LEAD", {0, 12, 64, 0, 90, 30, 10, 64}, {4, 70, 100, 50}, 20, 1, FX(0, 10, 45, 30), PAT(4)},
    {"SOFT PAD", {0, 20, 64, 4, 60, 10, 0, 32}, {80, 90, 110, 95}, 10, 0, FX(0, 60, 20, 70), PAT(5)},
    {"SQR BASS", {1, 0, 0, 0, 50, 70, 40, 64}, {0, 60, 40, 30}, 30, 1, FX(5, 0, 10, 10), PAT(2)},
    {"PWM STR", {4, 8, 40, 0, 75, 20, 0, 48}, {60, 80, 110, 85}, 8, 0, FX(0, 50, 20, 60), PAT(5)},
    {"ACID", {0, 0, 0, 0, 50, 100, 25, 64}, {0, 55, 20, 30}, 48, 1, FX(20, 0, 45, 15), PAT(1)},
    {"SINE KEY", {3, 6, 50, 0, 127, 0, 0, 0}, {2, 80, 30, 70}, 0, 0, FX(0, 30, 25, 40), PAT(6)},
    {"RAVE", {4, 30, 64, 0, 85, 20, 30, 64}, {20, 80, 110, 60}, 10, 1, FX(30, 40, 30, 30), PAT(13)},
    {"SUB BASS", {3, 0, 0, 0, 40, 0, 20, 0}, {0, 60, 100, 20}, 0, 1, FX(0, 0, 0, 10), PAT(8)},
    {"PLUCK", {0, 8, 50, 0, 30, 40, 0, 64}, {0, 88, 0, 60}, 55, 0, FX(0, 20, 50, 30), PAT(3)},
    {"BRASS", {0, 10, 64, 0, 45, 20, 10, 64}, {35, 70, 90, 45}, 40, 0, FX(0, 20, 20, 40), PAT(6)},
    {"WIND", {0, 0, 0, 90, 30, 90, 0, 0}, {60, 90, 60, 80}, 50, 0, FX(0, 30, 30, 70), PAT(5)},
    {"STRINGS", {0, 25, 64, 0, 70, 10, 0, 32}, {70, 90, 115, 90}, 5, 0, FX(0, 60, 20, 70), PAT(5)},
    {"BG CARR SAW", {0,0,0,0,127,0,0,0}, {0,0,127,20}, 0,1,FX(0,0,0,0),PAT(4)},
    {"Crunchy Bass", {6,0,42,0,106,42,0,0}, {0,17,57,0}, 12,1,FX(0,0,0,0),PAT(4)},
    {"Velvet Pulse", {4,0,0,0,25,55,0,127}, {16,26,81,18}, 55,1,FX(0,0,0,0),PAT(4)},
};
static const engine_t BG_ANALOG = {
    .name = "ANALOG",
    .page_title = {"OSC", "FLT"},
    .edit = {
        {"WAVE", F_ENUM, 0, 6, 0, N_ANALOG_WAVE, 0},
        {"DTN", F_INT, 0, 127, 10, 0, "ct"},
        {"MIX", F_PCT, 0, 127, 64, 0, 0},
        {"NOIS", F_PCT, 0, 127, 0, 0, 0},
        {"CUT", F_CUTOFF, 0, 127, 90, 0, 0},
        {"RES", F_PCT, 0, 127, 30, 0, 0},
        {"DRV", F_PCT, 0, 127, 0, 0, 0},
        {"KTR", F_PCT, 0, 127, 64, 0, 0},
    },
    .presets = BG_ANALOG_PRESETS,
    .npresets = NELEM(BG_ANALOG_PRESETS),
    .note_on = analog_note_on,
    .render = analog_render,
    .desc = analog_desc,
    .knob = {P_E4, P_E5, P_ATK, P_REL},
    .keep = 0x03,                /* the filter */
};
/* Normal factory loading applies extra common parameters, inside the existing load transaction. */
static void sh_style_factory_loaded(track_t *t) {
    if (t->eng_req != 0) return;
    switch(t->preset) {
    case 13:
        t->p[P_ATK] = 0;
        t->p[P_CHOR] = 0;
        t->p[P_DEC] = 17;
        t->p[P_DIST] = 0;
        t->p[P_DLY] = 0;
        t->p[P_E0] = 6;
        t->p[P_E1] = 0;
        t->p[P_E2] = 42;
        t->p[P_E3] = 0;
        t->p[P_E4] = 106;
        t->p[P_E5] = 42;
        t->p[P_E6] = 0;
        t->p[P_E7] = 0;
        t->p[P_ED_FLT] = 12;
        t->p[P_GLIDE] = 0;
        t->p[P_LD_FLT] = 50;
        t->p[P_LD_PIT] = 0;
        t->p[P_LRATE] = 22;
        t->p[P_LWAVE] = 1;
        t->p[P_REL] = 0;
        t->p[P_REV] = 0;
        t->p[P_SUS] = 57;
        t->p[P_VOICE] = 1;
        break;
    case 14:
        t->p[P_ATK] = 16;
        t->p[P_CHOR] = 0;
        t->p[P_DEC] = 26;
        t->p[P_DIST] = 0;
        t->p[P_DLY] = 0;
        t->p[P_E0] = 4;
        t->p[P_E1] = 0;
        t->p[P_E2] = 0;
        t->p[P_E3] = 0;
        t->p[P_E4] = 25;
        t->p[P_E5] = 55;
        t->p[P_E6] = 0;
        t->p[P_E7] = 127;
        t->p[P_ED_FLT] = 55;
        t->p[P_GLIDE] = 0;
        t->p[P_LD_FLT] = 0;
        t->p[P_LD_PIT] = 0;
        t->p[P_LRATE] = 65;
        t->p[P_LWAVE] = 1;
        t->p[P_REL] = 18;
        t->p[P_REV] = 0;
        t->p[P_SUS] = 81;
        t->p[P_VOICE] = 1;
        break;
    default: break;
    }
}

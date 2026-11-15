/* SPDX-License-Identifier: MPL-2.0
 * Copyright (C) 2026 Charles Vestal */
/* BGIRL electro app. Input/accessibility/storage adapted from the fm1-base template.
 * DSP and frame-clock sequencing live in dsp/, not the UI frame loop. */
#include "fastmath.h"
#include "dsp/electro.h"

#define SET_MAGIC 0x42475231u            /* "BGR1": the settings record; another magic loads defaults */
typedef struct {
    uint32_t magic;
    int8_t octave;                       /* -2 .. +2 */
    uint8_t wave;                        /* 0 bass, 1 experimental live vocoder */
    uint8_t ble_off;                     /* Bluetooth switched off (REC held); 0, the default: on */
    uint8_t rsv[1];
} settings_t;
static settings_t set;
static uint8_t set_dirty;                /* changed since the last save: SAVE is lit */

static const char *const PART_NAME[] = {"BASS", "VOICE"};
/* the controls by their names in plat.h, in its order */
static const char *const BTN_NAME[NB] = {"FX", "SEL", "ENV", "LFO", "EDIT", "GLO", "HOME", "SAVE", "ARP", "SEQ",
                                         "PLAY", "REC", "OCT-", "OCT+"};
static const char *const ENC_NAME[NE] = {"SELECT", "ALGO", "PRESET", "K1", "K2", "K3", "K4"};

/* ------------------------------------------------------------------ voice --- */
/* one voice, the last note wins; the main loop and MIDI in set it, the audio interrupt plays it */
/* One aligned scalar publishes note, velocity and retrigger generation together.
 * Low byte is MIDI note+1 (0 released); bits 8..14 velocity; high 17 bits generation.
 * UI and render are serialized by the platform's interrupt model, not a heap queue. */
static volatile uint32_t note_request;
#define voice_note ((int32_t)(note_request & 255u) - 1)
static electro engine;
static volatile uint32_t run_request, panic_serial;
static volatile uint32_t bpm_request=120, mute_request, tone_request=45;
static volatile uint32_t displayed_step, displayed_steps, displayed_running;

static void note_release_all(void) { note_request &= ~255u; }
static void note_on(int32_t note, uint32_t vel)
{
    uint32_t generation;
    if(note<0 || note>127) return;
    generation=(note_request+32768u)&0xffff8000u;
    note_request=generation | ((vel>127?127:vel)<<8) | (uint32_t)(note+1);
}
static void note_off(int32_t note)
{
    if(voice_note==note) note_release_all();
}

static volatile uint32_t midi_last, midi_last_port, midi_count;   /* the last message in (shown), and how many */
static void midi_in(void)                    /* audio interrupt: the packets that came in */
{
    uint32_t pkt;
    int port;
    while (plat_midi_in(&pkt, &port)) {
        uint32_t st = (pkt >> 8) & 0xFFu, d1 = (pkt >> 16) & 0x7Fu, d2 = (pkt >> 24) & 0x7Fu;
        if (st != 0xF8u && st != 0xFEu) {    /* (clock and active sensing would flood the screen) */
            midi_last = pkt;
            midi_last_port = (uint32_t)port;
            midi_count++;
        }
        if (st == 0xFFu || ((st & 0xF0u) == 0xB0u && (d1 == 120u || d1 == 123u)))
            app_panic();
        else if ((st & 0xF0u) == 0x90u && d2)
            note_on((int32_t)d1, d2);
        else if ((st & 0xF0u) == 0x80u || (st & 0xF0u) == 0x90u)
            note_off((int32_t)d1);
    }
}

void app_render(int32_t *lr, uint32_t n, uint32_t master_q12)
{
    static uint32_t last_panic;
    midi_in();
    if (panic_serial != last_panic) {
        last_panic=panic_serial;
        electro_panic(&engine);
    }
    if (run_request != engine.running)
        electro_transport(&engine, run_request);
    { uint32_t req=note_request;
      electro_note(&engine,(int)(req&255u)-1,(req>>8)&127u,req>>15,set.wave); }
    electro_render(&engine,lr,n,master_q12,bpm_request,mute_request,tone_request);
    displayed_step=engine.step;
    displayed_steps=engine.steps;
    displayed_running=engine.running;
}

void app_panic(void)
{
    note_release_all();
    run_request=0;
    panic_serial++;
}

/* --------------------------------------------------------------- settings --- */
static void settings_defaults(void)
{
    set.magic = SET_MAGIC;
    set.octave = 0;
    set.wave = 0;
    set.ble_off = 0;
}

void app_init(void)
{
    electro_init(&engine);
    if (plat_store_load(OBJ_SETTINGS, &set, sizeof set) != (int)sizeof set || set.magic != SET_MAGIC
        || set.octave < -2 || set.octave > 2 || set.wave > 1u || set.ble_off > 1u)
        settings_defaults();
}

/* --------------------------------------------------------------------- UI --- */
static struct {
    char msg[2][24];                     /* a short message over the header */
    uint32_t msg_until;
    uint32_t keys, btn;                  /* held at the last input */
    uint8_t key_note[NKEYS];             /* original MIDI note, even across octave changes */
    uint8_t vel[NKEYS], prs[NKEYS];      /* each key's last velocity, and its pressure while held */
    int32_t last_key;                    /* the key pressed last: its numbers in the header */
    uint8_t looking;                     /* Bluetooth MIDI devices: REC pressed, the scan's list up (lib/btdev) */
    uint32_t rec_t;                      /* REC went down (0: not held, or its hold already acted) */
    int32_t turns[NE];                   /* detents since boot, per encoder */
    uint32_t turned_at[NE];              /* when each was last turned: it shows lit for a moment */
    uint8_t redraw;
} ui;

static void text_cat(char *d, const char *s)
{
    while (*d)
        d++;
    while (*s)
        *d++ = *s++;
    *d = 0;
}

static void int_text(int32_t v, char *b)     /* signed, no padding */
{
    char t[12];
    int n = 0;
    uint32_t u = (uint32_t)(v < 0 ? -v : v);
    do
        t[n++] = (char)('0' + u % 10u);
    while ((u /= 10u) && n < 11);
    if (v < 0)
        *b++ = '-';
    while (n)
        *b++ = t[--n];
    *b = 0;
}

void app_say(const char *a, const char *b)
{
    uint32_t i;
    char t[48];
    for (i = 0; i < 23u && a && a[i]; i++)
        ui.msg[0][i] = a[i];
    ui.msg[0][i] = 0;
    for (i = 0; i < 23u && b && b[i]; i++)
        ui.msg[1][i] = b[i];
    ui.msg[1][i] = 0;
    ui.msg_until = plat_ms() + 1300u;
    ui.redraw = 1;
    if (ann_on()) {                          /* and to a screen reader (announce.c, fm1-reader) */
        t[0] = 0;
        text_cat(t, ui.msg[0]);
        if (ui.msg[1][0]) {
            text_cat(t, " ");
            text_cat(t, ui.msg[1]);
        }
        ann_say(t, 0, ANN_NOW);
    }
}

static void octave_text(char *t)
{
    t[0] = set.octave < 0 ? '-' : '+';
    t[1] = (char)('0' + (set.octave < 0 ? -set.octave : set.octave));
    t[2] = 0;
}

/* a screen reader that has just started listening (or asks) hears the whole screen */
static void speak(void)
{
    char t[96], o[4];
    if (ann_on() && ann_where()) {
        octave_text(o);
        t[0] = 0;
        text_cat(t, APP_NAME ". OCTAVE ");
        text_cat(t, o);
        text_cat(t, ". PART ");
        text_cat(t, PART_NAME[set.wave]);
        text_cat(t, run_request ? ". RUNNING " : ". STOPPED ");
        { char n[12]; int_text((int32_t)bpm_request,n);text_cat(t,n); }
        text_cat(t, mute_request ? " BPM. DRUMS MUTED" : " BPM. DRUMS ON");
        ann_say(t, 0, ANN_NOW);
    }
    ann_tick(plat_ms());
}

static void note_name(int32_t note, char *b)
{
    static const char *const N[] = {"C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B"};
    char o[12];
    b[0] = 0;
    text_cat(b, N[note % 12]);
    int_text(note / 12 - 1, o);
    text_cat(b, o);
}

/* Match all outgoing key-ons before octave/mode/stop/panic. Key-up later is harmless. */
static void release_keys(void)
{
    unsigned k;
    for(k=0;k<NKEYS;k++) if((ui.keys|touch_keys())&(1u<<k))
        plat_midi_send(0x08u | 0x80u<<8 | (uint32_t)ui.key_note[k]<<16,TO_USB|TO_BLE);
}

static void octave_step(int32_t d)
{
    int32_t o = clamp(set.octave + d, -2, 2);
    char t[4];
    if (o == set.octave)
        return;
    release_keys();
    note_release_all();                         /* a key held across the change would not find its note off */
    set.octave = (int8_t)o;
    set_dirty = 1;
    octave_text(t);
    app_say("OCTAVE", t);
}

int template_ble_off(void) { return set.ble_off; }

/* REC held a second: Bluetooth off or on (kept with the settings, so it stays so at the next power-on) */
static void ble_toggle(void)
{
    set.ble_off = (uint8_t)!set.ble_off;
    set_dirty = 1;
    plat_ble_enable(!set.ble_off);
    app_say("BLUETOOTH", set.ble_off ? "OFF" : "ON");
}

/* REC: a Bluetooth MIDI device (FM1B_BLE_CENTRAL, lib/btdev): look for devices, then connect to the one ALGORITHM
 * picks, then let it go */
static void ble_device_say(void)
{
    char name[21];
    if (btdev_found(btdev_pick(), name, 0))
        app_say("DEVICE", name);
    else
        app_say("BLE", "SCANNING");
}
static void ble_device_button(void)
{
    int ev;
    if (!btdev_busy() && !ui.looking) {
        ui.looking = 1;
        app_say("BLE", "SCANNING");
        return;
    }
    ev = btdev_save();
    ui.looking = 0;
    app_say(ev == BTDEV_CONNECTING ? btdev_word(ev) : "BLE", ev == BTDEV_CONNECTING ? btdev_name() : btdev_word(ev));
}

static void input(void)
{
    uint32_t keys, btn = plat_buttons(), k;
    int32_t e;
    touch_poll();                            /* the sensed keys' presses, velocities and pressure */
    keys = touch_keys();
    for (k = 0; k < NKEYS; k++) {            /* key 0 is F3 (MIDI 53) */
        uint32_t bit = 1u << k, v;
        int32_t note = 53 + (int32_t)k + 12 * set.octave;
        if ((keys & bit) && !(ui.keys & bit)) {
            v = touch_vel(k);
            ui.key_note[k]=(uint8_t)note;
            ui.vel[k] = (uint8_t)v;
            ui.prs[k] = 0;
            ui.last_key = (int32_t)k;
            note_on(note, v);
            plat_midi_send(0x09u | 0x90u << 8 | (uint32_t)note << 16 | v << 24, TO_USB | TO_BLE);   /* BLE: if built with it */
        } else if (!(keys & bit) && (ui.keys & bit)) {
            ui.prs[k] = 0;
            note=ui.key_note[k];
            note_off(note);
            plat_midi_send(0x08u | 0x80u << 8 | (uint32_t)note << 16, TO_USB | TO_BLE);
        } else if ((keys & bit) && touch_pressure(k) != ui.prs[k]) {
            note=ui.key_note[k];   /* pressure: poly aftertouch */
            ui.prs[k] = touch_pressure(k);
            ui.redraw = 1;
            plat_midi_send(0x0Au | 0xA0u << 8 | (uint32_t)note << 16 | (uint32_t)ui.prs[k] << 24, TO_USB | TO_BLE);
        }
    }
    for (k = 0; k < NB; k++) {               /* a button pressed: its name, and what it does here */
        uint32_t bit = 1u << k;
        if (!(btn & bit) || (ui.btn & bit))
            continue;
        if (k == B_OCTDN)
            octave_step(-1);
        else if (k == B_OCTUP)
            octave_step(1);
        else if (k == B_REC && FM1B_BLE)
            ui.rec_t = plat_ms();                /* a press: on release (a hold switches Bluetooth) */
        else if (k == B_ARP) {
            run_request=!run_request;
            if(!run_request) {release_keys();app_panic();}
            app_say("GROOVE",run_request?"RUNNING":"STOPPED");
        } else if (k == B_FX) {
            mute_request=!mute_request;
            app_say("DRUMS",mute_request?"MUTED":"ON");
        } else if (k == B_HOME) {
            release_keys();
            app_panic();
            app_say("PANIC","STOPPED");
        } else if (k == B_SAVE) {
            if (plat_store_save(OBJ_SETTINGS, &set, sizeof set) == 0) {
                set_dirty = 0;
                app_say("SAVED", "");
            } else {
                app_say("SAVE", "FAILED");
            }
        } else {
            app_say("BUTTON", BTN_NAME[k]);
        }
    }
    if (FM1B_BLE && ui.rec_t) {             /* REC: held a second, Bluetooth on / off; a press, a device */
        if (btn >> B_REC & 1u) {
            if (plat_ms() - ui.rec_t >= 1000u) {
                ui.rec_t = 0;
                ble_toggle();
            }
        } else {
            ui.rec_t = 0;
            if (FM1B_BLE_CENTRAL && !set.ble_off)
                ble_device_button();
            else
                app_say("BUTTON", "REC");
        }
    }
    for (k = 0; k < NE; k++) {               /* an encoder turned: PRESET selects the playable part */
        char t[12];
        if ((e = plat_enc((int)k)) == 0)
            continue;
        ui.turns[k] += e;
        ui.turned_at[k] = plat_ms();
        ui.redraw = 1;
        if (k == EN_ALGO && FM1B_BLE_CENTRAL && ui.looking) {
            btdev_choose(btdev_pick() + e);
            ble_device_say();
        } else if (k == EN_PRESET) {
            release_keys();
            note_release_all();
            set.wave ^= 1u;
            set_dirty = 1;
            app_say("PART", PART_NAME[set.wave]);
        } else if (k == EN_K1) {
            bpm_request=(uint32_t)clamp((int32_t)bpm_request+e,60,180);
            int_text((int32_t)bpm_request,t); app_say("TEMPO",t);
        } else if (k == EN_K2) {
            tone_request=(uint32_t)clamp((int32_t)tone_request+e,0,100);
            int_text((int32_t)tone_request,t); app_say("TONE",t);
        } else {
            int_text(ui.turns[k], t);
            app_say(ENC_NAME[k], t);
        }
    }
    if (keys != ui.keys || btn != ui.btn)
        ui.redraw = 1;
    ui.keys = keys;
    ui.btn = btn;
    plat_glow(1);                            /* the unlit buttons glow, readable in the dark */
    plat_leds(btn | (set_dirty ? 1u << B_SAVE : 0u) | (run_request ? 1u << B_ARP : 0u)
              | (mute_request ? 1u << B_FX : 0u), keys);
}

void app_input_only(void) { input(); }

static void cell(int32_t x, int32_t y, int32_t w, const char *s, int on)   /* a labelled box, lit when on */
{
    cv_rect(x, y, w, 18, on ? C_HI : C_LINE);
    cv_text(x + (w - text_w(&FONT_XS, s)) / 2, y + 2, &FONT_XS, s, on ? C_BLACK : C_GRAY);
}

static void key_cell(int32_t x, int32_t y, int32_t w, int32_t h, uint32_t k, uint16_t off)
{
    int32_t v, p;
    cv_rect(x, y, w, h, off);
    if (!(ui.keys >> k & 1u))
        return;
    v = (ui.vel[k] * h + 126) / 127;
    p = (ui.prs[k] * h + 126) / 127;
    cv_rect(x, y + h - v, w, v, C_HI);
    if (p)
        cv_rect(x + 2, y + h - p, w - 4, p, C_WHITE);
}

static void draw(void)
{
    char t[24];
    uint32_t i;
    static const uint16_t pattern[3]={0x0941,0x1010,0xaaaa};
    static const char *const names[3]={"KICK","SNARE","HAT"};
    static const uint8_t WK[16]=PLAT_WHITE_KEYS, BK[11]=PLAT_BLACK_KEYS;
    cv_begin(240,180,C_BLACK);
    cv_text(8,4,&FONT_L,APP_NAME,C_HI);
    cv_text(140,12,&FONT_S,run_request?"RUNNING":"STOPPED",C_WHITE);
    int_text((int32_t)bpm_request,t);
    cv_text(8,42,&FONT_L,t,C_WHITE);
    cv_text(68,55,&FONT_XS,"BPM",C_GRAY);
    cv_text(115,44,&FONT_M,PART_NAME[set.wave],C_HI);
    cv_text(8,80,&FONT_XS,mute_request?"DRUMS MUTED / FX RETURN":"DRUMS ON / FX MUTE",C_GRAY);
    for(i=0;i<16;i++) {
        unsigned row;
        for(row=0;row<3;row++)
            cv_rect(44+(int)i*12,101+(int)row*16,9,10,
                run_request&&displayed_step==i?C_WHITE:(pattern[row]&(1u<<i)?C_HI:C_LINE));
    }
    for(i=0;i<3;i++)cv_text(2,98+(int)i*16,&FONT_XS,names[i],C_GRAY);
    cv_text(8,153,&FONT_XS,"ARP RUN   K1 BPM   K2 TONE",C_GRAY);
    cv_blit(0,0);
    cv_begin(240,60,C_BLACK);
    for(i=0;i<16;i++)key_cell(4+(int)i*14,14,13,14,WK[i],C_DIM);
    for(i=0;i<11;i++) {
        static const uint8_t AT[11]={0,1,2,4,5,7,8,9,11,12,14};
        key_cell(4+(int)AT[i]*14+8,0,11,12,BK[i],C_LINE);
    }
    octave_text(t);cv_text(4,36,&FONT_XS,"OCT",C_GRAY);cv_text(30,36,&FONT_XS,t,C_HI);
    cv_text(70,36,&FONT_XS,"TONE",C_GRAY);int_text((int)tone_request,t);cv_text(106,36,&FONT_XS,t,C_HI);
    cv_text(150,36,&FONT_XS,"VOL",C_GRAY);int_text((int)(plat_master()*100u/4096u),t);cv_text(180,36,&FONT_XS,t,C_HI);
    cv_text(4,48,&FONT_XS,set.wave?"EXPERIMENTAL VOICE / HOME PANIC":"PRESET PART / HOME PANIC",C_GRAY);
    cv_blit(0,180);
}

void app_ui_init(void)
{
    touch_on(1);                             /* velocity and pressure: the keys' readings on */
    lcd_fill(0, 0, 240, 240, C_BLACK);
    ui.redraw = 1;
}

void app_frame(void)
{
    static uint32_t last_pct, lit_until, last_ble, last_midi;
    uint32_t i, now = plat_ms(), pct = plat_master() * 100u / 4096u;
    input();
    speak();
    { static uint32_t last_step; if(displayed_steps!=last_step) {last_step=displayed_steps;ui.redraw=1;} }
    for (i = 0; i < NE; i++)                 /* an encoder's light goes out */
        if (ui.turned_at[i] && now - ui.turned_at[i] < 400u && now > lit_until)
            lit_until = ui.turned_at[i] + 300u;
    if (lit_until && now >= lit_until) {
        lit_until = 0;
        ui.redraw = 1;
    }
    if (FM1B_BLE_CENTRAL) {                  /* looking: a device found is named at once (REC connects to it) */
        static int found;
        int n = ui.looking ? btdev_count() : 0, ev;
        btdev_page(ui.looking && !set.ble_off);
        if (n > found && btdev_pick() < n)
            ble_device_say();
        found = n;
        while ((ev = btdev_event()) != BTDEV_NONE)   /* a device connected, given up or lost */
            if (ev == BTDEV_CONNECTED || ev == BTDEV_NO_ANSWER || ev == BTDEV_LOST)
                app_say("DEVICE", btdev_word(ev));
    }
    if (midi_count != last_midi) {
        last_midi = midi_count;
        ui.redraw = 1;
    }
    if (pct != last_pct || (uint32_t)(plat_ble_connected() | plat_ble_device() << 1) != last_ble ||
        (ui.msg_until && now >= ui.msg_until)) {
        last_pct = pct;
        last_ble = (uint32_t)(plat_ble_connected() | plat_ble_device() << 1);
        if (now >= ui.msg_until)
            ui.msg_until = 0;
        ui.redraw = 1;
    }
    if (ui.redraw) {
        ui.redraw = 0;
        draw();
    }
}

#ifdef PLAT_HOST
int app_expect(const char *what, int *got)
{
    if (!strcmp(what,"running")) *got=displayed_running;
    else if (!strcmp(what,"tempo")) *got=bpm_request;
    else if (!strcmp(what,"muted")) *got=mute_request;
    else if (!strcmp(what,"step")) *got=displayed_step;
    else if (!strcmp(what,"steps")) *got=displayed_steps;
    else if (!strcmp(what,"tone")) *got=tone_request;
    else if (!strcmp(what, "octave"))
        *got = set.octave;
    else if (!strcmp(what, "wave"))
        *got = set.wave;
    else if (!strcmp(what, "note"))
        *got = voice_note;
    else if (!strcmp(what, "dirty"))
        *got = set_dirty;
    else if (!strcmp(what, "vel"))           /* the last key's velocity and pressure */
        *got = ui.vel[ui.last_key];
    else if (!strcmp(what, "prs"))
        *got = ui.prs[ui.last_key];
    else if (!strncmp(what, "turns", 5))    /* turnsN: encoder N's count (plat.h EN_*) */
        *got = ui.turns[atoi(what + 5) % NE];
    else
        return 0;
    return 1;
}
#endif

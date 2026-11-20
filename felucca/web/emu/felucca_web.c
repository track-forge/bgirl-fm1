/* SPDX-License-Identifier: GPL-3.0-only
 * Copyright (C) 2026 Leo Kuroshita (@kurogedelic), Hügelton Instruments
 * The design (the device clock driven by the audio the worklet asks for, the exports below, the saved
 * flash handed back before boot) follows X0X's browser build, web/emu/x0x_web.c of charlesvestal/fm1-x0x
 * (GPL-3.0), a Felucca fork. */
/* Felucca in the browser: the firmware's sources, as the host tests build them (tests/hostsim.c: libc,
 * engines, voices, FX, sequencer; then the UI, main.c, storage and projects, in src/felucca.c's order),
 * compiled to WebAssembly and run in an AudioWorklet against a simulated FM-1:
 *   - the device clock: TIMER4 ticks (24 MHz) and fm1_ms advance with the audio the worklet asks for; the
 *     audio ISR (audio.c fm1_alnk0_irq, the real one) renders each I2S half (HALF_FRAMES) as the DMA would
 *     ask for it, and the main loop (main.c's, below in web_frame) runs every 15 ms, with ui_input on every
 *     millisecond in between as main.c's wait loop calls it;
 *   - the panel: buttons and encoders by their printed labels (panel.c's table maps them to the matrix),
 *     the keys as fm1_in.notes, the LEDs read back from fm1_led / fm1_led_dim through the key matrix;
 *   - the screen: lcd_fill / lcd_blit into a 240 x 240 RGB565 frame (the panel's byte order);
 *   - the flash: a 1 MiB NOR image behind storage.c's hooks (settings, projects, user presets, the user
 *     sample slots read through it as XIP); the page keeps its storage sectors and gives them back at boot.
 * Not here: USB (MIDI, audio, the editor, updates), TRS MIDI, the serial console, the battery (full), the
 * crash / UBOOT paths. Nothing of the firmware changes for this file.
 *   web/emu/build.sh  ->  build/emu/{felucca.wasm, worklet.js, index.html} */
#include <stdint.h>

#define FELUCCA_WEB 1
#ifndef FELUCCA_CDC
#define FELUCCA_CDC 1                    /* as src/felucca.c: MENU > USB SERIAL is there (nothing behind it) */
#endif
#define FELUCCA_FLASH 1
#define FELUCCA_OTA 0
#define FELUCCA_UAC 0
#define FELUCCA_UART 0
#ifndef FELUCCA_VERSION
#define FELUCCA_VERSION "WEB"
#endif

static uint8_t nor[0x100000];            /* the SPI NOR (erased: 0xFF) */
#define SMP_USER_XIP(k) ((const uint8_t *)nor + SMP_USER_BASE + (k) * SMP_USER_SIZE)   /* eng_sample.c */
#define main hostsim_main
#include "../../tests/hostsim.c"
#undef main

/* ------------------------------------------------------------ the clock --- */
#define FM1_TICKS_PER_US 24u             /* TIMER4: 24 MHz */
static uint32_t web_ticks;
static uint32_t fm1_ticks(void) { return web_ticks; }

/* ------------------------------------------------------ audio (ALNK0) --- */
static uint32_t web_half;                /* the half the next render fills */
#define FM1_AUDIO_HALF 1u
static uint8_t fm1_audio_pending(void) { return FM1_AUDIO_HALF; }
static void fm1_audio_ack_aux(uint8_t p) { (void)p; }
static uint32_t fm1_audio_free_half(void) { return web_half; }
static void fm1_audio_ack_half(void) {}
static void fm1_audio_init(int32_t *buf, uint32_t words, void (*isr)(void), uint32_t prio)
{ (void)buf; (void)words; (void)isr; (void)prio; }
static void fm1_audio_stop(void) {}
void isr_alnk0(void) {}
#include "../../firmware/src/audio.c"

/* ------------------------------------------------------------ the panel --- */
#define FM1_NCOL 11u
static const int8_t FM1_KEYMAP[6][FM1_NCOL] = {      /* as bsp/boards/fm1/fm1_input.h: key id at (column, row bit) */
    {-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    { 5, 11,  4, 10,  3,  9,  2,  8, -1, -1, -1},
    {34, 35, 36, 37, 38, 40, 39, 13,  7,  6, 12},
    {23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33},
    { 0,  1, 15, 14, 17, 16, 19, 18, 20, 21, 22},
    {-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
};
static uint8_t fm1_led[FM1_NCOL], fm1_led_dim[FM1_NCOL], fm1_led_breath[FM1_NCOL], fm1_led_mid[FM1_NCOL];
static uint32_t web_dim_lo;
static void fm1_led_dim_level(uint32_t lo) { web_dim_lo = lo; }
/* the power-on LED sweep (bsp/boards/fm1/fm1_input.h fm1_led_anim_start, the picture bsp/boards/fm1/fm1_led_anim.h): its frame from the
 * clock (a scan frame 1.1 ms), the page draws each LED's level (web_anim_levels); at its end, or on a key or a button
 * down, the last picture (the glow everywhere, or dark) and the UI's LEDs from the next frame, as the scan does */
#include "../../firmware/bsp/boards/fm1/fm1_led_anim.h"
static uint32_t web_anim_t0, web_anim_glow, web_anim_run, web_anim_idle;   /* (idle: 1.2's idle animation, MENU >
                                                                              * ANIM IDLE: fm1_idle_level, endless) */
static uint32_t web_anim_frame(void) { return (web_ticks - web_anim_t0) / (1100u * FM1_TICKS_PER_US); }
static void fm1_led_anim_start(uint32_t glow)
{
    web_anim_idle = 0;
    memset(fm1_led, 0, sizeof fm1_led);
    memset(fm1_led_dim, 0, sizeof fm1_led_dim);
    memset(fm1_led_breath, 0, sizeof fm1_led_breath);
    memset(fm1_led_mid, 0, sizeof fm1_led_mid);
    web_anim_t0 = web_ticks;
    web_anim_glow = glow != 0u;
    web_anim_run = 1;
}
static int fm1_led_anim_on(void)
{
    if (web_anim_run && web_anim_idle && (fm1_in.notes || fm1_in.buttons))
        web_anim_run = 0;                       /* (the idle animation: ended, the UI's LEDs from now) */
    if (web_anim_run && !web_anim_idle && (web_anim_frame() >= FM1_ANIM_FRAMES || fm1_in.notes || fm1_in.buttons)) {
        uint32_t c, r;
        web_anim_run = 0;
        for (c = 0; c < FM1_NCOL; c++)
            for (r = 1; r < 5u; r++)
                if (web_anim_glow && FM1_KEYMAP[r][c] >= 0)
                    fm1_led_dim[c] |= (uint8_t)(1u << r);
    }
    return (int)web_anim_run;
}
static void fm1_led_idle_start(uint32_t glow)
{
    fm1_led_anim_start(glow);
    web_anim_idle = 1;
}
static int fm1_led_idle_on(void) { return fm1_led_anim_on() && web_anim_idle; }
static void fm1_led_anim_stop(void) { web_anim_run = 0; }
static uint32_t web_pressed, web_released, web_notes_pressed;
static int32_t web_enc_steps[7];
static uint32_t fm1_input_edges(uint32_t *released)
{
    uint32_t p = web_pressed;
    if (released)
        *released = web_released;
    web_pressed = web_released = 0;
    return p;
}
static uint32_t fm1_input_note_edges(void) { uint32_t n = web_notes_pressed; web_notes_pressed = 0; return n; }
static int32_t fm1_enc_take(uint32_t e) { int32_t s = web_enc_steps[e % 7u]; web_enc_steps[e % 7u] = 0; return s; }
static void fm1_input_init(void) {}
static void fm1_input_tick(void) {}
static void fm1_wdt_feed(void) {}
static void fm1_irq_off(void) {}
static void fm1_irq_on(void) {}

/* ----------------------------------------------------------- the screen --- */
static uint16_t web_fb[240 * 240];       /* RGB565 as the panel gets it (high byte first) */
static uint32_t web_draws;
static void lcd_fill(uint32_t x, uint32_t y, uint32_t w, uint32_t h, uint16_t c)
{
    uint32_t i, j;
    uint16_t sw = (uint16_t)((c >> 8) | (c << 8));
    for (j = 0; j < h && y + j < 240u; j++)
        for (i = 0; i < w && x + i < 240u; i++)
            web_fb[(y + j) * 240u + x + i] = sw;
    web_draws++;
}
static void lcd_blit(uint32_t x, uint32_t y, uint32_t w, uint32_t h, const uint16_t *p)
{
    uint32_t i, j;
    for (j = 0; j < h && y + j < 240u; j++)
        for (i = 0; i < w && x + i < 240u; i++)
            web_fb[(y + j) * 240u + x + i] = p[j * w + i];
    web_draws++;
}
static void lcd_sync(void) {}
static void lcd_power(uint32_t s)              /* MENU > SCREEN OFF (lcd.c): off, the page dark; on: redrawn before */
{
    if (!s) {
        memset(web_fb, 0, sizeof web_fb);
        web_draws++;
    }
}
static void lcd_wake_now(void) {}
static void lcd_init(void) {}

#include "../../firmware/src/gfx.c"
#include "../../firmware/src/panel.c"
#include "../../firmware/src/ui.c"
#include "../../firmware/src/menu_items.c"
#include "../../firmware/src/icons.c"
#include "../../firmware/src/ui_graph.c"
#include "../../firmware/src/ui_draw.c"
#include "../../firmware/src/ui_menu.c"
#include "../../firmware/src/ui_input.c"
#include "../../firmware/src/ui_layer.c"

/* ------------------------------------------------------------ the flash --- */
static uint32_t web_flash_writes;
static uint8_t flash_ok;
static int st_read(uint32_t off, void *dst, uint32_t n)
{
    if (off >= sizeof nor || n > sizeof nor - off)
        return -1;
    memcpy(dst, nor + off, n);
    return 0;
}
static int st_erase(uint32_t off)
{
    if (off >= sizeof nor || 4096u > sizeof nor - off)
        return -8;
    memset(nor + (off & ~4095u), 0xFF, 4096);
    web_flash_writes++;
    return 0;
}
static int st_prog(uint32_t off, const void *src, uint32_t n)       /* NOR: a write only clears bits */
{
    const uint8_t *s = src;
    uint32_t i;
    if (off >= sizeof nor || n > sizeof nor - off)
        return -8;
    for (i = 0; i < n; i++)
        nor[off + i] &= s[i];
    web_flash_writes++;
    return 0;
}
static uint32_t irq_save(void) { return 0; }
static void irq_restore(uint32_t f) { (void)f; }
static uint32_t fl_jedec_ram(void) { return 0x856014u; }             /* the FM-1's part (project.c) */
static void fl_plain_window_init(void) {}
#define FL_FAR(fn) (fn)
#include "../../firmware/src/storage.c"
#include "../../firmware/src/upreset.c"
#include "../../firmware/src/project.c"

/* ------------------------------------------------- main.c and its HAL --- */
#if FELUCCA_CPU2
typedef struct { uint32_t magic, count, vec, pc, emu, dbg, rets, core; } fm1_crash_t;
#else
typedef struct { uint32_t magic, count, vec, pc, emu, dbg, rets; } fm1_crash_t;
#endif
#define FM1_CRASH_MAGIC 0x43525348u
static fm1_crash_t fm1_crash;                     /* (none: main.c boot_leds runs the sweep) */
static struct { uint8_t p3_rst, wdt_con; uint32_t rst_src; } fm1_boot;
uint32_t _data_start[1], _data_end[1], _data_load[1], _bss_start[1], _bss_end[1];
uint32_t _pool_start[1], _pool_end[1], _rt_start[1], _rt_end[1], _rt_load[1];
static uint32_t web_master_adc = 700;   /* MASTER, 0..1023 as the ADC reads it */
enum { FM1_ADC_BATT = 3, FM1_ADC_MASTER = 4 };
static int32_t fm1_adc_read(uint32_t ch) { return ch == FM1_ADC_MASTER ? (int32_t)web_master_adc : 620; }
static void fm1_adc_init(void) {}
static void fm1_timer5_ack(void) {}
static void fm1_timer5_start(void (*isr)(void), uint32_t prio) { (void)isr; (void)prio; }
void isr_timer5(void) {}
static void fm1_reboot(void) {}
static void fm1_enter_uboot(void) {}
static void fm1_guard_lock_top(void) {}
static void fm1_irq_enable_all(void) {}
static void fm1_irq_init(void) {}
static void fm1_time_init(void) {}
static void fm1_reset_reason(void) {}
static void fm1_wdt_arm(uint32_t v) { (void)v; }
static void fm1_mailbox_clear(void) {}
static void fm1_guard_enable(uint32_t m) { (void)m; }
enum { FM1_GUARD_STACK = 1, FM1_GUARD_WRITE = 2, FM1_GUARD_BUS = 4, FM1_GUARD_PC = 8 };
#if FELUCCA_CPU2
/* the second core (felucca-bsp chip/fm1_cpu1.h): none here, one core whatever MENU > 2ND CORE says (fx.c mix_block's
 * C1_ON() is 0 without felucca.c's macros) */
static uint8_t fm1_cpu1_on, fm1_cpu1_gave_up;
static void fm1_cpu1_stop(void) {}
static int fm1_cpu1_start(void) { return -1; }
#endif
#if FELUCCA_CDC
static void cdc_task(void) {}
#endif
/* main.c's calls into the USB driver (registers): nothing here (usb.c's own functions stay as they are) */
#define usb_poll() ((void)0)
#define usb_start() ((void)0)
#define usb_retry(ms) ((void)(ms))
#define usb_detach() ((void)0)
#include "../../firmware/src/master.c"
#include "../../firmware/src/main.c"
#undef usb_poll
#undef usb_start
#undef usb_retry
#undef usb_detach

/* --------------------------------------------------------- the device --- */
static uint32_t web_booted, web_boot_ms, web_last_frame;

/* main.c fm1_main's boot, up to the main loop (no USB, no UART, no panel setup) */
static void web_power_on(void)
{
    persist_boot();
    settings_init();
    usb_serial_apply();
    lcd_init();
    draw_splash();                        /* main.c: the splash (ui_draw.c) */
    memset(&felucca_dbg, 0, sizeof felucca_dbg);
    felucca_dbg.magic = DBG_MAGIC;
    fm1_input_init();
    fm1_adc_init();
    panel_init();
    felucca_init();
    master_boot();                        /* main.c (#137): the pot's level from the first block */
    kb_boot_hold = 1;                     /* main.c (#137): the keys silent under the splash */
    audio_init();
    boot_leds();                          /* main.c: the power-on LED sweep, from the scan's start */
    autosave_boot(boot_clean);            /* main.c: the last session's music (1.2, project.c) */
    web_boot_ms = fm1_ms + 430u;         /* main.c: 30 + 400 ms before the first frame */
    web_booted = 1;
}

/* one pass of main.c fm1_main's loop (what the browser has: no USB, editor, UBOOT, console) */
static void web_frame(void)
{
    int32_t b = fm1_adc_read(FM1_ADC_BATT);
    if (b > 0)
        song.batt_raw = song.batt_raw ? song.batt_raw + (b - song.batt_raw) / 32 : b;
    master_poll();
    felucca_dbg.ui_frames++;
    ui_input();
    settings_poll();
    autosave_poll();
    ui_leds();
    ui_draw();
}

/* output ring, stereo float, filled by web_ms, drained by web_render */
#define RING 4096u
static float ring[RING * 2];
static uint32_t ring_w, ring_r, web_samples_due;

/* one millisecond of the device: the clock, the audio halves due by now, the main loop */
static void web_ms(void)
{
    fm1_ms++;
    web_ticks += 1000u * FM1_TICKS_PER_US;
    web_samples_due += FS;
    while (web_samples_due >= 1000u * HALF_FRAMES) {      /* an I2S half: the audio ISR renders it */
        uint32_t k;
        const int32_t *o = &abuf[web_half * HALF_WORDS];
        web_samples_due -= 1000u * HALF_FRAMES;
        fm1_alnk0_irq();
        for (k = 0; k < HALF_FRAMES; k++) {
            uint32_t i = (ring_w++ % RING) * 2u;
            ring[i] = (float)o[2u * k] / 8388608.0f;
            ring[i + 1] = (float)o[2u * k + 1u] / 8388608.0f;
        }
        web_half ^= 1u;
    }
    if (!web_booted || (int32_t)(fm1_ms - web_boot_ms) < 0)
        return;
    if (web_boot_ms) {                                    /* main.c: the splash goes, the loop starts */
        web_boot_ms = 0;
        kb_boot_hold = 0;
        lcd_fill(0, 0, 240, 240, T_BG);
        web_last_frame = fm1_ms - 15u;
    }
    if (fm1_ms - web_last_frame >= 15u) {                /* main.c: a frame, then ui_input until 15 ms are over */
        web_last_frame = fm1_ms;
        web_frame();
    } else {
        ui_input();                                       /* main.c's wait loop */
    }
}

#define EXPORT __attribute__((used, visibility("default")))
#undef __attribute__

static float out_l[1024], out_r[1024];
EXPORT float *web_out_l(void) { return out_l; }
EXPORT float *web_out_r(void) { return out_r; }
/* n frames (<= 1024) into out_l / out_r: the device runs until they exist */
EXPORT void web_render(uint32_t n)
{
    uint32_t k;
    if (n > 1024u)
        n = 1024u;
    while (ring_w - ring_r < n)
        web_ms();
    for (k = 0; k < n; k++) {
        uint32_t i = (ring_r++ % RING) * 2u;
        out_l[k] = ring[i];
        out_r[k] = ring[i + 1];
    }
}

/* the flash: erased, then the page's saved sectors (web_nor), then boot */
EXPORT uint8_t *web_nor(void) { return nor; }
EXPORT uint32_t web_nor_size(void) { return sizeof nor; }
EXPORT void web_nor_erase(void) { memset(nor, 0xFF, sizeof nor); }
EXPORT uint32_t web_flash_writes_count(void) { return web_flash_writes; }
EXPORT void web_boot(void)
{
    if (!web_booted)
        web_power_on();
}

/* input, by the printed labels: buttons bit b = B_FX .. B_OCTUP (panel.c), keys bit k = key k (F3 .. G5),
 * encoders by role (EN_SELECT .. EN_K4), MASTER 0..1023 */
static uint32_t web_btn_labels, web_key_mask;
EXPORT void web_buttons(uint32_t labels)
{
    uint32_t m = 0, b, old = fm1_in.buttons;
    web_btn_labels = labels;
    for (b = 0; b < NB; b++)
        if ((labels >> b) & 1u)
            m |= 1u << panel.btn[b];
    web_pressed |= m & ~old;
    web_released |= old & ~m;
    fm1_in.buttons = m;
}
EXPORT void web_keys(uint32_t mask)
{
    uint32_t old = fm1_in.notes;
    mask &= (1u << 27) - 1u;
    web_key_mask = mask;
    web_notes_pressed |= mask & ~old;
    fm1_in.notes = mask;
}
EXPORT void web_enc(uint32_t role, int32_t n)
{
    if (role < NE)
        web_enc_steps[panel.enc[role] % 7u] += n * panel.dir[role];
}
EXPORT void web_master(uint32_t v) { web_master_adc = v > 1023u ? 1023u : v; }
/* a channel message from Web MIDI, as a USB-MIDI packet arrives (usb.c midi_enqueue): 0 = the ring was full */
EXPORT uint32_t web_midi(uint32_t status, uint32_t d1, uint32_t d2)
{
    if (status < 0x80u || status >= 0xF0u)
        return 1;
    return (uint32_t)midi_enqueue((status >> 4) | status << 8 | (d1 & 127u) << 16 | (d2 & 127u) << 24, 1);
}

/* the LEDs: bit b = the button labelled b lit (bit 14: PLAY's green), keys bit k; the dim glow and the breath alike */
static uint32_t led_on(const uint8_t *l, uint32_t id)
{
    uint32_t c, r;
    for (c = 0; c < FM1_NCOL; c++)
        for (r = 1; r < 5u; r++)
            if (FM1_KEYMAP[r][c] == (int8_t)id)
                return (l[c] >> r) & 1u;
    return 0;
}
static uint32_t led_buttons(const uint8_t *l)
{
    uint32_t b, m = 0;
    for (b = 0; b < NB; b++)
        m |= led_on(l, panel.btn[b]) << b;
    return m;
}
static uint32_t led_keys(const uint8_t *l)
{
    uint32_t k, m = 0;
    for (k = 0; k < 27u; k++)
        m |= led_on(l, 14u + k) << k;
    return m;
}
EXPORT uint32_t web_lit_buttons(void)
{
    return led_buttons(fm1_led) | ((fm1_led[LED_PLAY_GREEN >> 3] >> (LED_PLAY_GREEN & 7u)) & 1u) << 14;
}
EXPORT uint32_t web_lit_keys(void) { return led_keys(fm1_led); }
EXPORT uint32_t web_dim_buttons(void) { return led_buttons(fm1_led_dim); }
EXPORT uint32_t web_dim_keys(void) { return led_keys(fm1_led_dim); }
EXPORT uint32_t web_dim_level(void) { return web_dim_lo; }
EXPORT uint32_t web_breath_buttons(void) { return led_buttons(fm1_led_breath); }   /* #119: dark .. ~60 % of lit */
EXPORT uint32_t web_breath_keys(void) { return led_keys(fm1_led_breath); }
EXPORT uint32_t web_mid_keys(void) { return led_keys(fm1_led_mid); }   /* the DRUM grid's beats: a steady mid level */
/* the power-on sweep (or 1.2's idle animation) running: each LED's level now (bsp/boards/fm1/fm1_led_anim.h: 0..64 /64 of lit, | 0x80 the glow under
 * it), 0..13 the buttons by label, 14..40 the keys F3..G5; 0 when it is not */
EXPORT uint8_t *web_anim_levels(void)
{
    static uint8_t lv[FM1_ANIM_NBTN + FM1_ANIM_NKEY];
    uint32_t f = web_anim_frame(), i;
    if (!fm1_led_anim_on())
        return 0;
    for (i = 0; i < FM1_ANIM_NBTN + FM1_ANIM_NKEY; i++)
        lv[i] = (uint8_t)(web_anim_idle ? fm1_idle_level : fm1_anim_level)(f, i < FM1_ANIM_NBTN ? (i < NB ? panel.btn[i] : i) : i,
                                                                          web_anim_glow);
    return lv;
}

/* for the test and the bench (web/emu/emu_test.mjs), a heavy song straight into the tracks (as hostsim's
 * renders set them): T1 FM6 PAD, T2 PHYS DRONE STRING (SYMP), T3 GRAIN CLOUD PAD, 4-note chords on all 16
 * steps, held; T4 DRUM, kick snare and hats. PLAY is the page's (or the test's) to press */
EXPORT void web_test_heavy(void)
{
    static const uint8_t ENG[3][2] = {{12, 4}, {ENGI_PHYS, 7}, {8, 0}};
    static const uint8_t CH[4][4] = {{48, 55, 60, 64}, {45, 52, 57, 60}, {41, 48, 53, 57}, {43, 50, 55, 59}};
    uint32_t p, i;
    for (p = 0; p < 3u; p++) {
        track_t *t = &trk[p];
        host_preset(t, ENG[p][0], ENG[p][1]);
        t->p[P_VOICE] = V_POLY;
        t->p[P_SUS] = 127;
        for (i = 0; i < 16u; i++) {
            uint8_t n[4], k;
            for (k = 0; k < 4u; k++)
                n[k] = (uint8_t)(CH[i / 4u][k] + 12u * p);
            put_step(t, i, 4, n, ST_NOTE, 0);
        }
    }
    host_drums(&trk[3]);
    for (i = 0; i < 16u; i++) {
        uint8_t n[4];
        uint32_t k = 0;
        if (i % 4u == 0u)
            n[k++] = 36;
        if (i == 4u || i == 12u)
            n[k++] = 38;
        n[k++] = i % 2u ? 42 : 46;
        put_step(&trk[3], i, k, n, ST_NOTE, 0);
    }
    ui.force = 1;
}

EXPORT uint32_t web_test_user_preset(uint32_t i) { return (uint32_t)up_used(i); }

EXPORT uint16_t *web_screen(void) { return web_fb; }
EXPORT uint32_t web_screen_draws(void) { return web_draws; }
EXPORT uint32_t web_now_ms(void) { return fm1_ms; }
EXPORT uint32_t web_playing(void) { return song.playing; }
EXPORT uint32_t web_sample_rate(void) { return FS; }

/* BGIRL explicit reset: use upstream project_new (stops, releases, clears the
 * project), never touch saved slots, and refuse while physical/panel notes held. */
EXPORT uint32_t web_bgirl_project(uint32_t demo)
{
    if (fm1_in.notes || fm1_in.buttons) return 1;
    if (project_new()) return 1;
    name_close();ui.confirm=0;ui.menu=0;ui.lock=ui.ly=ui.layer=0;go_home();
    {
        uint32_t i;
        undo_depth++;
        for (i = 0; i < NTRK; i++) {
            set_engine_of(&trk[i], BGIRL_TRK_DEF[i][0]);
            apply_preset_to(&trk[i], BGIRL_TRK_DEF[i][1]);
        }
        undo_depth--;
    }
    if (demo) {
        uint32_t i;
        for (i = 1; i < 3u; i++)
            load_pat16(&trk[i], PATTERNS[BGIRL_TRK_DEF[i][2] - 1u].note,
                       PATTERNS[BGIRL_TRK_DEF[i][2] - 1u].flags);
        bgirl_demo_steps(2); /* Named Mega Jam: original TIGHT kit. */
    }
    ui.home = 1;
    ui.force = 1;
    return 0;
}

/* SH-STYLE is an additive ANALOG collection: exactly the normal factory loader. */
EXPORT uint32_t web_bgirl_sh_style_sound(uint32_t sound) {
    if (sound >= 2 || fm1_in.notes || fm1_in.buttons) return 1;
    set_engine_of(TSEL, 0);
    apply_preset_to(TSEL, 13 + sound);
    ui.force = 1;
    return 0;
}
/* Read-only state for browser/host acceptance; no alternate editing path. */
EXPORT int32_t web_bgirl_sh_style_info(uint32_t track, uint32_t field) {
    const track_t *t = &trk[track % NTRK];
    static const uint8_t ids[] = {P_E4,P_E5,P_ATK,P_DEC,P_SUS,P_REL,P_LD_FLT,P_ED_FLT,P_VOICE};
    if (field == 0) return t->eng_req;
    if (field == 1) return t->preset;
    if (field == 11) return song.sel;
    return field >= 2 && field < 11 ? t->p[ids[field-2]] : -1;
}

/* Normal factory selections, also reachable through PRESETS / SIGNATURE. */
static const uint8_t BG_SIGNATURE_SELECT[10][2] = {
    {12,8},{12,9},{10,4},{10,5},{10,6},{10,7},{10,8},{12,10},{12,11},{12,12}
};
EXPORT uint32_t web_bgirl_signature_sound(uint32_t sound) {
    if (sound>=10 || fm1_in.notes || fm1_in.buttons) return 1;
    track_t *t=TSEL;
    set_engine_of(t,BG_SIGNATURE_SELECT[sound][0]);
    apply_preset_to(t,BG_SIGNATURE_SELECT[sound][1]);
    ui.force=1;
    return 0;
}
/* Explicit project operations only: startup/autosave/vocal demo untouched.
 * mode0 blank standalone template, 1 retained-kit context, 2 standalone groove,
 * 3 cross-track choke exercise. All load stopped. */
EXPORT uint32_t web_bgirl_signature(uint32_t mode) {
    static const uint8_t sounds[4][4]={{2,4,5,0},{255,1,7,8},{2,4,5,0},{5,6,2,1}};
    if (mode>3 || web_bgirl_project(0)) return 1;
    undo_depth++;
    for(uint32_t t=0;t<4;t++) {
        uint32_t v=sounds[mode][t];
        set_engine_of(&trk[t],v==255?10:BG_SIGNATURE_SELECT[v][0]);
        apply_preset_to(&trk[t],v==255?2:BG_SIGNATURE_SELECT[v][1]);
        if(v==255)trk[t].p[P_LEVEL]=100;
        trk[t].p[P_CHRD]=v==8?CH_MIN:CH_OFF;
        trk[t].p[P_SLEN]=16;
        memset(trk[t].step,0,sizeof trk[t].step);
    }
    undo_depth--;
    if(mode) for(uint32_t t=0;t<4;t++) for(uint32_t k=0;k<16;k++) {
        step_t *st=&trk[t].step[k];uint32_t note=60,hit=0;
        if(mode==1) {
            if(t==0) { st->hit=(k%4==0?1:0)|(k%8==4?6:0)|(k%2==0?8:0);hit=st->hit; }
            if(t==1) { hit=k%4==2 || k==15;note=k==15?43:36; }
            if(t==2) { hit=k==3 || k==11;note=k==3?60:67; }
            if(t==3) { hit=k==6 || k==14; note=k==6?60:65; }
        } else if(mode==2) {
            hit=t==0?(k%4==0 || k==11):t==1?k%8==4:t==2?k%2==0:k%4==2;
            note=t==3?(k==14?43:36):60;
        } else {
            hit=t==0?(k==4 || k==8 || k==12):t==1?(k==0 || k==4 || k==10):t==2?k%4==0:k%4==2;
            note=t==3?36:60;
        }
        if(!hit)continue;
        st->time=ST_NOTE;st->vel=k%8==0?110:90;
        if(!st->hit) { st->n=1;st->note[0]=(uint8_t)note; }
        /* Sequence stores final chord tones; live root expansion remains CHRD MIN/+OCT. */
        if(mode==1 && t==3) { st->n=4;st->note[1]=note+3;st->note[2]=note+7;st->note[3]=note-12; }
    }
    song.sel=0;ui.home=1;ui.force=1;
    return 0;
}

/* Read-only inspection for host/browser parity checks, no alternate loading. */
EXPORT uint32_t web_bgirl_signature_info(uint32_t track, uint32_t field) {
    const track_t *t=&trk[track%NTRK];
    switch(field) {
    case 0:return t->eng_req;
    case 1:return t->preset;
    case 2:return t->p[P_E0];
    case 3:return t->p[P_LEVEL];
    case 4:return t->p[P_CHRD];
    case 5:{uint32_t hits=0;for(uint32_t i=0;i<NSTEP;i++)hits+=!!(t->step[i].n || t->step[i].hit);return hits;}
    default:return song.playing;
    }
}

/* Existing isolated ANALOG comparison remains reproducible; the live wet buttons
 * retain electro drums/bass and use the factory TRIO / FAT BASS carrier. */
static uint32_t bgirl_vocoder_setup(uint32_t mode, int pad_demo)
{
    uint32_t k;
    if(web_bgirl_project(0)) return 1;
    if(pad_demo) {
        for(k=1;k<3;k++)load_pat16(&trk[k],PATTERNS[BGIRL_TRK_DEF[k][2]-1u].note,PATTERNS[BGIRL_TRK_DEF[k][2]-1u].flags);
        bgirl_legacy_demo_steps();
        trk[3].p[P_VBANDS]=mode==1 ? 0 : 1;
        song.sel=3; ui.home=1; ui.force=1; bg_vreset++;
        return 0;
    }
    undo_depth++;
    set_engine_of(&trk[1],0); apply_preset_to(&trk[1],0);
    set_engine_of(&trk[3],13); apply_preset_to(&trk[3],0);
    undo_depth--;
    for(k=0;k<NTRK;k++) {
        trk[k].p[P_CHOR]=trk[k].p[P_DLY]=trk[k].p[P_REV]=0;
        trk[k].p[P_DIST]=0;
    }
    trk[1].p[P_E4]=127; trk[1].p[P_E5]=0; trk[1].p[P_E7]=0;
    trk[1].p[P_SUS]=127; trk[1].p[P_ATK]=0;
    trk[1].p[P_ED_FLT]=0; trk[1].p[P_DEC]=0; trk[1].p[P_REL]=20;
    trk[1].p[P_SGATE]=120; trk[1].p[P_LEVEL]=90;
    trk[3].p[P_E0]=SLC_SRC_SPEECH; trk[3].p[P_E1]=SLC_DIV_AUTO;
    trk[3].p[P_E4]=SLC_ONE; trk[3].p[P_E6]=127; trk[3].p[P_ATK]=0;
    trk[3].p[P_SUS]=127; trk[3].p[P_DEC]=0; trk[3].p[P_LEVEL]=96;
    trk[3].p[P_VCARR]=mode ? 2 : 0; trk[3].p[P_VMON]=0;
    trk[3].p[P_VBANDS]=mode==1 ? 0 : 1;
    trk[3].p[P_VCONS]=0;
    trk[3].p[P_SLEN]=64;
    trk[1].p[P_SLEN]=64;
    memset(trk[3].step,0,sizeof trk[3].step); /* replace old VOX reply */
    for(k=0;k<64;k++) {
        if(k%4==0) { uint8_t note[4]={(uint8_t)(48+(k/4%4)*3),0,0,0}; put_step(&trk[1],k,1,note,ST_NOTE,0); }
        if(k%16==0) { uint8_t note[4]={(uint8_t)(60+k/16),0,0,0}; put_step(&trk[3],k,1,note,ST_NOTE,0); }
    }
    if(!mode) trk[1].p[P_MUTE]=1;
    song.sel=3; ui.home=1; ui.force=1; bg_vreset++;
    return 0;
}

/* Explicit, stopped audition project; never overrides restored music on boot.
 * mode: whole dry/wet, chops dry/wet. group is the stable parent view 1..6. */
EXPORT uint32_t web_bgirl_vocals(uint32_t mode, uint32_t group)
{
    if(mode>3 || group<1 || group>=NELEM(BG_VOCAL_GROUPS)) return 1;
    if(web_bgirl_project(0)) return 1;
    bgirl_legacy_demo_steps();
    fm1_irq_off();
    undo_depth++;
    set_engine_of(&trk[3],13); apply_preset_to(&trk[3],0);
    undo_depth--;
    trk[0].p[P_MUTE]=trk[1].p[P_MUTE]=1;
    trk[2].p[P_MUTE]=!(mode&1);
    trk[3].p[P_E0]=SLC_SRC_VOCALS;
    trk[3].p[P_E1]=mode<2 ? 0 : (int16_t)group;
    trk[3].p[P_E2]=0; trk[3].p[P_ROOT]=0;
    trk[3].p[P_E3]=0; trk[3].p[P_E4]=SLC_ONE;
    trk[3].p[P_E5]=0; trk[3].p[P_E6]=127;
    trk[3].p[P_ATK]=0; trk[3].p[P_DEC]=0; trk[3].p[P_SUS]=127;
    trk[3].p[P_VCARR]=(mode&1) ? 3 : 0;
    trk[3].p[P_VBANDS]=1; trk[3].p[P_VMON]=0;
    trk[3].p[P_CHOR]=trk[3].p[P_DLY]=trk[3].p[P_REV]=0;
    slc_vocal_select(&trk[3]);
    memset(trk[3].step,0,sizeof trk[3].step);
    if(mode<2) {
        uint8_t note[4]={(uint8_t)(60+group-1),0,0,0};
        put_step(&trk[3],0,1,note,ST_NOTE,0);
    } else {
        uint32_t count=BG_VOCAL_GROUPS[group].n;
        for(uint32_t k=0;k<64;k+=4) {
            uint8_t note[4]={(uint8_t)(60+(k/4)%count),0,0,0};
            put_step(&trk[3],k,1,note,ST_NOTE,0);
        }
    }
    song.sel=3; song.octave=0; ui.home=1; ui.force=1; bg_vreset++;
    fm1_irq_on();
    return 0;
}

/* Independent, editable auditions. No startup/autosave hook. All synthesis is
 * through normal engines, tracks, sequencer and the existing two-input vocoder.
 * mode0 whole,1 chops,2 arrangement,3 carrier-only,4 dry whole.
 * carrier0 saw,1 historical GRAIN (offline comparisons),2 factory TRIO FAT BASS. */
EXPORT uint32_t web_bgirl_clarity(uint32_t mode,uint32_t bands,uint32_t release,uint32_t carrier,uint32_t wet)
{
    if(mode>4 || bands>2 || release>127 || carrier>2 || wet>127) return 1;
    uint32_t group=1;
    for(uint32_t i=1;i<NELEM(BG_VOCAL_GROUPS);i++)
        if(!strcmp(BG_VOCAL_VIEWS[i],"G6")) group=i;
    if(web_bgirl_vocals(mode==1 ? 3 : 1,group))return 1;
    fm1_irq_off();undo_depth++;
    if(!carrier) {
        set_engine_of(&trk[2],0);apply_preset_to(&trk[2],NELEM(ANALOG_PRESETS));
    } else if(carrier==1) {
        /* Preserve existing recorded-comparison API identity; not a live UI choice. */
        set_engine_of(&trk[2],8);apply_preset_to(&trk[2],0);
    }
    trk[2].p[P_LEVEL]=90;
    trk[2].p[P_SLEN]=64;trk[2].p[P_SDIV]=2;
    trk[3].p[P_SLEN]=64;trk[3].p[P_SDIV]=2;
    trk[3].p[P_VBANDS]=bands;trk[3].p[P_VRANGE]=1;trk[3].p[P_VREL]=release;
    trk[3].p[P_VWET]=wet;trk[3].p[P_VCONS]=0;trk[3].p[P_VMON]=mode==3;
    for(uint32_t t=0;t<NTRK;t++) {
        trk[t].p[P_CHOR]=trk[t].p[P_DLY]=trk[t].p[P_REV]=trk[t].p[P_DIST]=0;
        if(t!=2 && t!=3) { trk[t].p[P_MUTE]=1; memset(trk[t].step,0,sizeof trk[t].step); }
    }
    memset(trk[2].step,0,sizeof trk[2].step);
    for(uint32_t k=0;k<64;k++) {
        step_t *s=&trk[2].step[k];s->time=k ? ST_TIE : ST_NOTE;
        if(!k) {s->n=1;s->note[0]=46;s->vel=94;}
    }
    /* New complete four-bar arrangement: whole hook then answering chops.
     * Carrier's melody is independent; drums/bass leave phrase space. */
    if(mode==2) {
        for(uint32_t t=0;t<2;t++) {trk[t].p[P_MUTE]=0;trk[t].p[P_SLEN]=64;trk[t].p[P_LEVEL]=t?72:84;}
        for(uint32_t k=0;k<64;k++) {
            step_t *d=&trk[0].step[k];d->time=ST_NOTE;d->vel=96;
            d->hit=(k%16==0 || k%16==10 ? 1 : 0) | (k%16==4 || k%16==12 ? 6 : 0) | (k%4==2 ? 8 : 0);
            if(k%8==0) {uint8_t note[4]={(uint8_t)(34+(k/16%2)*5),0,0,0};put_step(&trk[1],k,1,note,ST_NOTE,0);}
            if(k==16 || k==32 || k==48) {trk[2].step[k].time=ST_NOTE;trk[2].step[k].n=1;trk[2].step[k].note[0]=46+(k/16%2)*7;trk[2].step[k].vel=94;}
        }
        /* Ordinary motion changes the bank view for the answering chops. */
        trk[3].p[P_E1]=0;slc_vocal_select(&trk[3]);
        memset(trk[3].step,0,sizeof trk[3].step);
        uint8_t hook[4]={(uint8_t)(60+group-1),0,0,0};put_step(&trk[3],0,1,hook,ST_NOTE,0);
        motion_set_event(&trk[3],0,P_E1,0);
        motion_set_event(&trk[3],32,P_E1,group);
        motion_set_enabled(&trk[3],1);
        for(uint32_t k=32;k<64;k+=6) {
            uint8_t answer[4]={(uint8_t)(60+(k/6)%BG_VOCAL_GROUPS[group].n),0,0,0};
            put_step(&trk[3],k,1,answer,ST_NOTE,0);
        }
    }
    if(mode==3) {memset(trk[3].step,0,sizeof trk[3].step);trk[3].p[P_LEVEL]=0;}
    if(mode==4) {trk[3].p[P_VCARR]=0;trk[2].p[P_MUTE]=1;}
    undo_depth--;song.sel=3;song.playing=0;transport_req=0;ui.home=1;ui.force=1;bg_vreset++;
    fm1_irq_on();return 0;
}
/* Disposable flyer uses normal tracks, transport and sample-fed vocoder.
 * Its page never restores or persists flash. No studio startup hook. */
EXPORT uint32_t web_bgirl_flyer(uint32_t action)
{
    if(action==0) {
        if(web_bgirl_clarity(2,2,80,0,127))return 1;
        memset(trk[3].step,0,sizeof trk[3].step);
        motion_set_enabled(&trk[3],0);
        transport_req=1;
    } else if(action==1) {
        if(!song.playing)return 1;
        uint32_t group=1;
        for(uint32_t i=1;i<NELEM(BG_VOCAL_GROUPS);i++)
            if(!strcmp(BG_VOCAL_VIEWS[i],"G6"))group=i;
        trk_note_on(&trk[3],60+group-1,100);
    } else if(action==2) {
        seq_stop();transport_req=0;
        for(uint32_t t=0;t<NTRK;t++)trk_all_off(&trk[t]);
    } else return 1;
    return 0;
}

EXPORT uint32_t web_bgirl_clarity_info(uint32_t field) {
    switch(field) {
    case 0:return trk[3].p[P_VBANDS];case 1:return trk[3].p[P_VRANGE];
    case 2:return trk[3].p[P_VREL];case 3:return trk[3].p[P_VWET];
    case 4:return trk[2].eng_req;case 5:return song.playing;
    case 6:return (uint32_t)(BG_REL_MS[clamp(trk[3].p[P_VREL],0,127)]+.5f);
    case 7:return trk[3].p[P_VHOLD];case 8:
#if BGIRL_VOCODER
        return bg_voc.hold;
#else
        return 0;
#endif
    case 9:return trk[2].preset;
    default:return 0;
    }
}

/* Small read-only status word for bank-info/browser diagnostics; no render state edits. */
EXPORT uint32_t web_bgirl_vocal_state(void) {
    return trk[3].eng_req==13 ? ((uint32_t)trk[3].p[P_E0]&15u) |
        (((uint32_t)trk[3].p[P_E1]&7u)<<4) | (((uint32_t)trk[3].p[P_E2]&31u)<<7) |
        (slc_vocal_match(&trk[3]) ? 4096u : 0) : 0;
}
/* Public group label can differ from this bank's packed VIEW index. */
EXPORT uint32_t web_bgirl_vocal_group_number(uint32_t view) {
    return view>0 && view<NELEM(BG_VOCAL_GROUPS) ? (uint32_t)(BG_VOCAL_VIEWS[view][1]-'0') : 0;
}
EXPORT uint32_t web_bgirl_vocal_groups(void) { return NELEM(BG_VOCAL_GROUPS)-1; }
EXPORT const uint8_t *web_bgirl_vocal_id(void) { return BG_VOCAL_ID; }

EXPORT uint32_t web_bgirl_vocoder(uint32_t mode)
{
    return bgirl_vocoder_setup(mode, mode!=0);
}

/* Read-only browser acceptance/agent diagnostics; never changes music or drawing. */
EXPORT uint32_t web_bgirl_crew_info(uint32_t field)
{
    switch(field) {
    case 0:return ui_home_view;
    case 1:return ui.home && home_crew() && !ui.menu && !ui.layer && !ui.confirm && !name_on();
    case 2:return song.sel;
    case 3:return crew_read(song.sel);
    case 4:return crew_moves[song.sel];
    case 5:return crew_graph_paints;
    case 6:return song.playing;
    case 7:return TSEL->p[P_MUTE];
    case 8:return home_levels();
    case 9:return ui.menu_sel;
    case 10:return !!(ui_prefs&PREF_LARGE);
    case 11:return palette_cur();
    case 12:return ui_style;
    default:return 0;
    }
}

EXPORT uint32_t web_bgirl_dance_info(uint32_t field)
{
    switch(field) {
    case 0: return dance_read(song.sel);
    case 1: return !ui.home && cur_page()->graph==GR_DANCE;
    case 2: return dance_style;
    case 3: return song.sel;
    case 4: return TSEL->p[P_AMODE];
    case 5: return TSEL->p[P_ARATE];
    case 6: return TSEL->p[P_APROB];
    case 7: return TSEL->p[P_AHOLD];
    case 8: return TSEL->nheld;
    case 9: return song.playing;
    default: return 0;
    }
}

/* Explicit editable six-track fixtures. Never startup/autosave replacement. */
EXPORT uint32_t web_bgirl_six_project(uint32_t dense)
{
    static const uint8_t sounds[6]={2,3,5,1,7,9};
    if(dense>1 || web_bgirl_project(0))return 1;
    song.g[G_ROUTE]=ROUTE_TRACK;song.g[G_BPM]=120;
    undo_depth++;
    for(uint32_t t=0;t<NTRK;t++) {
        set_engine_of(&trk[t],BG_SIGNATURE_SELECT[sounds[t]][0]);
        apply_preset_to(&trk[t],BG_SIGNATURE_SELECT[sounds[t]][1]);
        trk[t].p[P_SLEN]=64;trk[t].p[P_SDIV]=2;trk[t].p[P_CHRD]=CH_OFF;
        trk[t].p[P_LEVEL]=t<3?94:78;
        track_defaults_steps(&trk[t]);trk[t].p[P_SLEN]=64;
        for(uint32_t k=0;k<NSTEP;k++) {
            step_t *st=&trk[t].step[k];
            uint32_t hit=t==0?k%4==0:t==1?k%8==4:t==2?k%2==0:t==3?k%4==2:t==4?k%16==3:k%16==14;
            if(!dense && !hit)continue;
            st->time=ST_NOTE;st->n=dense?4:1;st->vel=100;
            st->note[0]=t==3?(k%16==14?43:36):t==4?60:72;
            if(dense){st->note[1]=64;st->note[2]=67;st->note[3]=72;st->flags=255;st->hit=st->acc=255;st->probability=100;}
        }
    }
    undo_depth--;song.sel=0;ui_home_view=HV_TRACKS;ui.home=1;ui.force=1;
    ui_message(dense?"DENSE: PROJECT FULL":"SIX TRACK ELECTRO");return 0;
}
EXPORT uint32_t web_bgirl_six_info(uint32_t field,uint32_t t)
{
    if(field==0)return NTRK;
    if(field==1)return NVOICE;
    if(field==2)return song.sel;
    if(field==3)return project_bytes();
    if(field==4)return as.writes;
    if(field==5)return as.full_blocked;
    if(field==6)return proj_rd(proj_slot[0].raw,4)==PROJ_MAGIC;
    if(field==7)return song.playing;
    if(field==8)return mix_base();
    if(t>=NTRK)return 0xFFFFFFFFu;
    if(field==9)return trk[t].eng_req;
    if(field==10)return trk[t].p[P_LEVEL];
    if(field==11)return trk[t].p[P_SLEN];
    if(field==12)return trk[t].p[P_MUTE];
    if(field==13){uint32_t n=0;for(uint32_t i=0;i<NVOICE;i++)n+=trk[t].v[i].active && trk[t].v[i].stage!=4;return n;}
    return 0xFFFFFFFFu;
}

#include "song_snapshot.c"

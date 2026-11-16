/* SPDX-License-Identifier: GPL-3.0-only */
#include "../felucca/web/emu/felucca_web.c"
#include <assert.h>
#include <stdio.h>
static void advance(uint32_t ms) { for(uint32_t n=0;n<ms*441u/10u;n+=128) web_render(128); }
static uint32_t page_graph(uint32_t g) { for(uint32_t i=0;i<NPAGES;i++) if(PAGES[i].graph==g) return i; assert(0); return 0; }
static void reset_track(track_t *t) { trk_all_off(t); t->nheld=t->arp_phys=0; t->p[P_AMODE]=AM_UP;t->p[P_APROB]=127;t->p[P_AOCT]=1;t->arp_idx=0; }
static void composition(void) {
    ui.home=0;ui.page=page_graph(GR_DANCE);song.sel=0;
    for(uint32_t theme=0;theme<NPALETTES*2;theme++) {
        /* Exercise FLAT/LINE and color/mono by existing palette machinery. */
        settings.palette=theme/2;palette_set(theme/2);ui_style=theme&1u;style_apply();
        for(uint32_t cur=0;cur<12;cur++) for(uint32_t ghost=0;ghost<2;ghost++) {
            dance_reset(0); for(uint32_t j=0;j<cur+2;j++) dance_trigger(0);
            dance_style=ghost;ui.force=1;draw_graph();ui.force=0;
            uint32_t s=dance_read(0),p=s&15u,prev=(s>>4)&15u;
            for(uint32_t y=0;y<96;y++)for(uint32_t x=0;x<96;x++) {
                uint32_t bit=128u>>(x&7u),off=y*12+x/8;
                uint16_t expected=bg_dance_masks[p][off]&bit ? ACC : ghost && (bg_dance_masks[prev][off]&bit) ? T_DIM : T_SURF;
                assert(web_fb[(Y_GRAPH+22+y)*240+72+x]==swap16(expected));
            }
            uint32_t draws=web_draws;ui.frame+=70;draw_graph();assert(web_draws==draws);
        }
    }
    dance_reset(0);dance_style=1;ui.force=1;draw_graph();ui.force=0;
    assert(!(dance_snapshot&256u));
    ui_prefs|=PREF_LARGE;assert(large_kind()==LK_LABEL && graph_h()==122);ui_prefs&=~PREF_LARGE;
    ui.page=page_graph(GR_ADSR);ui.force=1;draw_graph();ui.force=0;
    uint32_t draws=web_draws;
    for(uint32_t k=0;k<25;k++){dance_trigger(0);draw_graph();}
    assert(web_draws==draws); /* hidden dance causes no extra graph drawing */
    ui.page=page_graph(GR_DANCE);draw_graph();assert(dance_snapshot==dance_read(0));
    song.sel=1;draw_graph();assert(dance_snapshot==dance_read(1));
    puts("dance pixels: 528 compositions (11 palettes x FLAT/LINE x 12 poses x 2 styles) incl. wrap, CRISP erasure, GHOST intersection/previous, style redraw, LARGE, no hidden/idle drawing PASS");
}
int main(int argc,char **argv) {
    web_nor_erase();web_boot();advance(1200);
    if(argc>1) { /* fresh process each run: identical engine/RNG, actual UI alongside busiest demo */
        int style=atoi(argv[1]);
        if(style>=0){ui.home=0;ui.page=page_graph(GR_DANCE);dance_style=style;}
        else {ui.home=0;ui.page=page_graph(GR_ADSR);}
        trk[0].p[P_AMODE]=AM_RND;trk[0].p[P_ARATE]=5;trk[0].p[P_APROB]=73;
        trk[0].p[P_ASWING]=37;arp_add(&trk[0],60);arp_add(&trk[0],64);arp_add(&trk[0],67);seq_start();
        for(uint32_t k=0;k<4000;k++) {web_render(128);fwrite(out_l,sizeof(float),128,stdout);fwrite(out_r,sizeof(float),128,stdout);fwrite(&rng_state,4,1,stdout);fwrite(&trk[0].arp_note,1,1,stdout);}
        return 0;
    }
    track_t *t=&trk[0];reset_track(t);
    arp_fire(t,1000);assert(!dance_read(0));
    arp_add(t,60);arp_fire(t,1000);assert(dance_read(0)==512u);
    arp_fire(t,1000);assert(dance_read(0)==(512u|256u|1u));
    for(uint32_t i=0;i<25;i++)arp_fire(t,1000);
    assert((dance_read(0)&15u)==2u && ((dance_read(0)>>4)&15u)==1u);
    uint32_t state=dance_read(0),r=rng_state;dance_trigger(1);assert(dance_read(0)==state && rng_state==r);
    arp_off(t);assert(dance_read(0)==state);
    t->p[P_AMODE]=0;arp_tick(t,999999);assert(dance_read(0)==state);
    t->p[P_AMODE]=AM_UP;t->p[P_APROB]=0;uint32_t rejected=0,accepted=0;
    for(uint32_t i=0;i<1024;i++) {uint32_t s=dance_read(0);arp_fire(t,1000);if(dance_read(0)==s)rejected++;else accepted++;}
    assert(rejected>900 && accepted>0); /* inherited <= probability: zero still occasionally succeeds */
    reset_track(t);t->p[P_AMODE]=AM_CHORD;for(uint32_t i=0;i<4;i++)arp_add(t,60+i*2);
    arp_fire(t,1000);assert(dance_read(0)==512);arp_fire(t,1000);assert((dance_read(0)&15u)==1u);
    t->p[P_AHOLD]=1;for(uint32_t i=0;i<4;i++)arp_remove(t,60+i*2);assert(t->nheld==4);arp_fire(t,1000);assert((dance_read(0)&15u)==2u);
    t->p[P_AHOLD]=0;reset_track(t);arp_add(t,60);arp_tick(t,32);assert(dance_read(0)==512);
    song.playing=0;arp_tick(t,div_samples(t->p[P_ARATE]));assert((dance_read(0)&15u)==1u); /* internal arp without PLAY */
    arp_remove(t,60);state=dance_read(0);arp_tick(t,99999);assert(dance_read(0)==state);
    trk_all_off(t);assert(!dance_read(0));arp_add(t,60);arp_fire(t,1000);assert(dance_read(0)==512);
    panic_req=1;events_block(32);assert(!dance_read(0));
    dance_trigger(0);project_new();assert(!dance_read(0));advance(100);
    puts("dance triggers: empty, first, 27/wrap/coalesce, probability, 4-note CHORD once, HOLD/release, ARP OFF, stopped internal arp, all-off/panic/project reset/isolation PASS");
    /* Real MIDI transport/clock ingestion, no alternative dance clock. */
    song.g[G_CLOCK]=1;advance(50);reset_track(t);arp_add(t,60);
    midi_enqueue(0xFu | (0xFAu<<8),1);advance(30);
    for(uint32_t k=0;k<72;k++){midi_enqueue(0xFu | (0xF8u<<8),1);advance(21);}
    assert(dance_read(0)&256u);
    midi_enqueue(0xFu | (0xFCu<<8),1);advance(30);state=dance_read(0);
    for(uint32_t k=0;k<24;k++){midi_enqueue(0xFu | (0xF8u<<8),1);advance(21);}
    assert(dance_read(0)==state);
    midi_enqueue(0xFu | (0xFBu<<8),1);advance(30);
    for(uint32_t k=0;k<24;k++){midi_enqueue(0xFu | (0xF8u<<8),1);advance(21);}
    assert(dance_read(0)!=state);song.g[G_CLOCK]=0;seq_stop();
    puts("dance external MIDI: START/pulses advance, STOP holds, CONTINUE/pulses resume PASS");
    composition();
    ui.home=0;ui.page=page_graph(GR_DANCE);song.sel=0;
    trk[0].p[P_AMODE]=AM_CHORD;trk[0].p[P_ARATE]=4;dance_style=1;
    assert(!project_save_as(0,"DANCE"));
    trk[0].p[P_ARATE]=0;dance_trigger(0);project_load(0);advance(100);
    assert(trk[0].p[P_ARATE]==4 && trk[0].p[P_AMODE]==AM_CHORD && !dance_read(0) && dance_style==1);
    dance_reset(0);ui.force=1;draw_graph();ui.force=0;
    uint32_t draws=web_draws;
    for(uint32_t k=0;k<1000;k++){dance_trigger(0);if(k%10u==9u)draw_graph();}
    assert(web_draws-draws==100);
    draws=web_draws;for(uint32_t k=0;k<100;k++)draw_graph();assert(web_draws==draws);
    puts("dance redraw coalescing: 1000 triggers / 100 paints = 100 graph blits; 100 idle attempts = 0 blits PASS");
    puts("dance persistence: normal project save/load retains musical mode/rate, resets progression, session style unchanged PASS");
    printf("dance sizes: masks=%zu telemetry=%zu UI-cache=%zu style=%zu track=%zu project=%zu canvas=%zu graph-transfer=%u bytes\n",sizeof bg_dance_masks,sizeof dance_state,sizeof dance_snapshot,sizeof dance_style,sizeof(track_t),sizeof(project_store_t),sizeof cv_px,240*122*2);
    return 0;
}

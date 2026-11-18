/* SPDX-License-Identifier: GPL-3.0-only */
#include "../felucca/web/emu/felucca_web.c"
#include <assert.h>
#include <stdio.h>
#include <pthread.h>
static void advance(uint32_t ms) { for(uint32_t n=0;n<ms*441u/10u;n+=128) web_render(128); }
static void bare(uint32_t i,uint32_t engine) {
    track_t *t=&trk[i];trk_all_off(t);t->p[P_MUTE]=0;perf_solo=0;pf.mute=0;
    set_engine_of(t,engine);apply_preset_to(t,0);engine_block(t);engine_block(t);
    t->xf_on=0;t->engine=t->eng_req;eng_mem_claim(t);
    trk_all_off(t);for(uint32_t v=0;v<NVOICE;v++)t->v[v].active=0;
    t->p[P_VOICE]=V_POLY;t->p[P_AMODE]=0;t->p[P_GLIDE]=0;t->p[P_SUS]=127;t->p[P_ATK]=0;t->p[P_DEC]=0;panic_req &= ~(1u<<i);
}
static void paint(void) { fm1_ms+=70;draw_graph();ui.force=0; }
static void pixels(void) {
    ui.home=1;ui_home_view=HV_CREW;
    for(uint32_t pal=0;pal<NPALETTES;pal++)for(uint32_t style=0;style<2;style++)
    for(uint32_t track=0;track<NTRK;track++)for(uint32_t step=0;step<22;step++) {
        uint32_t pose=step<12?step:22-step;
        palette_set(pal);ui_style=style;style_apply();song.sel=track;
        uint32_t phase=(step*256u+21u)/22u;
        __atomic_store_n(&crew_state[track],phase|CREW_ACTIVE|CREW_SEEN,__ATOMIC_RELAXED);
        ui.force=1;paint();
        for(uint32_t y=0;y<96;y++)for(uint32_t x=0;x<96;x++) {
            uint32_t bit=128u>>(x&7u),off=y*12u+x/8u;
            uint16_t expected=crew_masks[crew_moves[track]][pose][off]&bit ? T_THEME : T_BG;
            assert(web_fb[(graph_y()+22u+y)*240u+132u+x]==swap16(expected));
        }
    }
    ui_prefs|=PREF_LARGE;assert(large_kind()==LK_LABEL && graph_h()==122);ui_prefs&=~PREF_LARGE;
    ui.force=1;paint();uint32_t before=crew_graph_paints;
    for(uint32_t i=0;i<100;i++)paint();assert(crew_graph_paints==before);
    ui.home=0;for(uint32_t i=0;i<100;i++) {crew_phase+=1234567;crew_publish(128);paint();}
    assert(crew_graph_paints==before);
    ui.home=1;ui.force=1;paint();before=crew_graph_paints;
    for(uint32_t i=0;i<1000;i++) {fm1_ms++;crew_phase+=1234567;crew_publish(32);draw_graph();}
    assert(crew_graph_paints-before<=15);
    printf("CREW pixels: %u exact forward/backward single-dancer compositions, old-pixel erasure, six identities, palettes/styles, LARGE, idle/hidden/cap PASS\n",NPALETTES*2u*NTRK*22u);
}
static uint32_t done;
static void *producer(void *unused) {
    (void)unused;
    for(uint32_t k=0;k<1000000;k++) {
        if(!(k%127))crew_reset(0);
        crew_phase+=0x1234567u;crew_attack(0,k%127+1);crew_publish(32);
    }
    __atomic_store_n(&done,1,__ATOMIC_RELEASE);return 0;
}
static void stress(void) {
    bare(0,0);trk[0].v[0].active=trk[0].v[0].gate=1;trk[0].v[0].stage=2;
    pthread_t th;assert(!pthread_create(&th,0,producer,0));
    do {uint32_t s=crew_read(0);assert(!(s&~16383u));assert(!(s&CREW_ACTIVE)||((s&CREW_SEEN)&&!(s&(CREW_MUTED|CREW_SETTLE))));}
    while(!__atomic_load_n(&done,__ATOMIC_ACQUIRE));pthread_join(th,0);
    puts("CREW whole-word: 1M actual producer publications/resets with concurrent snapshot reads PASS");
}
int main(int argc,char **argv) {
    web_nor_erase();web_boot();advance(1200);
    if(argc>1) {
        int visible=atoi(argv[1]);assert(!web_bgirl_six_project(0));advance(50);
        ui.home=visible!=0;ui_home_view=HV_CREW;
        if(visible==2){ui.home=1;ui.layer=LAYER_GLO;}
        trk[1].p[P_AMODE]=AM_RND;trk[1].p[P_APROB]=73;trk[1].p[P_ASWING]=37;
        arp_add(&trk[1],60);arp_add(&trk[1],64);arp_add(&trk[1],67);seq_start();
        for(uint32_t k=0;k<4000;k++) {song.sel=k%NTRK;web_render(128);fwrite(out_l,sizeof(float),128,stdout);fwrite(out_r,sizeof(float),128,stdout);fwrite(&rng_state,4,1,stdout);fwrite(&trk[1].arp_note,1,1,stdout);}
        return 0;
    }
    assert(ui_home_view==HV_CREW && HV_SCOPE==0 && HV_TRACKS==1 && HV_LEVELS==2 && HV_CREW==3);
    static const uint8_t walk[22]={0,1,2,3,4,5,6,7,8,9,10,11,10,9,8,7,6,5,4,3,2,1};
    for(uint32_t phase=0;phase<256;phase++)assert(crew_pose(phase)==walk[phase*22u/256u]);
    assert(crew_pose(0)==0 && crew_pose(128)==11 && crew_pose(255)==1 && crew_pose(256)==0);
    puts("CREW ping-pong: all256 phase bins, both endpoints, reverse traversal and wrap PASS");
    persist_t pref={0};settings_export(&pref);
    for(uint32_t v=0;v<5;v++) {pref.favorites.factory[15][26]=v==4?255:v;assert(settings_import(&pref,sizeof pref));settings_init();assert(ui_home_view==(v==4?255:v));if(v==4)assert(menu_get(MI_HOME)==HV_SCOPE);}
    /* Real pre-upgrade records have no marker. Upgrade each old HOME choice
     * once without touching other settings, then preserve the next user choice. */
    for(uint32_t v=0;v<5;v++) for(uint32_t idle=0;idle<2;idle++) {
        persist_t old=pref, expected;
        old.favorites.factory[15][26]=v==4?255:v;
        old.favorites.factory[15][10]=(uint8_t)(0x54u|idle);
        expected=old;
        expected.favorites.factory[15][26]=HV_CREW;
        expected.favorites.factory[15][10]|=HOME_CREW_DEFAULT_APPLIED;
        assert(settings_import(&old,sizeof old)==1);
        assert(!memcmp(&old,&expected,sizeof old));
        settings_init();assert(ui_home_view==HV_CREW);
        assert((ui_idle&1u)==idle);
        assert(settings_import(&old,sizeof old)==1 && !memcmp(&old,&expected,sizeof old));
        menu_put(MI_HOME,v<4?v:0);settings_export(&old);
        assert(settings_import(&old,sizeof old)==1);settings_init();
        assert(ui_home_view==(v<4?v:0));
        menu_put(MI_ANIM,2);assert(ui_idle&HOME_CREW_DEFAULT_APPLIED);
        menu_put(MI_ANIM,0);assert(ui_idle&HOME_CREW_DEFAULT_APPLIED);
    }
    puts("CREW preferences: fresh default, one-time saved-settings upgrade, unrelated bytes/IDLE preserved, later HOME choices retained PASS");
    for(uint32_t i=0;i<NTRK;i++)bare(i,0);
    track_t *t=&trk[0];
    trk_note_on(t,60,100);trk_note_off(t,60);crew_publish(32);
    assert(crew_read(0)&CREW_ACTIVE); /* admitted short gate in the same block still accents */
    bare(0,0);
    trk_note_on(t,60,100);crew_publish(32);assert(crew_read(0)&CREW_ACTIVE);assert(!crew_read(1));
    uint32_t phase=crew_read(0)&255u;advance(300);assert((crew_read(0)&255u)!=phase && (crew_read(0)&CREW_ACTIVE));
    trk_note_off(t,60);advance(100);assert(crew_read(0)&CREW_SETTLE);advance(700);assert(!(crew_read(0)&(CREW_ACTIVE|CREW_SETTLE)));
    uint32_t frozen=crew_read(0);advance(500);assert(crew_read(0)==frozen);
    trk_note_on(t,60,100);crew_publish(32);t->p[P_MUTE]=1;crew_publish(32);assert(crew_read(0)==CREW_MUTED);
    t->p[P_MUTE]=0;crew_publish(32);assert(!(crew_read(0)&CREW_ACTIVE));trk_note_on(t,62,100);crew_publish(32);assert(crew_read(0)&CREW_ACTIVE);
    pf.mute=1;crew_publish(32);assert(crew_read(0)==CREW_MUTED);pf.mute=0;
    trk_all_off(t);crew_publish(32);assert(!crew_read(0));
    t->p[P_MUTE]=1;trk_note_on(t,64,100);assert(!crew_rt[0].pending);t->p[P_MUTE]=0;
    bare(0,0);t->p[P_VOICE]=V_MONO;t->p[P_PRIO]=1;
    trk_note_on(t,60,100);crew_publish(32);trk_note_on(t,72,100);assert(!crew_rt[0].pending);
    t->p[P_VOICE]=V_LEGATO;trk_note_on(t,48,100);assert(crew_rt[0].pending);crew_publish(32);
    trk_all_off(t);t->eng_req=3;trk_note_on(t,60,100);assert(!crew_rt[0].pending && !crew_read(0));
    bare(0,ENGI_DRUM);trk_note_on(t,36,100);trk_note_off(t,36);crew_publish(32);assert(crew_read(0)&CREW_ACTIVE);advance(1500);assert(!(crew_read(0)&(CREW_ACTIVE|CREW_SETTLE)));
    bare(0,0);trk_note_on(t,60,100);for(uint32_t v=0;v<NVOICE;v++)if(t->v[v].active)t->v[v].stage=4;crew_publish(32);assert(!(crew_read(0)&CREW_ACTIVE));
    bare(0,0);trk_note_on(t,60,100);crew_publish(32);panic_req=1;advance(10);assert(!crew_read(0));
    trk_note_on(t,60,100);crew_publish(32);assert(!project_new());assert(!crew_read(0));
    crew_phase=0xFFFFFFF0u;events_block(128);assert(crew_phase<0xFFFFFFF0u);
    uint32_t prior=crew_phase;song.g[G_BPM]=240;events_block(128);assert(crew_phase!=prior && crew_phase!=0);
    puts("CREW activity: sustained/release/settle/freeze, stopped live, mute/solo inhibition, new attack, rejected/deferred, mono priority/legato, one-shot, stolen, panic/project, phase wrap/BPM PASS");
    song.g[G_CLOCK]=1;events_block(32);midi_clock_transport(0xFAu,fm1_ms);
    assert(!crew_phase);midi_clock.have_pulse=1;midi_clock.last_ms=fm1_ms;midi_clock.pulse_units=1000;
    midi_clock.pos=6000;events_block(32);assert(crew_phase==0x10000000u);
    song.g[G_BPM]=70;midi_clock.pos=12000;events_block(32);assert(crew_phase==0x20000000u);
    midi_clock_transport(0xFCu,fm1_ms);prior=crew_phase;events_block(128);assert(crew_phase!=prior);
    prior=crew_phase;midi_clock_transport(0xFBu,fm1_ms);events_block(128);assert(crew_phase==prior);
    midi_clock_transport(0xFAu,fm1_ms);assert(!crew_phase);song.g[G_CLOCK]=0;events_block(32);
    puts("CREW external START/STOP/CONTINUE: exact GRID musical units, BPM-independent phase, stopped local phase, CONTINUE preserves, START resets PASS");
    pixels();stress();
    printf("CREW bytes: masks=%zu atomic=%zu producer=%zu phase=%zu canvas=%zu track=%zu wire=%zu graph=%u\n",sizeof bg_windmill_masks*4u,sizeof crew_state,sizeof crew_rt,sizeof crew_phase,sizeof cv_px,sizeof(track_t),sizeof(project_store_t),240*122*2);
    return 0;
}

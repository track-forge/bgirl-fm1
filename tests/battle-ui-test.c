/* SPDX-License-Identifier: GPL-3.0-only
 * Battle Circuit: actual boot/settings compatibility and native framebuffer evidence. */
#include "../felucca/web/emu/felucca_web.c"
#include <assert.h>
#include <stdio.h>
static void advance(void) { for(int k=0;k<420;k++) web_render(128); }
static uint32_t page_named(const char *name) {
    for(uint32_t k=0;k<NPAGES;k++) if(!strcmp(PAGES[k].title,name)) return k;
    assert(0); return 0;
}
static void shot(const char *out,const char *name) {
    ui.force=1; ui_draw(); ui.force=0;
    if(!out) return;
    char path[512];snprintf(path,sizeof path,"%s/%s.ppm",out,name);
    FILE *f=fopen(path,"wb");assert(f);fprintf(f,"P6\n240 240\n255\n");
    for(uint32_t i=0;i<240*240;i++) {
        uint16_t c=swap16(web_fb[i]);
        uint8_t rgb[]={ (c>>11)*255/31,((c>>5)&63)*255/63,(c&31)*255/31 };
        fwrite(rgb,1,3,f);
    }
    fclose(f);
}
int main(int argc,char **argv) {
    const char *out=argc>1?argv[1]:NULL;
    web_nor_erase();web_boot();advance();
    assert(settings.palette==UI_STUDIO_INDEX && !ux.circuit && ui_style==ST_FLAT);
    assert(ux.studio && T_RHYTHM!=T_THEME && T_ACCENT!=T_RHYTHM && T_ACCENT!=T_REC);
    assert(T_PLAY==T_THEME && T_STAGE==T_REC && T_TEXT!=T_THEME);
    persist_t p,q;
    static const char *names[]={"GREY","GREEN","AMBER","ICE","VIOLET","ROSE","PAPER","HI-CON","NIGHT","MONO"};
    static const uint8_t old[]={1,2,3,5,0,4,5,3,2,3,4,7,6,6,6,6,6,6,6,7};
    for(uint32_t k=0;k<20;k++) assert(palette_from_stored(k)==old[k]);
    for(uint32_t k=0;k<NPALETTES;k++) {
        if(k<10) assert(!strcmp(UI_PALETTES[k].name,names[k]));
        for(uint32_t style=0;style<2;style++) for(uint32_t large=0;large<2;large++) {
            settings.palette=k;ui_style=style;ui_prefs=large?PREF_LARGE:0;
            settings_export(&p);assert(p.palette==UI_PAL_TAG+k);
            memset(&settings,0,sizeof settings);settings_init();assert(!ux.circuit && settings.palette==UI_STUDIO_INDEX);
            assert(settings_import(&p,sizeof p)==1);settings_init();style_apply();
            assert(settings.palette==k && ui_style==style && !!(ui_prefs&PREF_LARGE)==large);
            assert(ux.circuit==(k==UI_CIRCUIT_INDEX));
            settings_export(&q);assert(!memcmp(&p,&q,sizeof p));
        }
    }
    const uint16_t custom[]={RGB(12,20,28),RGB(130,200,240),RGB(240,248,255),RGB(255,120,80),RGB(220,240,255)};
    cpal_store(custom);ui_cpal_flags|=CPAL_ACTIVE;palette_apply();settings_export(&p);
    memset(&favorites,0,sizeof favorites);assert(settings_import(&p,sizeof p)==1);settings_init();style_apply();
    assert(cpal_on() && !ux.circuit && !ux.studio && !memcmp(ux.pal,custom,sizeof custom));
    assert(T_RHYTHM==T_THEME && T_SUPPORT==T_MID && T_PLAY==T_TEXT && T_STAGE==T_THEME);
    settings_export(&q);assert(!memcmp(&p,&q,sizeof p));
    ui_cpal_flags=0;settings.palette=UI_CIRCUIT_INDEX;ui_style=ST_FLAT;ui_prefs=0;style_apply();palette_apply();
    /* No idle redraw added by the skin. */
    ui.home=0;ui.page=page_named("VOCODER");ui.force=1;draw_graph();ui.force=0;
    uint32_t draws=web_draws;draw_graph();assert(web_draws==draws);
    song.sel=3;ui.home=0;ui.page=page_named("VOCODER");shot(out,"vocoder");
    ui.page=page_named("VOC INPUT");trk[3].p[P_VHOLD]=1;shot(out,"vocoder-hold");
    ui.page=page_named("VOC COLOR");shot(out,"vocoder-color");
    trk[3].p[P_E0]=SLC_SRC_VOCALS;trk[3].p[P_E1]=0;trk[3].p[P_E2]=0;slc_vocal_select(&trk[3]);ui.page=page_named("EDIT 1");
    shot(out,"vocal-ready");bg_vocal_expected[3][0]^=1;shot(out,"bank-miss");
    song.sel=1;ui.page=page_named("ARP DANCE");
    dance_trigger(1);dance_trigger(1);dance_trigger(1);dance_style=0;shot(out,"dance-crisp");
    dance_style=1;shot(out,"dance-ghost");ui_prefs|=PREF_LARGE;shot(out,"dance-large");
    /* Selected and coincident playhead remain separate shapes in the existing grid. */
    song.sel=0;ui_prefs=0;ui.page=page_named("STEP");ui.cursor=0;trk[0].seq_idx=0;song.playing=1;shot(out,"grid-coincident");
    puts("Battle Circuit: fresh boot; 44 palette/style/LARGE round-trips; 20 legacy IDs; custom palette bytes; idle redraw PASS");
    return 0;
}

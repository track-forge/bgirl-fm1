/* SPDX-License-Identifier: GPL-3.0-only
 * BGIRL delta integration: real firmware, panel, sequencer and simulated NOR. */
#include "../felucca/web/emu/felucca_web.c"
#include <assert.h>
static void advance(uint32_t ms) { uint32_t n; for(n=0;n<ms*441u/10u;n+=128u) web_render(128); }
static void tap(uint32_t bit) {web_buttons(1u<<bit); advance(60);web_buttons(0);advance(60);}
static void defaults(int demo) {
    assert(trk[0].eng_req==ENGI_DRUM && trk[0].preset==2);
    assert(trk[1].eng_req==0 && trk[1].preset==2 && trk[1].p[P_VOICE]==(demo && BGIRL_VOCODER ? V_MONO : V_LEGATO));
#if BGIRL_VOCODER
    if(demo) {
        assert(trk[2].eng_req==0 && trk[2].preset==12 && trk[2].p[P_SUS]==127);
        assert(trk[3].eng_req==13);
        assert(trk[3].p[P_VCARR]==3 && !trk[3].p[P_VMON] && trk[3].p[P_VBANDS]==2);
        assert(trk[2].step[1].time==ST_TIE);
        assert(trk[2].step[0].note[0]==21 && trk[2].step[16].note[0]==19);
        assert(trk[3].p[P_E0]==SLC_SRC_VOCALS && trk[3].p[P_E1]==0);
        assert(slc_vocal_match(&trk[3]));
        assert(trk[2].p[P_SLEN]==64 && trk[3].p[P_SLEN]==64);
        assert(trk[4].eng_req==8 && trk[4].step[33].note[0]==64);
        assert(song.g[G_BPM]==128);
    } else
#endif
    {
        assert(trk[2].eng_req==ENGI_FM6 && trk[2].preset==1);
        assert(trk[3].eng_req==5 && trk[3].preset==1 && !trk[3].p[P_VCARR]);
    }
    assert(!song.playing);
}
int main(void) {
    uint32_t t,i; float peak=0;
#if BGIRL_VOCODER
    uint64_t speechEnergy=0;
    uint32_t vocalWraps=0,previousStep=0;
#endif
    web_nor_erase(); web_boot(); advance(1200); defaults(1);
#if BGIRL_VOCODER
    assert(trk[0].step[4].hit==22 && trk[0].step[0].hit==9);
#else
    assert(trk[0].step[4].hit==6 && trk[0].step[0].hit==1);
#endif
    assert(trk[1].step[1].n && trk[2].step[0].n);
#if BGIRL_VOCODER
    assert(trk[3].step[0].n);
#else
    assert(trk[3].step[11].n);
#endif
    for(t=0;t<NTRK-1;t++) {
        web_enc(EN_ALGO,1); advance(80); /* ALGORITHM is track selection */
        assert(song.sel==t+1);
    }
    for(i=0;i<NTRK-1;i++) { web_enc(EN_ALGO,-1); advance(80); } assert(song.sel==0);
    tap(B_PLAY); assert(song.playing);
    for(i=0;i<44100u*8u;i+=128u) {
        web_render(128);
#if BGIRL_VOCODER
        assert(bg_bus.owner==3 && bg_bus.carrier==2);
        assert(trk[3].seq_idx<64);
        if(trk[3].seq_idx<previousStep) vocalWraps++;
        previousStep=trk[3].seq_idx;
        for(uint32_t k=0;k<CTL;k++)speechEnergy+=(uint64_t)llabs(bg_bus.mod[k]);
#endif
        for(t=0;t<128;t++) { assert(isfinite(out_l[t]) && isfinite(out_r[t]));
            assert(fabsf(out_l[t])<=1 && fabsf(out_r[t])<=1);
            if(fabsf(out_l[t])>peak)peak=fabsf(out_l[t]); }
    }
#if BGIRL_VOCODER
    assert(speechEnergy>0); /* backing mix alone cannot pass the demo test */
    assert(vocalWraps>=1);
#endif
    assert(peak>0.001f); tap(B_PLAY); assert(!song.playing);
    trk[3].p[P_VWET]=81;
    trk[1].p[P_E4]=42; trk[1].step[1].note[0]=41;
    assert(!project_save_as(0,"MY ELECTRO"));
    assert(!web_bgirl_project(0)); advance(100); defaults(0);
    for(t=0;t<NTRK;t++)for(i=0;i<NSTEP;i++)assert(!trk[t].step[i].n && !trk[t].step[i].hit);
    project_load(0); advance(100);
    assert(trk[1].p[P_E4]==42 && trk[1].step[1].note[0]==41 && trk[3].p[P_VWET]==81);
    advance(70000); /* autosave then boot initialization, as a fresh instance */
    felucca_init(); autosave_boot(1); advance(100);
    assert(trk[1].p[P_E4]==42 && trk[1].step[1].note[0]==41 && trk[3].p[P_VWET]==81);
    assert(!web_bgirl_project(1)); advance(100); defaults(1);
#if BGIRL_VOCODER
    assert(trk[1].step[1].note[0]==21 && trk[0].step[4].hit==22);
#else
    assert(trk[1].step[1].note[0]==36 && trk[0].step[4].hit==6);
#endif
    /* Switching track while held must release the originally selected track. */
    web_enc(EN_ALGO,1);advance(60);web_keys(1u<<7);advance(100);
    web_enc(EN_ALGO,1);advance(60);web_keys(0);advance(4000);
    for(t=0;t<NTRK;t++)assert(!trk[t].nheld);
    assert(!web_bgirl_project(0));advance(4000);
    peak=0;for(i=0;i<128;i++)peak+=fabsf(out_l[i])+fabsf(out_r[i]);assert(peak<0.0001f);
    puts("BGIRL: roles, editable demo/bounds, track selection, project roundtrip, autosave preservation, blank reset and note release PASS");
    return 0;
}

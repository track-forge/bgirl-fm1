/* SPDX-License-Identifier: GPL-3.0-only
 * Focused real-firmware composition and redraw receipt; shares upstream lint. */
#define BGIRL_RENDER_LIBRARY
#include "../felucca/tests/ui_render.c"
#undef main
int main(int argc, char **argv) {
    const char *out=argc>1?argv[1]:"local/stage-lab-cut/after";
    char path[512], name[96];
    snprintf(path,sizeof path,"%s/report.txt",out); rep=fopen(path,"w");
    if(!rep)return 2;
    const int scenes[]={S_HOME_IDLE,S_FM6_OP,S_DRUM,S_ENV};
    const char *names[]={"stage","lab","cut","env"};
    for(uint32_t p=0;p<NPALETTES;p++)for(int large=0;large<2;large++)for(int style=0;style<2;style++)for(int s=0;s<4;s++)for(int play=0;play<2;play++) {
        setup(scenes[s]);pal(p);ui_style=style;style_apply();
        ui_prefs=(ui_prefs&~PREF_LARGE)|(large?PREF_LARGE:0);song.playing=play;
        if(s==0){ui_home_view=HV_CREW; ui.home=1;}
        snprintf(name,sizeof name,"%s_%s_%s_%s",names[s],large?"large":"normal",style?"line":"flat",play?"play":"stop");
        cur_name=name; ui.force=1; draw(scenes[s]); lint();mono_check();
        if(p==UI_STUDIO_INDEX || p==UI_CIRCUIT_INDEX || !strcmp(UI_PALETTES[p].name,"MONO") || !strcmp(UI_PALETTES[p].name,"PAPER"))write_ppm(out,UI_PALETTES[p].name,name);
    }
    /* CREW's shared register must fit every engine's real HOME bindings, not
     * just the default drum/ANALOG examples. Exercise complete ordinary ranges. */
    for(int studio=0;studio<2;studio++)for(int large=0;large<2;large++)for(uint32_t e=0;e<NENGINES;e++) {
        if(!eng_ok(e))continue;
        setup(S_HOME_IDLE);eng(e);go_home();ui_home_view=HV_CREW;
        pal(studio?UI_STUDIO_INDEX:UI_CIRCUIT_INDEX);ui_prefs=large?PREF_LARGE:0;
        for(uint32_t col=0;col<4;col++) {
            int16_t *vp;const param_desc_t *d=home_param(col,&vp);int32_t old=*vp;
            for(int32_t v=d->min;v<=d->max;v++) {
                *vp=v;ui.hot_col=col;ui.hot_t=30;ui.force=1;
                snprintf(name,sizeof name,"stage-engine%u-k%u-large%d",e,col+1,large);cur_name=name;
                draw(S_HOME_IDLE);lint();
                if(v-d->min>512)v=d->max-1;
            }
            *vp=old;
        }
    }
    /* Every actual FM topology and selected operator, including LARGE geometry. */
    for(int studio=0;studio<2;studio++)for(int large=0;large<2;large++)for(int alg=1;alg<=32;alg++)for(int op=0;op<6;op++) {
        setup(S_FM6_OP);pal(studio?UI_STUDIO_INDEX:UI_CIRCUIT_INDEX);ui_prefs=large?PREF_LARGE:0;
        TSEL->p[P_E0]=alg;fop_op=op;ui.force=1;
        snprintf(name,sizeof name,"lab-alg%d-op%d-large%d",alg,op,large);cur_name=name;
        draw(S_FM6_OP);lint();
    }
    /*64-step banks: cursor and playhead coincide with accented/ratcheted events,
     * selected and muted lanes, and a shortened last bank. */
    for(int studio=0;studio<2;studio++)for(int large=0;large<2;large++)for(int pos=0;pos<64;pos++)for(int lane=0;lane<NLANE;lane++) {
        setup(S_DRUM);pal(studio?UI_STUDIO_INDEX:UI_CIRCUIT_INDEX);ui_prefs=large?PREF_LARGE:0;
        TSEL->p[P_SLEN]=64;ui.bank=pos/16;ui.cursor=pos;ui.lane=lane;
        TSEL->seq_idx=pos;song.playing=1;drum_mute[trk_index(TSEL)%NPART]=1u<<lane;
        for(int k=16;k<64;k++)TSEL->step[k]=TSEL->step[k%16];
        snprintf(name,sizeof name,"cut-step%d-lane%d-large%d",pos,lane,large);cur_name=name;
        ui.force=1;draw(S_DRUM);lint();
    }
    /* Native icon contact sheet uses the actual compressed firmware decoder. */
    setup(S_HOME_IDLE);pal(UI_STUDIO_INDEX);ui_style=0;style_apply();
    nscr=npend=nrules=0;cur_name="BGIRL-icons";
    lcd_fill(0,0,240,240,T_BG);
    cv_begin(240,24,T_BG);cv_oy=0;
    cv_text(8,3,&AF_C,"BGIRL / SECTION ICONS",T_TEXT);cv_blit(0,0);
    const uint32_t icons[3]={ICON_BG_STAGE,ICON_BG_LAB,ICON_BG_CUT};
    const char *titles[3]={"STAGE","LAB","CUT"};
    const uint32_t sizes[3]={12,16,24};
    for(uint32_t row=0;row<3;row++) {
        char label[8];fmt_int(label,sizes[row]);str_cpy(label+str_len(label)," PX",4);
        cv_begin(240,64,T_BG);cv_oy=0;
        cv_text(8,26,&AF_C,label,T_SUPPORT);
        for(uint32_t col=0;col<3;col++) {
            cv_text_in(64+col*56,2,48,&AF_C,titles[col],T_TEXT,T_BG);
            cv_icon_in(64+col*56,22,48,32,sizes[row],icons[col],T_TEXT,T_BG);
        }
        cv_blit(0,24+row*64);lint();
    }
    write_ppm(out,"STUDIO","icons");
    fprintf(rep,"focused findings=%u ink=%u mono=%u align=%u lost=%u canvas=%zu\n",nfind,nink,mono_bad,al_fail,al_lost,sizeof cv_px);
    uint32_t failures=nfind+nink+mono_bad+al_fail+al_lost;
    nscr=npend=nrules=0;al_off=1;
    /* Same deterministic non-audio draw recipes before and after. */
    for(int s=0;s<4;s++) {
        setup(scenes[s]);pal(UI_CIRCUIT_INDEX); if(!s){ui_home_view=HV_CREW;ui.home=1;}
        ui.force=1;ui_draw();ui.force=0;
        uint64_t b=blit_px;
        for(int i=0;i<100;i++){fm1_ms+=10;ui_draw();}
        fprintf(rep,"%s idle100 pixels=%llu\n",names[s],(unsigned long long)(blit_px-b));
        b=blit_px;
        for(int i=0;i<100;i++){ui.hot_col=i%4;ui.hot_t=30;fm1_ms+=10;ui_draw();}
        fprintf(rep,"%s focus100 pixels=%llu\n",names[s],(unsigned long long)(blit_px-b));
    }
    fprintf(rep,"total pixels=%llu\n",(unsigned long long)blit_px);fclose(rep);
    return failures?1:0;
}

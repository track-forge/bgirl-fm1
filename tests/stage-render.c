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
        if(p==UI_CIRCUIT_INDEX || !strcmp(UI_PALETTES[p].name,"MONO") || !strcmp(UI_PALETTES[p].name,"PAPER"))write_ppm(out,UI_PALETTES[p].name,name);
    }
    fprintf(rep,"focused findings=%u ink=%u mono=%u align=%u lost=%u canvas=%zu\n",nfind,nink,mono_bad,al_fail,al_lost,sizeof cv_px);
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
    return nfind||nink||mono_bad||al_fail||al_lost?1:0;
}

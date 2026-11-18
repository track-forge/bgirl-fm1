/* SPDX-License-Identifier: GPL-3.0-only */
#define BGIRL_RENDER_LIBRARY
#include "../felucca/tests/ui_render.c"
#undef main
int main(int argc,char **argv) {
 const char *out=argc>1?argv[1]:"local/lab-rollout/after";char path[512],name[96];
 snprintf(path,sizeof path,"%s/report.txt",out);rep=fopen(path,"w");if(!rep)return 2;
 const int scenes[]={S_ENVDEST,S_LFO,S_LFO2,S_MOD,S_FX,S_DLY,S_REVERB,S_INSERT_FOLD,S_INSERT_PHASER,S_FILTER,S_VOICE,S_EDIT_ANALOG,S_FM6_ALG5,S_FM6_OP_ENV};
 const char *names[]={"env-dest","lfo","lfo2","mod","fx","delay","reverb","insert-fold","insert-phaser","filter","voice","analog","fm6","op-env"};
 for(uint32_t p=0;p<NPALETTES;p++)for(int large=0;large<2;large++)for(int style=0;style<2;style++)for(uint32_t s=0;s<sizeof scenes/sizeof *scenes;s++){
  setup(scenes[s]);pal(p);ui_style=style;style_apply();ui_prefs=large?PREF_LARGE:0;
  snprintf(name,sizeof name,"%s-%s-%s",names[s],large?"large":"normal",style?"line":"flat");cur_name=name;ui.force=1;draw(scenes[s]);lint();mono_check();
  if(p==UI_STUDIO_INDEX)write_ppm(out,"STUDIO",name);
 }
 /* Every visible LAB page/engine combination: actual bindings and LARGE range faces. */
 FILE *inv;snprintf(path,sizeof path,"%s/visible-pages.tsv",out);inv=fopen(path,"w");
 fprintf(inv,"engine\tpage\ttitle\tlarge\n");
 for(uint32_t e=0;e<NENGINES;e++)if(eng_ok(e))for(uint32_t i=0;i<NPAGES;i++)for(int large=0;large<2;large++){
  setup(S_ENV);eng(e);if(!page_visible(i))continue;
  const page_t *pg=&PAGES[i];if(pg->fam!=FAM_ENV&&pg->fam!=FAM_LFO&&pg->fam!=FAM_FX&&pg->fam!=FAM_EDIT)continue;
  if(pg->graph==GR_SLICES||pg->graph==GR_SLCR||pg->id[0]==P_LN0||pg->id[0]==P_LN4)continue;
  if(pg->scope==SC_ENGINE&&e==ENGI_SAMPLE)continue;
  ui.page=i;ui.home=0;pal(UI_STUDIO_INDEX);ui_prefs=large?PREF_LARGE:0;ui_style=1;style_apply();
  snprintf(name,sizeof name,"engine%u-page%u-large%d",e,i,large);cur_name=name;ui.force=1;draw(S_ENV);lint();
  fprintf(inv,"%u\t%u\t%s\t%d\n",e,i,pg->title,large);
  if(e==ENGI_SAMPLE||e==13||pg->scope==SC_ENGINE)write_ppm(out,"VARIANTS",name);
  for(uint32_t k=0;k<4;k++) {
   int16_t *vp;const param_desc_t *d=page_desc(pg,k,&vp);
   if(!d||!vp)continue;
   int16_t saved=*vp;
   for(int edge=0;edge<2;edge++){
    *vp=edge?d->max:d->min;ui.hot_t=30;ui.hot_col=k;ui.force=1;
    draw(S_ENV);lint();
   }
   *vp=saved;
  }
 }
 fclose(inv);
#ifndef BGIRL_LAB_BASELINE
 setup(S_ENV);pal(UI_STUDIO_INDEX);nscr=npend=nrules=0;cur_name="vinyl-beat";
 lcd_fill(0,0,240,240,T_BG);cv_begin(240,120,T_BG);cv_oy=0;
 cv_text(12,6,&AF_C,"VINYL / BEAT",T_TEXT);
 const uint32_t sizes[3]={12,16,24};
 for(uint32_t k=0;k<3;k++){char b[8];fmt_int(b,sizes[k]);cv_text(16+72*k,40,&AF_C,b,T_SUPPORT);cv_icon_in(12+72*k,64,48,32,sizes[k],ICON_BG_VINYL,T_TEXT,T_BG);}
 cv_blit(0,40);write_ppm(out,"STUDIO","vinyl-icons");
#endif
 fprintf(rep,"LAB findings=%u ink=%u neutral=%u align=%u lost=%u canvas=%zu\n",nfind,nink,mono_bad,al_fail,al_lost,sizeof cv_px);
 fclose(rep);return nfind+nink+mono_bad+al_fail+al_lost?1:0;
}

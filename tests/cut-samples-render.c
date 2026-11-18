/* SPDX-License-Identifier: GPL-3.0-only */
#define UI_LINT_LIMIT 12000
#define BGIRL_RENDER_LIBRARY
#include "../felucca/tests/ui_render.c"
#undef main
#include <assert.h>
static const char *const cut_names[]={"sample","sample-play","sample-empty","sample-unlooped","sample-loading","slices-break","slices-user","slices-missing","slices-end","slices-dense","slices-split","slices-join","slices-piano","slice-source","slice-play","vocal-whole","vocal-chops","vocal-missing","slicer-off","slicer-gate","slicer-stutter","slicer-depth0","slicer-depth127","lanes","lanes2","lanes-muted","lanes-zero","lanes-kit2","lanes-kit3","lanes-trackmute","slicer-playing","sample-looped","sample-user","sample-high"};
static void cut_setup(uint32_t s){
 setup(S_EDIT_SAMPLE);song.playing=0;ui.msg_t=0;
 if(s>=31){eng(4);go_title("EDIT 1");TSEL->p[P_E3]=1;
  if(s==31){int found=0;for(uint32_t set=0;set<SMP_NSETS&&!found;set++)for(uint32_t k=0;k<SMP_SETS[set].nz;k++){const smp_zone_t *z=&SMP_ZONES[SMP_SETS[set].z0+k];if(z->looped&&z->n){TSEL->p[P_E0]=set;last_note=z->lo;found=1;break;}}assert(found);}
  if(s==32){host_slot_make(0);smp_user_scan(0);TSEL->p[P_E0]=SMP_NSETS;}
  if(s==33)last_note=127;sample_wave.zone=0;sample_wave.ready=0;for(int i=0;i<2000&&!sample_wave.ready;i++)sample_wave_tick(TSEL);
 }else if(s<5){eng(4);go_title(s==1?"EDIT 2":"EDIT 1");TSEL->p[P_E0]=s==2?SMP_NSETS:0;TSEL->p[P_E3]=s!=3;sample_wave.zone=0;for(int i=0;i<(s==4?1:256);i++)sample_wave_tick(TSEL);}
 else if(s<13){setup(s==6||s==8||s==9||s==10||s==11?S_SLICES_USR:s==7?S_SLICES_NOFILE:S_SLICES_BREAK);song.playing=0;
  if(s==8)sp.sel=slice_count();
  if(s==9){TSEL->p[P_E1]=3;sp.sel=16;}
  if(s==10||s==11)ui.act=s==10?3:4;
  if(s==12){TSEL->p[P_E0]=SLC_SRC_PIANO;sp.sel=0;}
 }else if(s<18){eng(13);go_title(s==14?"EDIT 2":"EDIT 1");
  if(s>=15){TSEL->p[P_E0]=SLC_SRC_VOCALS;TSEL->p[P_E1]=s==15?0:1;slc_vocal_select(TSEL);if(s==17)bg_vocal_expected[trk_index(TSEL)][0]^=1;}
 }else if(s<23){go_title("SLICER");TSEL->p[P_SLCR]=s==18?SL_OFF:s==20?SL_STUT:SL_GATE;TSEL->p[P_SLDEPTH]=s==21?0:127;sl[song.sel].idx=7;}
 else if(s==30){go_title("SLICER");TSEL->p[P_SLCR]=SL_GATE;sl[song.sel].idx=5;song.playing=1;}
 else{drum(0);go_title(s==24?"LANES 2":"LANES");for(int k=0;k<8;k++)TSEL->p[P_LN0+k]=s==26?0:(k+1)*15;drum_mute[trk_index(TSEL)]=s==25?0x55:0;TSEL->p[P_MUTE]=s==29;if(s==27||s==28)TSEL->p[P_E0]=s-26;ui.hot_col=2;ui.hot_t=30;}
 ui.msg_t=0;
}
static uint32_t cut_state(void){uint32_t h=fnv(2166136261u,trk,sizeof trk);h=fnv(h,&motion,sizeof motion);h=fnv(h,&chain_config,sizeof chain_config);h=fnv(h,slc_man,sizeof slc_man);h=fnv(h,bg_vocal_expected,sizeof bg_vocal_expected);return fnv(h,drum_mute,sizeof drum_mute);}
int main(int argc,char **argv){
 const char *out=argc>1?argv[1]:"local/cut-complete/after/native";char path[512],name[100];snprintf(path,sizeof path,"%s/report.txt",out);rep=fopen(path,"w");assert(rep);uint32_t cases=0;
 for(uint32_t p=0;p<NPALETTES;p++)for(int large=0;large<2;large++)for(int style=0;style<2;style++)for(uint32_t s=0;s<NELEM(cut_names);s++){
  cut_setup(s);pal(p);ui_style=style;style_apply();ui_prefs=large?PREF_LARGE:0;snprintf(name,sizeof name,"%s-%s-%s",cut_names[s],large?"large":"normal",style?"line":"flat");cur_name=name;
  uint32_t h=cut_state();draw(S_ENV);assert(h==cut_state());lint();mono_check();cases++;
  if(p==UI_STUDIO_INDEX)write_ppm(out,"STUDIO",name);
 }
 for(int large=0;large<2;large++)for(int mode=0;mode<3;mode++)for(int pat=1;pat<=16;pat++)for(int play=0;play<16;play++){
  cut_setup(30);pal(UI_STUDIO_INDEX);ui_prefs=large?PREF_LARGE:0;TSEL->p[P_SLCR]=mode;TSEL->p[P_SLPAT]=pat;sl[song.sel].idx=play;
  snprintf(name,sizeof name,"slicer-%d-%d-%d-%d",large,mode,pat,play);cur_name=name;uint32_t h=cut_state();draw(S_ENV);assert(h==cut_state());lint();cases++;
#ifndef BGIRL_CUT_BASELINE
  if(mode)assert(swap16(host_screen[(graph_y()+26)*240+13+13*play])==T_PLAY);
#endif
 }
 fprintf(rep,"CUT samples cases=%u findings=%u ink=%u neutral=%u align=%u lost=%u canvas=%zu\n",cases,nfind,nink,mono_bad,al_fail,al_lost,sizeof cv_px);fclose(rep);return nfind+nink+mono_bad+al_fail+al_lost?1:0;
}

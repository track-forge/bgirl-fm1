/* SPDX-License-Identifier: GPL-3.0-only
 * CUT real-firmware layouts + musical-state preservation and overlap sweeps. */
#define UI_LINT_LIMIT 12000
#define BGIRL_RENDER_LIBRARY
#include "../felucca/tests/ui_render.c"
#undef main
#include <assert.h>
static uint32_t cut_music_hash(void){uint32_t h=fnv(2166136261u,trk,sizeof trk);h=fnv(h,&motion,sizeof motion);return fnv(h,&chain_config,sizeof chain_config);}
int main(int argc,char **argv){
 const char *out=argc>1?argv[1]:"local/cut-rollout/after/native";char path[512],name[96];uint32_t cases=0;
 snprintf(path,sizeof path,"%s/report.txt",out);rep=fopen(path,"w");if(!rep)return 2;
 const int scenes[]={S_ROLL_ACID,S_ROLL_CHORDS,S_ROLL_TIES,S_ROLL_LOW,S_ROLL_HIGH,S_ROLL_WIDE,S_ROLL_LEN32,S_ROLL_LOCKS,S_ROLL_PLAYING,S_DETAIL,S_DETAIL_COND,S_DETAIL_FILL,S_DETAIL_DRUM,S_DETAIL_REST,S_AUTO_LIST,S_AUTO_LIST_ADD,S_AUTO_QNT,S_AUTO_NUDGE,S_AUTO_VEL,S_AUTO_COND,S_SONG_EMPTY,S_SONG_LANES,S_SONG_LONG,S_SONG_PLAYING,S_SONG_LONG_PLAYING,S_SONG_UNSAVED,S_SONG_ADD,S_SONG_PICTURES};
 const char *names[]={"roll-acid","roll-chords","roll-ties","roll-low","roll-high","roll-wide","roll-bank","roll-locks","roll-playing","detail","detail-cond","detail-fill","detail-drum","detail-rest","auto-list","auto-add","auto-quantize","auto-nudge","auto-velocity","auto-condition","song-empty","song-lanes","song-long","song-playing","song-long-playing","song-missing","song-add","song-density"};
 for(uint32_t p=0;p<NPALETTES;p++)for(int large=0;large<2;large++)for(int style=0;style<2;style++)for(uint32_t s=0;s<sizeof scenes/sizeof *scenes;s++){
  setup(scenes[s]);pal(p);ui_style=style;style_apply();ui_prefs=large?PREF_LARGE:0;
  snprintf(name,sizeof name,"%s-%s-%s",names[s],large?"large":"normal",style?"line":"flat");cur_name=name;ui.force=1;
  uint32_t before=cut_music_hash();draw(scenes[s]);assert(before==cut_music_hash());lint();mono_check();cases++;
  if(p==UI_STUDIO_INDEX)write_ppm(out,"STUDIO",name);
 }
 for(int large=0;large<2;large++)for(int detail=0;detail<2;detail++)for(uint32_t pos=0;pos<64;pos++)for(uint32_t play=0;play<3;play++){
  setup(detail?S_DETAIL_COND:S_ROLL_CHORDS);pal(UI_STUDIO_INDEX);ui_style=1;style_apply();ui_prefs=large?PREF_LARGE:0;
  TSEL->p[P_SLEN]=64;for(uint32_t k=16;k<64;k++)TSEL->step[k]=TSEL->step[k%16];ui.bank=pos/16;ui.cursor=pos;
  song.playing=play!=0;TSEL->seq_idx=play==2?(pos+1)%64:pos;
  snprintf(name,sizeof name,"%s-step%u-large%d-play%u",detail?"detail":"roll",pos,large,play);cur_name=name;ui.force=1;
  uint32_t before=cut_music_hash();draw(S_ENV);assert(before==cut_music_hash());lint();cases++;
#ifndef BGIRL_CUT_BASELINE
  /* Independent screen readback: the selected bank frame remains magenta even
   * when the mint transport stripe shares or neighbours the selected step. */
  int32_t cx=(detail?SD_X0:PR_X0)+(int32_t)(pos%16)*(detail?12:PR_CW);
  int32_t cy=detail?26:PR_Y0-2;
  assert(swap16(host_screen[(graph_y()+cy)*240+cx])==T_ACCENT);
  if(play && TSEL->seq_idx/16==ui.bank){
   int32_t px=(detail?SD_X0:PR_X0)+(int32_t)(TSEL->seq_idx%16)*(detail?12:PR_CW)+6;
   assert(swap16(host_screen[(graph_y()+(detail?28:PR_Y0))*240+px])==T_PLAY);
  }
#endif
 }
 fprintf(rep,"CUT cases=%u findings=%u ink=%u neutral=%u align=%u lost=%u canvas=%zu\n",cases,nfind,nink,mono_bad,al_fail,al_lost,sizeof cv_px);
 fclose(rep);return nfind+nink+mono_bad+al_fail+al_lost?1:0;
}

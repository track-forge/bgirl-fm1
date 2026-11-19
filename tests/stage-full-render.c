/* SPDX-License-Identifier: GPL-3.0-only */
#define UI_LINT_LIMIT 12000
#define BGIRL_RENDER_LIBRARY
#include "../felucca/tests/ui_render.c"
#undef main
#include <assert.h>
static const char *const stage_names[]={"scope","scope-silent","scope-large-signal","scope-external","scope-unknown","tracks","tracks-stopped","tracks-bank2","tracks-muted","levels","levels-bank2","mixer","mixer-pan","mixer-bank2","mixer-muted-arm","mixer-midi","clock","clock-external","clock-min","clock-max","arp","arp-off","arp2","arp2-zero","scale","scale-chromatic","scale-root","chord","chord-wide","chord-off","chord-kit","chord-mono","dance","dance-ghost","dance-idle","dance-off","crew"};
static void stage_setup(uint32_t s){
 setup(S_HOME);ui_home_view=HV_SCOPE;ui.msg_t=0;
 if(s<5){if(s==1)memset((void*)scope_buf,0,sizeof scope_buf);if(s==2)for(uint32_t i=0;i<SCOPE_N;i++)scope_buf[i]=(i%40<20?32767:-32768);if(s==3)song.g[G_CLOCK]=1;if(s==4)ui_home_view=99;}
 else if(s<11){setup(s==6?S_HOME_TRACKS_STOPPED:s>=9?S_HOME_LEVELS:S_HOME_TRACKS);if(s==7||s==10)song.sel=5;if(s==8)for(int i=0;i<NTRK;i++){trk[i].p[P_MUTE]=1;trk[i].p[P_SLEN]=64;}ui.msg_t=0;}
 else if(s<16){setup(s==12?S_MIXER_PAN:s==15?S_MIXER_MIDI:S_MIXER);if(s==13)song.sel=5;if(s==14){song.rec=63;for(int i=0;i<NTRK;i++)trk[i].p[P_MUTE]=1;}ui.msg_t=0;}
 else if(s<20){setup(S_CLOCK);if(s==17)song.g[G_CLOCK]=1;if(s==18)song.g[G_BPM]=40;if(s==19){song.g[G_BPM]=240;song.g[G_SWING]=100;}ui.msg_t=0;}
 else if(s<24){setup(S_ARP);TSEL->p[P_AMODE]=AM_UP;TSEL->p[P_AOCT]=4;if(s==21)TSEL->p[P_AMODE]=0;if(s>=22){go_title("ARP 2");TSEL->p[P_ASWING]=127;TSEL->p[P_APROB]=s==23?0:100;TSEL->p[P_AHOLD]=1;}}
 else if(s<27){setup(S_SCL);if(s==25)TSEL->p[P_SCALE]=0;if(s==26)TSEL->p[P_ROOT]=11;}
 else if(s<32){setup(s==28?S_CHORD_WIDE:s==29?S_CHORD_OFF:s==30?S_CHORD_KIT:S_CHORD);if(s==31)TSEL->p[P_VOICE]=V_MONO;}
 else if(s<36){setup(S_ARP);go_title("ARP DANCE");TSEL->p[P_AMODE]=s==35?0:AM_UP;TSEL->nheld=s==34?0:3;dance_reset(song.sel);for(int k=0;k<7;k++)dance_trigger(song.sel);dance_style=s==33;}
 else{ui_home_view=HV_CREW;}
 ui.msg_t=0;
}
/* MIXER already consumes/resets peak telemetry. Exclude only that ephemeral
 * meter field; parameters, voices, steps, automation and musical state must not change. */
static uint32_t stage_state(void){track_t copy[NTRK];memcpy(copy,trk,sizeof copy);for(int i=0;i<NTRK;i++)copy[i].peak=0;uint32_t h=fnv(2166136261u,copy,sizeof copy);h=fnv(h,&motion,sizeof motion);h=fnv(h,&chain_config,sizeof chain_config);return fnv(h,chord_last,sizeof chord_last);}
int main(int argc,char **argv){const char *out=argc>1?argv[1]:"local/stage-rollout/after/native";char path[512],name[96];snprintf(path,sizeof path,"%s/report.txt",out);rep=fopen(path,"w");assert(rep);uint32_t cases=0;
 for(uint32_t p=0;p<NPALETTES;p++)for(int large=0;large<2;large++)for(int style=0;style<2;style++)for(uint32_t s=0;s<NELEM(stage_names);s++){
  stage_setup(s);pal(p);ui_style=style;style_apply();ui_prefs=large?PREF_LARGE:0;snprintf(name,sizeof name,"%s-%s-%s",stage_names[s],large?"large":"normal",style?"line":"flat");cur_name=name;
  uint32_t h=stage_state();draw(S_ENV);assert(h==stage_state());lint();mono_check();cases++;if(p==UI_STUDIO_INDEX)write_ppm(out,"STUDIO",name);
 }
 /* Sweep all selected tracks, 64-step banks, and playheads independently of screenshots. */
 for(int large=0;large<2;large++)for(int sel=0;sel<NTRK;sel++)for(int bank=0;bank<4;bank++)for(int pos=0;pos<16;pos++){
  stage_setup(5);pal(UI_STUDIO_INDEX);ui_prefs=large?PREF_LARGE:0;song.sel=sel;song.playing=1;
  for(int k=0;k<NTRK;k++){trk[k].p[P_SLEN]=64;trk[k].seq_idx=bank*16+pos;}
  cur_name="tracks-bank-playhead";uint32_t h=stage_state();draw(S_ENV);assert(h==stage_state());lint();cases++;
#ifndef BGIRL_STAGE_BASELINE
  for(int k=0;k<NTRK;k++)assert(swap16(host_screen[(graph_y()+29+k*15)*240+123+pos*6])==T_PLAY);
#endif
 }
#ifndef BGIRL_STAGE_BASELINE
 /* Geometry-aware checks on the actual framebuffer, not matching production hashes. */
 for(int large=0;large<2;large++){
  stage_setup(13);pal(UI_STUDIO_INDEX);ui_prefs=large?PREF_LARGE:0;draw(S_ENV);
  for(int c=2;c<4;c++)assert(swap16(host_screen[(graph_y()+70)*240+8+56*c])==T_BG);
  stage_setup(20);pal(UI_STUDIO_INDEX);ui_prefs=large?PREF_LARGE:0;ui.hot_t=0;draw(S_ENV);ui.force=0;
  uint64_t cost=blit_px;ui.frame+=20;draw_graph();assert(blit_px==cost);
  uint32_t sig=graph_signature();TSEL->nheld++;assert(sig!=graph_signature());
  stage_setup(16);pal(UI_STUDIO_INDEX);ui_prefs=large?PREF_LARGE:0;song.playing=0;draw(S_ENV);ui.force=0;
  cost=blit_px;ui.frame+=20;clk_step++;draw_graph();assert(blit_px==cost);
  song.playing=1;uint32_t previous=graph_signature();clk_step++;assert(previous!=graph_signature());
  for(int pos=0;pos<16;pos++){clk_step=pos;cur_name="clock-playhead";draw(S_ENV);lint();cases++;
   assert(swap16(host_screen[(graph_y()+45)*240+14+13*pos])==T_PLAY);}
 }
#endif
 fprintf(rep,"STAGE full cases=%u findings=%u ink=%u neutral=%u align=%u lost=%u canvas=%zu\n",cases,nfind,nink,mono_bad,al_fail,al_lost,sizeof cv_px);fclose(rep);return nfind+nink+mono_bad+al_fail+al_lost?1:0;
}

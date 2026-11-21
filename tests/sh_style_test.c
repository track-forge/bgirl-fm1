/* SPDX-License-Identifier: GPL-3.0-only */
#include "../felucca/web/emu/felucca_web.c"
#include <assert.h>
static void tick(unsigned n) {for(unsigned i=0;i<n;i+=CTL)web_render(CTL);}
static unsigned active(void) {unsigned v=0;for(unsigned t=0;t<NTRK;t++)for(unsigned k=0;k<NVOICE;k++)v+=!!trk[t].v[k].active;return v;}
static double render_energy(unsigned n) {
 double e=0;for(unsigned k=0;k<n;k+=CTL) {web_render(CTL);assert(active()<=8);
  for(unsigned i=0;i<CTL;i++){assert(isfinite(out_l[i])&&isfinite(out_r[i]));assert(fabsf(out_l[i])<=1&&fabsf(out_r[i])<=1);e+=out_l[i]*out_l[i]+out_r[i]*out_r[i];}}
 return e;
}
static project_t a,b;static project_store_t wire;
int main(void) {
 web_nor_erase();web_boot();tick(4096);
 assert(NTRK==6 && NVOICE==8 && BG_ANALOG.npresets==18);
 for(unsigned i=0;i<12;i++)assert(!memcmp(&ANALOG_PRESETS[i],&BG_ANALOG_PRESETS[i],sizeof(preset_t)));
 assert(!strcmp(BG_ANALOG_PRESETS[12].name,"BG CARR SAW"));
 assert(preset_cat(0,13)==CAT_SH_STYLE && preset_cat(0,14)==CAT_SH_STYLE && CAT_SIGNATURE==9 && CAT_SH_STYLE==10);
 assert(!strcmp(CAT_NAME[CAT_SH_STYLE],"SH-ISH"));
 assert(!strcmp(BG_ANALOG_PRESETS[13].name,"Crunchy Bass"));
 assert(!strcmp(BG_ANALOG_PRESETS[14].name,"Velvet Pulse"));
 apply_preset_to(&trk[1],12);assert(!strcmp(sound_group_name(&trk[1]),"ANALOG"));
 apply_preset_to(&trk[1],13);assert(!strcmp(sound_group_name(&trk[1]),"SH-ISH"));
 apply_preset_to(&trk[1],14);assert(!strcmp(sound_group_name(&trk[1]),"SH-ISH"));
 uint32_t total,k;list_set(1+CAT_SH_STYLE);preset_pos(&total);assert(total==5);
 for(unsigned i=0;i<total;i++) assert(preset_at(i,&k)==0 && k==13+i);
 list_set(0);
 const int lfo_depth[]={50,0,0,0,16}, env_depth[]={12,55,49,63,15};
 for(unsigned sound=0;sound<total;sound++)for(unsigned t=0;t<6;t++) {
  assert(!web_bgirl_project(0));tick(4096);track_select(t);
  song.g[G_ROUTE]=ROUTE_TRACK;for(unsigned i=0;i<6;i++)mch_set(i,i);
  trk[t].p[P_LEVEL]=98;trk[t].p[P_PAN]=9;trk[t].p[P_TRANS]=-12;
  trk[t].step[63]=(step_t){.n=1,.note={60},.time=ST_NOTE,.vel=100};
  assert(!web_bgirl_sh_style_sound(sound));tick(4096);
  assert(trk[t].eng_req==0 && trk[t].preset==13+sound && trk[t].p[P_VOICE]==V_MONO);
  assert(trk[t].p[P_LEVEL]==98&&trk[t].p[P_PAN]==9&&trk[t].p[P_TRANS]==-12&&trk[t].step[63].n==1);
  assert(!strcmp(sound_group_name(&trk[t]),"SH-ISH"));
  assert(trk[t].p[P_LD_FLT]==lfo_depth[sound]&&trk[t].p[P_ED_FLT]==env_depth[sound]&&trk[t].p[P_LWAVE]==1);
  for(unsigned k=0;k<P_COUNT;k++){const param_desc_t*d=track_desc(&trk[t],k);assert(trk[t].p[k]>=d->min&&trk[t].p[k]<=d->max);}
  assert(ml_learn(21,t,P_E4)==1);assert(midi_learned(21,63));assert(trk[t].p[P_E4]==63);
  assert(!motion_set_lock(&trk[t],0,P_E4,99));
  assert(!motion_set_event(&trk[t],63,P_E5,70));
  assert(!up_store_cat(0,"SH EDIT",0));trk[t].p[P_E4]=1;assert(!up_load(0));assert(trk[t].p[P_E4]==63);
  project_capture(&a);assert(proj_pack(&wire,&a)&&proj_import(&b,&wire,sizeof wire));a.sum=proj_sum(&a);assert(!memcmp(&a,&b,sizeof a));
  assert(!project_save_as(0,"SH-STYLE"));trk[t].p[P_E4]=1;project_load(0);tick(4096);assert(trk[t].p[P_E4]==63);
  /* Actual six-channel MIDI dispatch, followed by note-off while selection moves. */
  web_midi(0x90+t,60,100);assert(render_energy(8192)>.000001);track_select((t+1)%6);
  web_midi(0x80+t,60,0);tick(FS*4);assert(!active());
  track_select(t);trk_note_on(&trk[t],60,100);tick(512);trk_note_on(&trk[t],64,100);tick(512);assert(active()==1);
  apply_preset_to(&trk[t],12);tick(4096);assert(!active());assert(trk[t].p[P_LD_FLT]==0); /* preset panic and no leaked settings */
  apply_preset_to(&trk[t],13+sound);tick(4096);trk_note_on(&trk[t],60,100);assert(render_energy(2048)>0);
  panic_req |= 1u << t;tick(FS);assert(!active());
  /* STOP releases sequenced notes, not intentionally held live keyboard notes. */
  trk[t].step[0]=trk[t].step[63];seq_start();assert(render_energy(2048)>0);
  seq_stop();tick(FS*4);assert(!active()&&render_energy(4096)<1e-10);
 }
 assert(web_bgirl_sh_style_sound(total));
 /* Combined six-track sequencing on the unchanged eight-voice pool. */
 assert(!web_bgirl_six_project(0));tick(4096);
 for(unsigned t=0;t<6;t++){track_select(t);assert(!web_bgirl_sh_style_sound(t%2));}
 seq_start();assert(render_energy(FS*2)>.001);seq_stop();tick(FS*4);assert(!active());
 printf("SH-STYLE: old preset bytes, 5 sounds x6 tracks, descriptor ranges, MIDI/retrigger/locks/motion, user/project roundtrips, note-off/panic/STOP, shared8 PASS (%u bytes)\n",project_bytes());
}

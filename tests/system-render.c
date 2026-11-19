/* SPDX-License-Identifier: GPL-3.0-only */
#define UI_LINT_LIMIT 16000
#define BGIRL_RENDER_LIBRARY
#include "../felucca/tests/ui_render.c"
#undef main
#include <assert.h>
static const int scenes[]={S_MENU_MIDI,S_CONFIRM_SEQ,S_CONFIRM_PROJ,S_CONFIRM_USER,S_CONFIRM_PAT,S_CONFIRM_MOTION,S_CONFIRM_ERASE,S_MENU,S_MENU_SPEAKER,S_ABOUT,S_ABOUT_REC,S_ABOUT_CREDITS,S_ABOUT_END,S_UBOOT,S_CALIBRATION,S_FX_PEEK,S_FX_HELD,S_FX_WAIT,S_FX_ASSIGN,S_MENU_HOLD,S_MENU_LEDS,S_MENU_END,S_MENU_SYSTEM,S_MENU_SLIDE,S_GLO_PEEK,S_GLO_ACTIVE,S_GLO_EXT,S_SCL_PEEK,S_SCL_ACTIVE,S_EDIT_PEEK,S_EDIT_ACTIVE,S_LAYER_LOCK,S_LAYER_LOCK_FX,S_SEQ_TOOLS,S_SEQ_TOOLS_DRUM,S_SEQ_TOOLS_SONG,S_NAME_USER,S_NAME_TYPING,S_NAME_123,S_NAME_EMPTY,S_NAME_FULL,S_NAME_PLAYING,S_PROJECT_NAMED,S_SONG_NAMED,S_NAME_CAT,S_NAME_RANDOM,S_MENU_ANIM_IDLE,S_MOMENTARY,S_MOMENTARY_EDIT,S_REC_LAYER,S_REC_LAYER_SET,S_REC_LAYER_SONG,S_COUNTIN,S_COUNTIN2,S_MENU_AUDIO,S_HEAD_GLO_TURN,S_MENU_TUNE,S_MENU_INFO,S_MENU_HOME,S_HELP_HOME,S_HELP_USER,S_HELP_GRID,S_HELP_LFO,S_HELP_GLO,S_HELP_SEQ,S_MENU_HELP,S_LEARN_WAIT,S_LEARN_PICK,S_LEARN_SET,S_LEARN_LONG,S_GLO_LEARN,S_MENU_LCLEAR,S_MENU_LCLEAR_NONE,S_HELP_DETAIL,S_GLO_FILL,S_MENU_SELECT,S_CONFIRM_ERASE_PROJ,S_CONFIRM_CLEAN,S_MENU_MIDI_TRACK,S_MENU_MIDI_TCH_UNUSED,S_MENU_MIDI_AUTO,S_USER,S_USER_FOOT,S_PRESETS,S_PRESETS_NOFAV,S_PRESETS_CAT,S_PRESETS_CAT_EMPTY,S_PHRASES,S_PROJECT,S_PROJECT_ERASE,S_TOOLS,S_TOOLS_CLEAN,S_MESSAGE,S_MESSAGE_KEY,S_MESSAGE_NOFILE};
static uint32_t music_state(void){
 uint32_t h=fnv(2166136261u,trk,sizeof trk);h=fnv(h,&motion,sizeof motion);h=fnv(h,&chain_config,sizeof chain_config);
 h=fnv(h,up_bank,sizeof up_bank);return fnv(h,proj_slot,sizeof proj_slot);
}
/* Read the real border pixels, then compare every intersecting text ink box.
 * This catches the PAD failure class, independently of canvas bounds lint. */
static void clear_frame(int x,int y,int w,int h,uint16_t color){
 assert(swap16(host_screen[(y+h/2)*240+x])==color);
 assert(swap16(host_screen[(y+h/2)*240+x+w-1])==color);
 for(uint32_t i=0;i<nscr;i++){
  const rbox_t *b=&scr[i];if(b->flags&4u)continue;
  if(b->x0>=x+w||b->x1<=x||b->y0>=y+h||b->y1<=y)continue;
  if(!(b->x0>x&&b->x1<x+w&&b->y0>y&&b->y1<y+h)){
   fprintf(stderr,"%s frame touches '%s' ink %d,%d-%d,%d box %d,%d %dx%d\n",cur_name,b->s,b->x0,b->y0,b->x1,b->y1,x,y,w,h);assert(0);
  }
 }
}
int main(int argc,char **argv){
 const char *out=argc>1?argv[1]:"local/system-rollout/after/native";char path[512],name[96];
 snprintf(path,sizeof path,"%s/report.txt",out);rep=fopen(path,"w");assert(rep);uint32_t cases=0;
 for(uint32_t p=0;p<NPALETTES;p++)for(int large=0;large<2;large++)for(int style=0;style<2;style++)for(uint32_t s=0;s<NELEM(scenes);s++){
  setup(scenes[s]);pal(p);ui_style=style;style_apply();ui_prefs=(ui_prefs&~PREF_LARGE)|(large?PREF_LARGE:0);
  snprintf(name,sizeof name,"%s-%s-%s",S_NAME[scenes[s]],large?"large":"normal",style?"line":"flat");cur_name=name;
  uint32_t before=music_state();draw(scenes[s]);assert(before==music_state());lint();mono_check();cases++;
  if(p==UI_STUDIO_INDEX)write_ppm(out,"STUDIO",name);
 }
 /* Oversized saves and old-format refusal use the real project codec/UI. */
 for(uint32_t p=0;p<NPALETTES;p++)for(int large=0;large<2;large++)for(int style=0;style<2;style++)for(int edge=0;edge<2;edge++){
  setup(S_PROJECT);pal(p);ui_prefs=large?PREF_LARGE:0;ui_style=style;style_apply();ui.msg_t=0;
  if(!edge){
   for(uint32_t t=0;t<NTRK;t++)for(uint32_t k=0;k<NSTEP;k++){
    step_t *st=&trk[t].step[k];st->time=ST_NOTE;st->n=4;st->vel=100;
    st->note[0]=60;st->note[1]=64;st->note[2]=67;st->note[3]=72;
    st->flags=st->hit=st->acc=255;st->probability=100;
   }
   assert(project_bytes()>3840);uint32_t before=music_state();project_save_as(0,"FULL");assert(before==music_state());
  }else{
   memset(&proj_slot[1],0,sizeof proj_slot[1]);memcpy(&proj_slot[1],"FUN6",4);
   proj_old_slots|=2; /* models the flag set by NOR discovery; this harness has FELUCCA_FLASH=0 */
   uint32_t before=music_state();project_load(1);assert(before==music_state());assert(str_eq(ui.msg,"OLD FORMAT: OVERWRITE"));
  }
  snprintf(name,sizeof name,"%s-%s-%s",edge?"project-old-format":"project-full",large?"large":"normal",style?"line":"flat");cur_name=name;
  uint32_t before=music_state();draw(S_PROJECT);assert(before==music_state());lint();cases++;
  if(p==UI_STUDIO_INDEX)write_ppm(out,"STUDIO",name);
 }
#ifndef BGIRL_SYSTEM_BASELINE
 /* Every menu row, all USER slots and all project selections, including T5/6 banks. */
 for(int large=0;large<2;large++)for(int style=0;style<2;style++){
  for(uint32_t row=0;row<MI_COUNT;row++){
   setup(S_MENU);pal(UI_STUDIO_INDEX);ui.menu_sel=row;ui_prefs=large?PREF_LARGE:0;ui_style=style;style_apply();
   cur_name="menu-row-clearance";draw(S_MENU);lint();
   clear_frame(MR_X,mr_y(row-mtab_first(menu_tab())),MR_W,mr_h(),T_ACCENT);cases++;
  }
  for(uint32_t row=0;row<MI_VALUES;row++)if(menu_valued(row))for(uint32_t value=0;value<menu_n(row);value++){
   setup(S_MENU);ui.menu_sel=row;menu_put(row,value);pal(UI_STUDIO_INDEX);
   ui_prefs=(ui_prefs&~PREF_LARGE)|(large?PREF_LARGE:0);ui_style=style;style_apply();cur_name="menu-value-clearance";
   draw(S_MENU);lint();clear_frame(MR_X,mr_y(row-mtab_first(menu_tab())),MR_W,mr_h(),T_ACCENT);cases++;
   /* Setting labels are not free names: don't hide collisions with ellipsis. */
   int32_t row_y=mr_y(row-mtab_first(menu_tab()));
   for(uint32_t i=0;i<nscr;i++)if(scr[i].x0==MR_LX&&scr[i].y0>=row_y&&scr[i].y1<=row_y+mr_h())
    assert(!strchr(scr[i].s,ELLIPSIS));
  }
  for(uint32_t slot=0;slot<UP_SLOTS;slot++){
   setup(S_USER);pal(UI_STUDIO_INDEX);ui.uslot=slot;ui_prefs=large?PREF_LARGE:0;ui_style=style;style_apply();
   cur_name="user-slot-clearance";draw(S_USER);lint();
   int first=clamp((int)slot-2,0,UP_SLOTS-6);clear_frame(8,graph_y()+26+16*(slot-first),224,16,T_ACCENT);cases++;
  }
  for(uint32_t slot=0;slot<4;slot++){
   setup(S_PROJECT);pal(UI_STUDIO_INDEX);song.g[G_SLOT]=slot+1;ui_prefs=large?PREF_LARGE:0;ui_style=style;style_apply();
   cur_name="project-slot-clearance";draw(S_PROJECT);lint();clear_frame(8,graph_y()+28+23*slot,224,16,T_ACCENT);cases++;
  }
  /* Restored/custom colors and both track banks, on every scene. */
  for(uint32_t scene=0;scene<NELEM(scenes);scene++){
   setup(scenes[scene]);pal(UI_STUDIO_INDEX);uint16_t custom[5]={0x1082,0x3186,0xffff,0x7ff0,0xfabf};cpal_store(custom);ui_cpal_flags|=CPAL_ACTIVE;palette_apply();
   ui_prefs=(ui_prefs&~PREF_LARGE)|(large?PREF_LARGE:0);ui_style=style;style_apply();song.sel=5;
   snprintf(name,sizeof name,"custom-bank2-%s-%d-%d",S_NAME[scenes[scene]],large,style);cur_name=name;uint32_t before=music_state();draw(scenes[scene]);assert(before==music_state());lint();cases++;
  }
 }
 /* Every confirmation kind and target/bank, preserving all underlying music. */
 for(int large=0;large<2;large++)for(int style=0;style<2;style++)for(uint32_t kind=CF_CLEAR_SEQ;kind<=CF_LEARN_CLEAR;kind++)for(uint32_t target=0;target<NTRK;target++){
  setup(S_USER);pal(UI_STUDIO_INDEX);ui_prefs=large?PREF_LARGE:0;ui_style=style;style_apply();ui.confirm=kind;ui.confirm_trk=target;
  cur_name="confirmation-target";uint32_t before=music_state();draw(S_CONFIRM_SEQ);assert(before==music_state());lint();
  clear_frame(DLG_X+10,DLG_Y+80,90,26,T_LINE);cases++;
 }
 /* Cache inputs of the new tools diagram: moving focus and CLEAN must repaint. */
 setup(S_TOOLS);pal(UI_STUDIO_INDEX);uint32_t sig=graph_signature();ui.act=2;assert(sig!=graph_signature());
 sig=graph_signature();ui.tl_clean^=1;assert(sig!=graph_signature());
 /* Full-frame overlay exit equality (not just a count of clear calls). */
 uint16_t expected[240*240];
 for(uint32_t l=1;l<LAYER_N;l++){
  cur_name="overlay-exit";setup(S_USER);pal(UI_STUDIO_INDEX);ui.msg_t=0;draw(S_USER);memcpy(expected,host_screen,sizeof expected);
  nscr=npend=nrules=0;ui.layer=l;ui.force=1;ui_draw();nscr=npend=nrules=0;ui.layer=0;ui.force=1;ui_draw();
  /* Drawing increments frame but the stopped list and its register are static. */
  assert(!memcmp(expected+24*240,host_screen+24*240,sizeof expected-24*240*sizeof(uint16_t)));cases++;
 }
#endif
 fprintf(rep,"SYSTEM cases=%u scenes=%zu findings=%u ink=%u neutral=%u align=%u lost=%u canvas=%zu\n",cases,NELEM(scenes),nfind,nink,mono_bad,al_fail,al_lost,sizeof cv_px);fclose(rep);
 return nfind+nink+mono_bad+al_fail+al_lost?1:0;
}

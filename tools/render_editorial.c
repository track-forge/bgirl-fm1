/* SPDX-License-Identifier: GPL-3.0-only */
/* Offline editorial captures only; no changes to the shipped instrument. */
#include "../felucca/web/emu/felucca_web.c"
#include <assert.h>
int main(int argc,char **argv) {
 assert(argc==3); int mode=atoi(argv[1]);web_nor_erase();web_boot();web_master(650);
 assert(!web_bgirl_clarity(mode==0||mode==5||mode==6?4:1,2,80,mode==2?2:0,127));
 /* One identical first G6 chop, dry or pitched by a held MIDI 46 carrier. */
 memset(trk[3].step,0,sizeof trk[3].step);motion_set_enabled(&trk[3],0);
 trk[3].p[P_E1]=1;slc_vocal_select(&trk[3]);
 uint8_t hit[4]={60};put_step(&trk[3],0,1,hit,ST_NOTE,0);
 if(mode==3||mode==4) {
  for(int t=0;t<NTRK;t++) {trk[t].p[P_MUTE]=1;memset(trk[t].step,0,sizeof trk[t].step);}
  track_t *t=&trk[1];set_engine_of(t,BG_SIGNATURE_SELECT[mode-3][0]);apply_preset_to(t,BG_SIGNATURE_SELECT[mode-3][1]);
  t->p[P_MUTE]=0;t->p[P_LEVEL]=90;t->p[P_SLEN]=64;t->p[P_SDIV]=2;
  for(int k=0;k<32;k+=4){uint8_t n[4]={(uint8_t)(k<16?34:39)};put_step(t,k,1,n,ST_NOTE,0);}
  song.sel=1;
 }
 if(mode==6){trk[0].p[P_MUTE]=0;trk[0].p[P_LEVEL]=84;trk[0].p[P_SLEN]=64;trk[0].p[P_SDIV]=2;
  for(int k=0;k<64;k++){step_t *d=&trk[0].step[k];d->time=ST_NOTE;d->vel=96;d->hit=(k%16==0?1:0)|(k%16==4||k%16==12?6:0)|(k%4==2?8:0);}
 }
 ui.home=0;ui.page=page_first(FAM_EDIT);ui.force=1;
 for(int n=0;n<4096;n+=128)web_render(128);
 char path[1024];snprintf(path,sizeof path,"%s.raw",argv[2]);FILE *f=fopen(path,"wb");assert(f);
 seq_start();
 for(int n=0;n<FS*4;n+=128){int len=FS*4-n<128?FS*4-n:128;web_render(len);for(int i=0;i<len;i++){assert(isfinite(out_l[i])&&fabsf(out_l[i])<=1);fwrite(&out_l[i],4,1,f);}}
 fclose(f);seq_stop();
 snprintf(path,sizeof path,"%s.ppm",argv[2]);f=fopen(path,"wb");assert(f);fprintf(f,"P6\n240 240\n255\n");
 for(int i=0;i<240*240;i++){unsigned s=web_fb[i],v=(s>>8)|(s<<8);fputc(((v>>11)&31)*255/31,f);fputc(((v>>5)&63)*255/63,f);fputc((v&31)*255/31,f);}fclose(f);
 return 0;
}

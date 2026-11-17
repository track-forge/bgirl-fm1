/* SPDX-License-Identifier: GPL-3.0-only */
#include "../felucca/web/emu/felucca_web.c"
#include <assert.h>
static void tick(int frames){for(int n=0;n<frames;n+=CTL)web_render(CTL);}
static project_t q,r;static project_store_t wire;
int main(void) {
 web_nor_erase();web_boot();tick(1000);
 FILE*f=fopen("../tests/fixtures/clarity-legacy-B6S1.bin","rb");
 if(!f)f=fopen("tests/fixtures/clarity-legacy-B6S1.bin","rb");assert(f);
 assert(fread(&wire,1,sizeof wire,f)==sizeof wire);fclose(f);
 assert(proj_import(&q,&wire,sizeof wire));
 assert(q.t[3].p[P_VREL]==127 && q.t[3].p[P_VRANGE]==0 && q.t[5].p[P_E0]==3);
 assert(q.motion.event[0].place==383 && q.motion.event[0].param==(P_E0|MOTION_LOCK));
 assert(proj_pack(&wire,&q) && wire.raw[12]==127);assert(proj_import(&r,&wire,sizeof wire));
 q.sum=proj_sum(&q);assert(!memcmp(&q,&r,sizeof q));
 assert(BG_REL_MS[0]==5 && BG_REL_MS[127]==500);
 for(int i=1;i<128;i++)assert(BG_REL_MS[i]>BG_REL_MS[i-1]);
 for(int ms=50;ms<=150;ms+=50)printf("release %d: code%d actual%.6fms\n",ms,bg_release_value(ms),BG_REL_MS[bg_release_value(ms)]);
 for(int b=0;b<2;b++) {
  int16_t p[P_COUNT];for(int k=0;k<P_COUNT;k++)p[k]=TP[k].def;p[P_VBANDS]=b;
  bg_vocoder_reset();uint32_t h=2166136261u;
  for(int block=0;block<1000;block++){int32_t m[32],c[32];for(int i=0;i<32;i++){int n=block*32+i;m[i]=(n*7919%40001)-20000;c[i]=(n*9973%30001)-15000;}
   bg_vocoder_process(m,c,32,p);for(int i=0;i<32;i++){h^=(uint32_t)m[i];h*=16777619u;}}
  assert(h==(b?0xee7cbd4fu:0x875ba8e9u));
 }
 for(int mode=0;mode<5;mode++)for(int b=0;b<3;b++)for(int c=0;c<3;c++) {
  assert(!web_bgirl_clarity(mode,b,bg_release_value(150),c,117));tick(3000);
  assert(trk[2].eng_req==(c==0 ? 0 : c==1 ? 8 : 6));
  if(c==2)assert(trk[2].preset==0 && trk[2].p[P_SUS]==60 && trk[2].p[P_E2]==-12);
  assert(!song.playing && trk[3].p[P_VRANGE]==1 && trk[3].p[P_VWET]==117 && trk[3].p[P_VBANDS]==b);
  assert(!project_save_as(0,"CLARITY"));assert(!web_bgirl_project(0));project_load(0);tick(3000);
  assert(trk[3].p[P_VRANGE]==1 && trk[3].p[P_VWET]==117 && trk[3].p[P_VREL]==bg_release_value(150));
  transport_req=1;double energy=0;for(int n=0;n<FS;n+=CTL){web_render(CTL);for(int i=0;i<CTL;i++){assert(isfinite(out_l[i]) && fabsf(out_l[i])<=1);energy+=fabsf(out_l[i]);}}
  assert(energy>.001);seq_stop();tick(1000);assert(!bg_voc.hold);
 }
 assert(!web_bgirl_clarity(0,2,0,0,127));tick(3000);
 song.sel=3;assert(!up_store_cat(0,"REL TEST",0));trk[3].p[P_VRANGE]=0;assert(!up_load(0));assert(trk[3].p[P_VRANGE]==1 && trk[3].p[P_VREL]==0);
 assert(!motion_set_lock(&trk[3],63,P_VREL,127));assert(!motion_set_event(&trk[3],0,P_VREL,0));
 assert(up_rec(0)->ver==UP_VER_CLARITY);
 static up_rec_t old;old=*up_rec(0);old.ver=UP_VER_VOCAL;old.np=126;
 for(unsigned i=P_VRANGE;i<126;i++)old.packed[i]=old.packed[i+1];
 old.packed[P_VREL]=32+64;assert(up_valid(&old));
 int16_t values[P_COUNT];up_values(&old,values);assert(values[P_VRANGE]==0 && values[P_VREL]==32 && values[P_E0]==SLC_SRC_VOCALS);
 assert(ml_learn(21,3,P_VREL)==1);assert(midi_learned(21,127));assert(trk[3].p[P_VREL]==127);
 assert(midi_learned(21,0));assert(trk[3].p[P_VREL]==0);
 assert(motion_param(P_VBANDS) && !motion_param(P_VRANGE));
 for(int pass=0;pass<100;pass++) {int32_t m[32],c[32];for(int i=0;i<32;i++){m[i]=i%2?1048576:-1048576;c[i]=1048576;}
  trk[3].p[P_VBANDS]=pass%3;trk[3].p[P_VHOLD]=pass%2;bg_vocoder_process(m,c,32,trk[3].p);
  for(int i=0;i<32;i++)assert(m[i]>=-49152 && m[i]<=49152);
 }
 char val[8];const char *unit;trk[3].p[P_VRANGE]=1;
 param_format(track_desc(&trk[3],P_VREL),0,val,&unit);assert(!strcmp(val,"5") && !strcmp(unit,"ms"));
 param_format(track_desc(&trk[3],P_VREL),127,val,&unit);assert(!strcmp(val,"500"));
 trk[3].p[P_VRANGE]=0;param_format(track_desc(&trk[3],P_VREL),32,val,&unit);assert(!strcmp(val,"33"));
 printf("clarity: legacy fixture + exact8/12 hashes, release tables, 45 saved auditions, preset, motion/locks, transitions, bounded output PASS; DSP=%zu track=%zu project=%zu\n",sizeof bg_voc,sizeof(track_t),sizeof(project_t));
}

/* SPDX-License-Identifier: GPL-3.0-only */
/* Native production DSP, WAV or timed six-active-track stress. All I/O outside render. */
#include "../felucca/web/emu/felucca_web.c"
#include <assert.h>
static void le16(FILE*f,uint32_t v){fputc(v&255,f);fputc(v>>8&255,f);}
static void le32(FILE*f,uint32_t v){le16(f,v);le16(f,v>>16);}
static int cmp(const void*a,const void*b){double x=*(double*)a,y=*(double*)b;return (x>y)-(x<y);}
int main(int argc,char**argv) {
 assert(argc>=7);int mode=atoi(argv[2]),bands=atoi(argv[3]),ms=atoi(argv[4]),carrier=atoi(argv[5]),wet=atoi(argv[6]);
 int perf=mode==5;web_nor_erase();web_boot();assert(!web_bgirl_clarity(perf?2:mode,bands,bg_release_value(ms),carrier,wet));
 if(perf) {
  undo_depth++;
  for(int t=4;t<6;t++){set_engine_of(&trk[t],t==4?12:10);apply_preset_to(&trk[t],0);trk[t].p[P_MUTE]=0;trk[t].p[P_SLEN]=64;
   trk[t].p[P_CHOR]=30;trk[t].p[P_DLY]=20;
   for(int k=0;k<64;k+=4){uint8_t n[4]={60,64,67,0};put_step(&trk[t],k,t==4?2:1,n,ST_NOTE,0);}}
  undo_depth--;
 }
 for(int n=0;n<4096;n+=128)web_render(128);
 FILE*f=perf?0:fopen(argv[1],"wb");assert(perf||f);unsigned frames=FS*(perf?24:12);
 if(f){fwrite("RIFF",1,4,f);le32(f,36+frames*4);fwrite("WAVEfmt ",1,8,f);le32(f,16);le16(f,1);le16(f,2);le32(f,FS);le32(f,FS*4);le16(f,4);le16(f,16);fwrite("data",1,4,f);le32(f,frames*4);}
 static double times[10000];int count=0,maxvoices=0;double energy=0,peak=0;
 seq_start();
 for(unsigned n=0;n<frames;n+=128) {
  unsigned len=frames-n<128?frames-n:128;uint64_t start=now_ns();web_render(len);double ns=now_ns()-start;
  if(count<10000)times[count++]=ns;assert(voices_busy()<=8);if(voices_busy()>maxvoices)maxvoices=voices_busy();
  for(unsigned i=0;i<len;i++)for(int c=0;c<2;c++){float v=c?out_r[i]:out_l[i];assert(isfinite(v)&&fabsf(v)<=1);energy+=v*v;if(fabsf(v)>peak)peak=fabsf(v);if(f)le16(f,(uint16_t)(int16_t)lrintf(v*32767));}
 }
 seq_stop();if(f)fclose(f);qsort(times,count,sizeof(double),cmp);
 double sum=0;for(int i=0;i<count;i++)sum+=times[i];
 printf("{\"mode\":%d,\"bands\":%d,\"release_code\":%d,\"release_ms\":%.9g,\"carrier\":%d,\"wet\":%d,\"frames\":%u,\"rate\":%u,\"peak\":%.9g,\"rms\":%.9g,\"blocks\":%d,\"mean_us\":%.6f,\"p99_us\":%.6f,\"max_us\":%.6f,\"voices_max\":%d,\"dsp_bytes\":%zu,\"project_bytes\":%u}\n",mode,bands,bg_release_value(ms),BG_REL_MS[bg_release_value(ms)],carrier,wet,frames,FS,peak,sqrt(energy/(frames*2)),count,sum/count/1000,times[(count-1)*99/100]/1000,times[count-1]/1000,maxvoices,sizeof bg_voc,project_bytes());
}

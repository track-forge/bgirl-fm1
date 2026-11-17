/* SPDX-License-Identifier: GPL-3.0-only */
#include "../felucca/web/emu/felucca_web.c"
#include <assert.h>
int main(int argc,char**argv){assert(argc==2);FILE*f=fopen(argv[1],"wb");assert(f);
 for(int b=1;b<=2;b++) {
  web_nor_erase();web_boot();assert(!web_bgirl_clarity(0,b,94,0,127));
  for(int n=0;n<4096;n+=128)web_render(128);
  web_buttons(1<<B_PLAY);web_render(128);web_buttons(0);web_render(128);
  for(int n=0;n<88200;n+=128){int m=88200-n<128?88200-n:128;web_render(m);for(int i=0;i<m;i++){float pair[2]={out_l[i],out_r[i]};fwrite(pair,sizeof pair,1,f);}}
 }
 fclose(f);
}

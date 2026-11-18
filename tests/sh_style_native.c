/* SPDX-License-Identifier: GPL-3.0-only */
#include "../felucca/web/emu/felucca_web.c"
#include <assert.h>
int main(int argc,char**argv){assert(argc==2);FILE*f=fopen(argv[1],"wb");assert(f);
 for(unsigned sound=0;sound<2;sound++) {
  web_nor_erase();web_boot();assert(!web_bgirl_sh_style_sound(sound));
  for(int n=0;n<4096;n+=128)web_render(128);
  web_midi(0x90,60,100);
  for(int n=0;n<88200;n+=128){if(n==44160)web_midi(0x80,60,0);int m=88200-n<128?88200-n:128;web_render(m);for(int i=0;i<m;i++){float pair[2]={out_l[i],out_r[i]};fwrite(pair,sizeof pair,1,f);}}
 }fclose(f);
}

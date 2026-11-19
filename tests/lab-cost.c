/* SPDX-License-Identifier: GPL-3.0-only
 * Actual firmware blit/pixel receipts, not device timing. */
#include <stdint.h>
static uint64_t pixels;
static uint32_t blits;
#define GFX_HOOK_TEXT(x0,y0,x1,y1,s,flags) ((void)0)
#define GFX_HOOK_BEGIN() ((void)0)
#define GFX_HOOK_PIXELS(n) ((void)0)
#define GFX_HOOK_BLIT(x,y,r0) (pixels+=(uint64_t)cv_w*cv_h,blits++)
#include "../felucca/web/emu/felucca_web.c"
#include <assert.h>
#include <stdio.h>
static void blocks(int n){for(int i=0;i<n;i++)web_render(128);}
static void reset(void){pixels=blits=0;}
static void receipt(const char *title,int large,const char *scenario){printf("%s,%d,%s,%u,%llu\n",title,large,scenario,blits,(unsigned long long)pixels);}
int main(void){
 const char *titles[]={"ENV DEST","LFO","MOD","FX","VOCODER","INSERT","EDIT 1","FILTER","VOICE","OP ENV"};
 printf("page,large,scenario,blits,pixels\n");
 for(int large=0;large<2;large++)for(uint32_t s=0;s<sizeof titles/sizeof *titles;s++){
  web_nor_erase();web_boot();blocks(700);song.sel=s==4?3:s==9?4:1;
  uint32_t p;for(p=0;p<NPAGES;p++)if(str_eq(PAGES[p].title,titles[s]))break;assert(p<NPAGES);
  ui.home=0;ui.page=p;page_entered();ui_prefs=(ui_prefs&~PREF_LARGE)|(large?PREF_LARGE:0);ui.force=1;blocks(700);
  reset();blocks(350);receipt(titles[s],large,"idle44800frames");
  seq_start();blocks(350);reset();blocks(350);receipt(titles[s],large,"play44800frames");
  reset();for(int k=0;k<100;k++){web_enc(EN_K2,k%2?1:-1);blocks(1);}receipt(titles[s],large,"knob100x128frames");
  reset();for(int k=0;k<100;k++){web_enc(EN_ALGO,1);blocks(7);}receipt(titles[s],large,"track100x896frames");assert(blits>0 && pixels>0);
 }
 return 0;
}

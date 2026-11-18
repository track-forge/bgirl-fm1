/* SPDX-License-Identifier: GPL-3.0-only
 * Actual firmware blit receipts, not physical-device timing. */
#include <stdint.h>
static uint64_t pixels;static uint32_t blits;
#define GFX_HOOK_TEXT(x0,y0,x1,y1,s,flags) ((void)0)
#define GFX_HOOK_BEGIN() ((void)0)
#define GFX_HOOK_PIXELS(n) ((void)0)
#define GFX_HOOK_BLIT(x,y,r0) (pixels+=(uint64_t)cv_w*cv_h,blits++)
#include "../felucca/web/emu/felucca_web.c"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
static void blocks(int n){for(int i=0;i<n;i++)web_render(128);}
static void reset(void){pixels=blits=0;}
static int want_engine;
static void receipt(const char *title,int large,const char *scenario){printf("%s-%d,%d,%s,%u,%llu\n",title,want_engine,large,scenario,blits,(unsigned long long)pixels);}
int main(int argc,char **argv){
 assert(argc==3);int want=atoi(argv[1]),size=atoi(argv[2]);assert(want>=0&&want<8&&size>=0&&size<2);
 const char *titles[]={"EDIT 1","EDIT 2","EDIT 1","EDIT 2","SLICES","SLICER","LANES","LANES 2"};
 const uint8_t engines[]={4,4,13,13,13,0,10,10};
 printf("page,large,scenario,blits,pixels\n");
 for(int large=size;large==size;large++)for(int s=want;s==want;s++){
  want_engine=engines[s];web_nor_erase();web_boot();blocks(700);song.sel=1;set_engine_of(TSEL,engines[s]);
  if(s==5)TSEL->p[P_SLCR]=SL_GATE;
  uint32_t p;for(p=0;p<NPAGES;p++)if(str_eq(PAGES[p].title,titles[s]))break;assert(p<NPAGES);
  ui.home=0;ui.page=p;page_entered();
  ui_prefs=(ui_prefs&~PREF_LARGE)|(large?PREF_LARGE:0);ui.force=1;blocks(2000);
  assert(!ui.home && !ui.menu && !ui.layer && str_eq(cur_page()->title,titles[s]));
  reset();blocks(350);receipt(titles[s],large,"idle44800frames");
  seq_start();blocks(350);reset();blocks(350);receipt(titles[s],large,"play44800frames");
  reset();for(int k=0;k<100;k++){web_enc(s>=6||s==4?EN_K1:EN_K2,k%2?1:-1);blocks(1);}receipt(titles[s],large,"knob100x128frames");assert(blits>0&&pixels>0);
  reset();for(int k=0;k<100;k++){web_enc(EN_ALGO,1);blocks(7);}receipt(titles[s],large,"track100x896frames");assert(blits>0&&pixels>0);
 }
 return 0;
}

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
static const char *const names[]={"SCOPE","TRACKS","LEVELS","MIXER","CLOCK","ARP","ARP 2","SCL","CHORD","ARP DANCE","CREW"};
static void receipt(int page,int size,const char *scenario){printf("%s,%d,%s,%u,%llu\n",names[page],size,scenario,blits,(unsigned long long)pixels);}
int main(int argc,char **argv){
 assert(argc==3);int page=atoi(argv[1]),size=atoi(argv[2]);assert(page>=0&&page<11&&size>=0&&size<2);
 web_nor_erase();web_boot();blocks(700);song.sel=0;
 if(page<3||page==10){ui.home=1;ui_home_view=page==10?HV_CREW:page;}
 else{uint32_t p;for(p=0;p<NPAGES;p++)if(str_eq(PAGES[p].title,names[page]))break;assert(p<NPAGES);ui.home=0;ui.page=p;}
 page_entered();ui_prefs=(ui_prefs&~PREF_LARGE)|(size?PREF_LARGE:0);ui.force=1;blocks(2000);
 assert(!ui.menu&&!ui.layer);reset();blocks(350);receipt(page,size,"idle44800frames");
 seq_start();blocks(350);reset();blocks(350);receipt(page,size,"play44800frames");
 reset();for(int k=0;k<100;k++){web_enc(EN_K1,k%2?1:-1);blocks(1);}receipt(page,size,"knob100x128frames");assert(blits&&pixels);
 reset();for(int k=0;k<100;k++){web_enc(EN_ALGO,1);blocks(7);}receipt(page,size,"track100x896frames");assert(blits&&pixels);
 return 0;
}

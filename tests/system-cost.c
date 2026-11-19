/* SPDX-License-Identifier: GPL-3.0-only
 * Fresh-process firmware blit counts, NOT hardware timing or deadlines. */
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
static const char *const names[]={"USER","PRESETS","PHRASES","PROJECT","TOOLS","MENU","NAME","CONFIRM","GLO","FX","SEQ","REC","ABOUT"};
static void receipt(int page,int size,const char *scenario){printf("%s,%d,%s,%u,%llu\n",names[page],size,scenario,blits,(unsigned long long)pixels);}
int main(int argc,char **argv){
 assert(argc==3);int page=atoi(argv[1]),size=atoi(argv[2]);assert(page>=0&&page<13&&size>=0&&size<2);
 web_nor_erase();web_boot();blocks(700);song.sel=0;
 uint32_t p;for(p=0;p<NPAGES;p++)if(str_eq(PAGES[p].title,names[page<5?page:0]))break;assert(p<NPAGES);
 ui.home=0;ui.page=p;page_entered();ui_prefs=(ui_prefs&~PREF_LARGE)|(size?PREF_LARGE:0);
 if(page==5)ui.menu=1,ui.menu_sel=MI_COLOR;
 if(page==6)name_open(NK_USER_SAVE,0);
 if(page==7)ui.confirm=CF_OVR_PROJ;
 if(page>=8&&page<=11){const uint8_t layers[]={LAYER_GLO,LAYER_FX,LAYER_SEQ,LAYER_REC};ui.layer=layers[page-8];}
 if(page==12)ui.menu=2;
 ui.force=1;blocks(2000);reset();blocks(350);receipt(page,size,"idle44800frames");
 reset();ui.force=1;blocks(7);receipt(page,size,"forced896frames");
 reset();for(int k=0;k<100;k++){web_enc(EN_K1,k%2?1:-1);blocks(1);}receipt(page,size,"knob100x128frames");
 return 0;
}

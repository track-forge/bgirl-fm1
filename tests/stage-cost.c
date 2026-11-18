/* SPDX-License-Identifier: GPL-3.0-only
 * Deterministic host draw receipts. Not device timing or an audio benchmark. */
#include <stdint.h>
static uint64_t stage_pixels;
static uint32_t stage_blits;
#define GFX_HOOK_BLIT(x,y,r0) (stage_pixels+=(uint64_t)cv_w*cv_h,stage_blits++)
#include "../felucca/web/emu/felucca_web.c"
#include <stdio.h>
#include <assert.h>
static void blocks(int n){for(int i=0;i<n;i++)web_render(128);}
static void reset_cost(void){stage_pixels=stage_blits=0;}
static void receipt(const char *screen,const char *mode){printf("%s,%s,%u,%llu\n",screen,mode,stage_blits,(unsigned long long)stage_pixels);}
static void page(const char *name){for(int i=0;i<NPAGES;i++)if(!strcmp(PAGES[i].title,name)){ui.home=0;ui.page=i;page_entered();return;}assert(0);}
int main(void){
 printf("screen,scenario,blits,pixels\n");
 const char *names[]={"STAGE","LAB","CUT","ENV"};
 for(int s=0;s<4;s++){
  web_nor_erase();web_boot();blocks(700);
  if(s==1){song.sel=4;page("OPERATOR");}
  else if(s==2){song.sel=0;page("STEP");}
  else if(s==3)page("ENV");
  else {ui.home=1;ui_home_view=HV_CREW;}
  ui.force=1;blocks(700);reset_cost();blocks(350);receipt(names[s],"idle44800frames");
  seq_start();blocks(350);reset_cost();blocks(350);receipt(names[s],"play44800frames");
  reset_cost();for(int i=0;i<100;i++){web_enc(EN_K2,i%2?1:-1);blocks(1);}receipt(names[s],"knob100x128frames");
  reset_cost();for(int i=0;i<100;i++){web_enc(EN_ALGO,1);blocks(7);}receipt(names[s],"track100x896frames");
 }
 web_nor_erase();web_boot();blocks(700);assert(!web_bgirl_six_project(1));
 ui.home=1;ui_home_view=HV_CREW;seq_start();blocks(350);reset_cost();blocks(350);receipt("STAGE","dense44800frames");
 printf("canvas,%zu\n",sizeof cv_px);
 return 0;
}

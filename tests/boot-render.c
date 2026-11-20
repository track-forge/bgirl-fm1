/* SPDX-License-Identifier: GPL-3.0-only */
#define BGIRL_RENDER_LIBRARY
#include "../felucca/tests/ui_render.c"
#undef main
#include <assert.h>
int main(int argc, char **argv) {
    for (uint32_t p=0; p<NPALETTES+1; p++) {
        setup(S_SPLASH); pal(p<NPALETTES?p:0);
        if(p==NPALETTES) {
            const uint16_t custom[]={RGB(30,70,110),RGB(230,180,90),RGB(240,240,240),RGB(255,120,80),RGB(180,220,240)};
            cpal_store(custom); ui_cpal_flags|=CPAL_ACTIVE; palette_apply();
        }
        cur_name="alpha_boot"; draw(S_SPLASH); if(p<NPALETTES) mono_check();
        unsigned clear=0, solid=0, xmin=240,ymin=240,xmax=0,ymax=0;
        for(unsigned y=0;y<240;y++) for(unsigned x=0;x<240;x++) {
            unsigned a=0;
            if(x>=SPL_LOGO_X && x<SPL_LOGO_X+BG_LOGO_W && y>=SPL_LOGO_Y && y<SPL_LOGO_Y+BG_LOGO_H) {
                unsigned pos=(y-SPL_LOGO_Y)*BG_LOGO_W+x-SPL_LOGO_X;
                unsigned v=BG_LOGO_PIXELS[pos/2]; a=BG_LOGO_ALPHA[pos&1?v&15:v>>4];
            }
            if(!a) { assert(host_screen[y*240+x]==swap16(T_BG)); clear++; }
            if(host_screen[y*240+x]!=swap16(T_BG)) {
                solid++; if(x<xmin)xmin=x; if(x>xmax)xmax=x;
                if(y<ymin)ymin=y; if(y>ymax)ymax=y;
            }
        }
        assert(clear>48000 && solid>2000);
        /* Visible framebuffer, not merely the enclosing asset rectangle. */
        assert(abs((int)(xmin+xmax)-239)<=1);
        assert(abs((int)(ymin+ymax)-239)<=1);
        if(argc>1) write_ppm(argv[1],p<NPALETTES?UI_PALETTES[p].name:"CUSTOM","boot");
    }
    assert(!nfind);
    puts("Native boot: every palette + custom, clear pixels == T_BG, visible bounds centered PASS");
    return 0;
}

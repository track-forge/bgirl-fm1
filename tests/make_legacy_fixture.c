/* SPDX-License-Identifier: GPL-3.0-only
 * Build AGAINST BASELINE source, never the new writer. See VOCODER_VERIFICATION.md. */
#ifndef LEGACY_WEB_SOURCE
#error "define LEGACY_WEB_SOURCE to the verified pre-vocoder felucca_web.c"
#endif
#include LEGACY_WEB_SOURCE
#include <assert.h>
_Static_assert(P_COUNT==114 && PROJ_MAGIC==0x46554E41u,"fixture requires pre-vocoder FUNA source");
int main(int argc,char **argv) {
    assert(argc==2);web_nor_erase();web_boot();for(int i=0;i<400;i++)web_render(128);
    assert(!web_bgirl_project(1));for(int i=0;i<100;i++)web_render(128);
    trk[1].p[P_E4]=42;trk[1].step[1].note[0]=41;
    assert(!project_save_as(0,"LEGACY BGIRL"));FILE*f=fopen(argv[1],"wb");assert(f);
    assert(fwrite(proj_slot[0].raw,1,3840,f)==3840);fclose(f);
}

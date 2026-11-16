/* SPDX-License-Identifier: GPL-3.0-only */
#include "../felucca/tests/c1_host.h"
#define main hostsim_main
#include "../felucca/tests/hostsim.c"
#undef main
#include <assert.h>
static uint64_t scene(uint32_t mask,uint32_t order) {
    host_tracks_init();c1h_off=mask==0;char value[8];snprintf(value,sizeof value,"%u",mask);setenv("C1_MASK",value,1);
    for(uint32_t t=0;t<4;t++)host_preset(&trk[t],10,t==order?7:t==1-order?8:6);
    uint64_t h=1469598103934665603ull;int32_t out[CTL*2];
    for(uint32_t b=0;b<800;b++) {
        if(b%100==0) {
            /* USB event queue means dispatch occurs inside serial events_block. */
            midi_enqueue(9 | (0x90u+order)<<8 | 60u<<16 | 100u<<24,1);
            midi_enqueue(9 | (0x90u+1u-order)<<8 | 60u<<16 | 100u<<24,1);
        }
        if(b%100==20)midi_enqueue(8 | (0x80u+order)<<8 | 60u<<16,1);
        if(b%100==40)midi_enqueue(8 | (0x80u+1u-order)<<8 | 60u<<16,1);
        mix_block(out,CTL);
        for(uint32_t i=0;i<CTL*2;i++){assert(out[i]<32767 && out[i]>-32768);h=(h^(uint32_t)out[i])*1099511628211ull;}
    }
    return h;
}
static uint64_t isolated(uint32_t mask,uint32_t order) {
    int fd[2],status;assert(!pipe(fd));pid_t p=fork();assert(p>=0);
    if(!p){close(fd[0]);uint64_t h=scene(mask,order);assert(write(fd[1],&h,sizeof h)==sizeof h);_exit(0);}
    close(fd[1]);uint64_t h;assert(read(fd[0],&h,sizeof h)==sizeof h);close(fd[0]);waitpid(p,&status,0);assert(WIFEXITED(status)&&!WEXITSTATUS(status));return h;
}
int main(void) {
    for(uint32_t o=0;o<2;o++){uint64_t h=isolated(0,o);for(uint32_t m=1;m<15;m++)assert(h==isolated(m,o));}
    puts("signature dual: 28 bit-identical serial/threaded grouped-hat comparisons, both track orders PASS");return 0;
}

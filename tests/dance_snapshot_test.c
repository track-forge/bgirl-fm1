/* SPDX-License-Identifier: GPL-3.0-only */
#include <stdint.h>
#include <assert.h>
#include <pthread.h>
#include <stdio.h>
#define NTRK 4
#include "../felucca/firmware/src/arp_dance.h"
static uint32_t done;
static void *produce(void *unused) {
    (void)unused;
    for(uint32_t i=0;i<1000000;i++) {if(i%37u==0) dance_reset(i%4u);else dance_trigger(i%4u);}
    __atomic_store_n(&done,1,__ATOMIC_RELEASE);return 0;
}
int main(void) {
    pthread_t writer;uint32_t reads=0;
    assert(!pthread_create(&writer,0,produce,0));
    do {for(uint32_t i=0;i<4;i++) {
        uint32_t s=dance_read(i),p=s&15u,prev=(s>>4)&15u;
        assert(p<12 && prev<12 && s<1024);
        if(s&256u) assert((prev+1u)%12u==p && (s&512u));
        else assert(s==0u || s==512u);
        reads++;
    }} while(!__atomic_load_n(&done,__ATOMIC_ACQUIRE));
    pthread_join(writer,0);printf("dance atomic: million single-writer trigger/resets, %u concurrent coherent reads PASS (host only)\n",reads);
}

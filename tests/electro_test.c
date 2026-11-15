/* SPDX-License-Identifier: MPL-2.0
 * Copyright (C) 2026 Track Forge */
#include "../app/dsp/electro.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
static int32_t out[512];
static void render(electro *e,unsigned n,unsigned bpm,int mute)
{
    unsigned i;electro_render(e,out,n,4096,bpm,mute,100);
    for(i=0;i<n*2;i++)assert(out[i]>=-7470000&&out[i]<=7470000);
}
int main(void)
{
    electro e;unsigned b,i;clock_t start=clock();
    static const unsigned tempos[]={60,120,123,137,180};
    for(b=0;b<5;b++) {
        const unsigned frames=44100u*60u; /* one minute at each tempo, fractional steps */
        electro_init(&e);electro_transport(&e,1);
        for(i=0;i<frames;) {
            unsigned n=1u+(i%256u);if(n>frames-i)n=frames-i;
            render(&e,n,tempos[b],0);i+=n;
        }
        assert(e.phase==0);assert(e.steps==tempos[b]*4u);
        assert(e.step==(tempos[b]*4u)%16u);
        render(&e,1,tempos[b],0);assert(e.steps==tempos[b]*4u+1u);
        electro_transport(&e,0);render(&e,256,tempos[b],0);
        for(i=0;i<512;i++)assert(out[i]==0);
        electro_transport(&e,1);render(&e,1,tempos[b],0);
        assert(e.step==0&&e.steps==1); /* restart always step 1 */
    }
    /* Tempo changes retain fractional phase rather than rounding/restarting steps. */
    electro_init(&e);electro_transport(&e,1);
    render(&e,137,123,0);
    { unsigned phase=e.phase; render(&e,251,137,0);
      assert(e.phase==phase+251u*137u*4u); assert(e.steps==1); }
    electro_init(&e);electro_transport(&e,1);
    render(&e,256,120,1);assert(e.muted&&e.steps==1);
    {unsigned phase=e.phase;render(&e,256,120,0);assert(e.phase>phase&&!e.muted);}
    /* Simultaneous drums, bass and voiced key at full MASTER, high tone, every MIDI pitch. */
    for(b=0;b<128;b++) {
        electro_note(&e,-1,0,b+1,1);
        electro_note(&e,b,127,b+1,1);
        e.kick_env=e.snare_env=e.hat_env=e.bass_env=1;
        for(i=0;i<520;i++)render(&e,256,180,0);
    }
    electro_note(&e,-1,0,130,1);
    for(i=0;i<40;i++)render(&e,256,120,1);
    assert(!e.voice.active&&e.voice_gain==0);
    electro_panic(&e);render(&e,256,120,0);
    for(i=0;i<512;i++)assert(out[i]==0);
    /* Last-note selection, release decay and mode switch cannot leave a voice latched. */
    electro_note(&e,40,100,131,0);render(&e,256,120,0);
    electro_note(&e,45,100,132,0);render(&e,256,120,0);assert(e.note==45);
    electro_note(&e,-1,0,132,0);
    for(i=0;i<100;i++)render(&e,256,120,0);
    assert(e.bass_env==0);
    electro_note(&e,60,127,133,1);render(&e,256,120,0);assert(e.voice.active);
    electro_note(&e,-1,0,133,0);render(&e,256,120,0);assert(!e.voice.active);
    printf("electro: timing (5 x 60s), restart, mute/return, 128 pitches, mix bounds, release/panic pass; state %zu bytes; host CPU %.2fs\n",sizeof e,(float)(clock()-start)/CLOCKS_PER_SEC);
}

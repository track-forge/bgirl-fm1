/* SPDX-License-Identifier: MPL-2.0
 * Copyright (C) 2026 Track Forge */
#include "electro.h"
#include "fastmath.h"
#include <string.h>
static float electro_hz(int note)
{
    /* Positive exponents avoid pinned fastmath.h's signed-negative-shift UB.
     * MIDI 0 = 8.175799 Hz. All app callers constrain notes to 0..127. */
    return 8.175798916f * fm_exp2f((float)note / 12.0f);
}
void electro_init(electro *e)
{
    memset(e,0,sizeof *e); e->noise=0x6b8b4567u; e->note=-1;
    vocoder_init(&e->voice);
}
void electro_panic(electro *e)
{
    e->running=0; e->pending=0; e->phase=0; e->step=0;
    e->note=-1; e->bass_env=e->kick_env=e->snare_env=e->hat_env=0;
    e->bass_lp=e->noise_lp=0; e->voice_gain=0;
    vocoder_stop(&e->voice);
}
void electro_transport(electro *e,int run)
{
    electro_panic(e);
    if(run) {e->running=1; e->pending=1; e->steps=0;}
}
void electro_note(electro *e,int note,unsigned velocity,unsigned serial,int mode)
{
    if(note < -1 || note > 127) note=-1;
    mode=!!mode;
    int fresh=note>=0&&(e->note<0 || serial!=e->note_serial || mode!=e->mode);
    int legato=e->note>=0&&e->mode==mode;
    if(mode!=e->mode) {e->bass_env=0;vocoder_stop(&e->voice);e->voice_gain=0;}
    e->mode=(uint8_t)mode;
    e->note=note; e->note_serial=serial;
    e->velocity=(float)(velocity>127?127:velocity)/127.0f;
    if(fresh) {
        if(mode) {if(!legato)vocoder_trigger(&e->voice);}
        else {e->bass_hz=electro_hz(note);e->bass_env=e->velocity;}
    }
    if(note>=0&&!mode)e->bass_hz=electro_hz(note);
}
static void electro_step(electro *e)
{
    static const uint16_t kick=0x0941u, snare=0x1010u, hat=0xaaaau;
    static const int8_t bass[16]={28,-1,28,40,-1,-1,31,-1,28,-1,35,40,-1,31,-1,26};
    uint32_t bit=1u<<e->step;
    e->steps++;
    if(kick&bit){e->kick_env=1;e->kick_phase=0;}
    if(snare&bit){e->snare_env=1;e->snare_phase=0;}
    if(hat&bit)e->hat_env=1;
    if((e->note<0 || e->mode) && bass[e->step]>=0) {
        e->bass_hz=electro_hz(bass[e->step]); e->bass_env=0.75f;
    }
}
void electro_render(electro *e,int32_t *lr,unsigned n,unsigned master,unsigned bpm,int mute,unsigned tone)
{
    unsigned i;
    float target_gain=(float)(master>4096?4096:master)/4096.0f;
    float carrier_hz=e->note>=0?electro_hz(e->note):110.0f;
    float cutoff=0.015f+0.25f*(float)(tone>100?100:tone)/100.0f;
    if(bpm<60)bpm=60;
    if(bpm>180)bpm=180;
    e->muted=(uint8_t)!!mute;
    for(i=0;i<n;i++) {
        float noise,drums,bass,voice,s; int32_t out;
        if(e->running&&e->pending){e->pending=0;electro_step(e);}
        e->noise^=e->noise<<13;e->noise^=e->noise>>17;e->noise^=e->noise<<5;
        noise=(float)(e->noise&65535u)/32768.0f-1.0f;
        e->noise_lp+=0.2f*(noise-e->noise_lp);
        e->kick_phase+=(45.0f+125.0f*e->kick_env*e->kick_env)/44100.0f;
        if(e->kick_phase>=1)e->kick_phase-=1;
        e->snare_phase+=180.0f/44100.0f;if(e->snare_phase>=1)e->snare_phase-=1;
        drums=0.42f*fm_sinf(e->kick_phase*6.2831853f)*e->kick_env
             +0.22f*(0.75f*noise+0.25f*fm_sinf(e->snare_phase*6.2831853f))*e->snare_env
             +0.12f*(noise-e->noise_lp)*e->hat_env;
        e->kick_env*=0.9994f;e->snare_env*=0.9989f;e->hat_env*=0.994f;
        if(e->kick_env<1e-8f)e->kick_env=0;
        if(e->snare_env<1e-8f)e->snare_env=0;
        if(e->hat_env<1e-8f)e->hat_env=0;
        e->bass_phase+=e->bass_hz/44100.0f;if(e->bass_phase>=1)e->bass_phase-=1;
        bass=(2.0f*e->bass_phase-1.0f)*e->bass_env;
        e->bass_lp+=cutoff*(bass-e->bass_lp);
        /* Short decay, faster release when the hand lets go or the groove gate closes. */
        if((e->note<0 || e->mode) && (!e->running || e->phase>ELECTRO_CLOCK*3u/5u))e->bass_env*=0.995f;
        else e->bass_env*=0.99975f;
        if(e->bass_env<1e-8f)e->bass_env=0;
        if(fm_fabsf(e->bass_lp)<1e-10f)e->bass_lp=0;
        e->drum_gain+=((mute?0.0f:1.0f)-e->drum_gain)*0.004f;
        e->voice_gain+=((e->note>=0&&e->mode?e->velocity:0.0f)-e->voice_gain)*0.004f;
        voice=vocoder_next(&e->voice,carrier_hz)*e->voice_gain;
        if(e->note<0&&e->voice_gain<1e-6f){e->voice_gain=0;vocoder_stop(&e->voice);}
        e->gain+=(target_gain-e->gain)*0.004f;
        s=(drums*e->drum_gain+0.26f*e->bass_lp+0.32f*voice)*e->gain;
        /* Explicit pre-cast limiter prevents integer overflow; .90 leaves platform headroom. */
        if(s>0.9f)s=0.9f;
        if(s< -0.9f)s= -0.9f;
        out=(int32_t)(s*8300000.0f);lr[2*i]=out;lr[2*i+1]=out;
        if(e->running) {
            e->phase+=bpm*4u;
            if(e->phase>=ELECTRO_CLOCK){e->phase-=ELECTRO_CLOCK;e->step=(e->step+1u)&15u;e->pending=1;}
        }
    }
}

/* SPDX-License-Identifier: MPL-2.0
 * Copyright (C) 2026 Track Forge */
#include "vocoder.h"
#include "fastmath.h"
#include "../assets/phrase.h"
#include <string.h>
static float voice_filter_next(voice_filter *f, float x)
{
    float y=f->b0*x+f->b2*f->x2-f->a1*f->y1-f->a2*f->y2;
    f->x2=f->x1; f->x1=x; f->y2=f->y1; f->y1=y;
    return y;
}
void vocoder_init(vocoder *v)
{
    static const float centers[VOICE_BANDS]={180,250,350,490,680,950,1320,1840,2560,3560,4950,6500};
    unsigned i;
    memset(v,0,sizeof *v);
    for(i=0;i<VOICE_BANDS;i++) {
        float w=6.2831853f*centers[i]/44100.0f;
        float a=fm_sinf(w)/6.0f, norm=1.0f/(1.0f+a);
        voice_filter *f=&v->analysis[i];
        f->b0=a*norm; f->b2=-f->b0;
        f->a1=-2.0f*fm_cosf(w)*norm; f->a2=(1.0f-a)*norm;
        v->synthesis[i]=*f;
    }
}
void vocoder_trigger(vocoder *v)
{
    unsigned i;
    v->position=0; v->phase=0; v->active=1;
    for(i=0;i<VOICE_BANDS;i++) {
        voice_filter *a=&v->analysis[i],*s=&v->synthesis[i];
        a->x1=a->x2=a->y1=a->y2=0;
        s->x1=s->x2=s->y1=s->y2=0;
        v->envelope[i]=0;
    }
}
void vocoder_stop(vocoder *v) { v->active=0; }
float vocoder_next(vocoder *v, float hz)
{
    float mod,carrier,out=0;
    unsigned i;
    if(!v->active) return 0;
    if(v->position>=PHRASE_FRAMES*2u) {v->active=0;return 0;}
    mod=(float)phrase_pcm[v->position++/2u]/32768.0f;
    v->phase+=hz/44100.0f;
    if(v->phase>=1) v->phase-=1;
    carrier=2.0f*v->phase-1.0f;
    for(i=0;i<VOICE_BANDS;i++) {
        float band=voice_filter_next(&v->analysis[i],mod);
        float level=fm_fabsf(band), env=v->envelope[i];
        env+=(level-env)*(level>env?0.025f:0.0015f);
        v->envelope[i]=env;
        out+=voice_filter_next(&v->synthesis[i],carrier)*env;
    }
    return out*1.5f;
}

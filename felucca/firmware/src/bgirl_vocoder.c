/* SPDX-License-Identifier: GPL-3.0-only
 * BGIRL bounded two-input vocoder. No oscillator, sampler, allocation or I/O.
 * fx.c owns capture/join; the speech track retains its normal downstream FX. */
#ifndef BGIRL_VOCODER_DSP
#define BGIRL_VOCODER_DSP
#include "bgirl_vocoder_coeff.h"
#include "bgirl_vocoder_time.h"
typedef struct { float z1, z2; } bg_bq_t;
typedef struct {
    bg_bq_t analysis[24], synthesis[24];
    float env[24], held[24], applied[24], hp, carrier_env;
    uint32_t hold, mode;
} bg_vocoder_t;
#if defined(BGIRL_VOCODER_BENCH) || defined(BGIRL_VOCODER_FIXED_TEST)
typedef struct { int32_t z1, z2; } bg_qbq_t;
typedef struct {
    bg_qbq_t analysis[24], synthesis[24];
    int32_t env[24], held[24], applied[24], hp;
    uint32_t hold, mode;
} bg_fixed_t;
#endif
#ifdef BGIRL_VOCODER_BENCH
/* Modes are mutually exclusive and reset at a half-buffer boundary. */
static union {
    bg_vocoder_t reference;
    bg_fixed_t candidate;
} bg_bank;
#define bg_voc bg_bank.reference
#define bg_fixed bg_bank.candidate
#else
static bg_vocoder_t bg_voc;
#endif
static float bg_abs(float x) { return x < 0 ? -x : x; }
static float bg_filter(bg_bq_t *s, const float *c, float x)
{
    float y=c[0]*x+s->z1;
    s->z1=-c[3]*y+s->z2;
    s->z2=c[2]*x-c[4]*y;
    return y;
}
static void bg_vocoder_reset(void) { memset(&bg_voc,0,sizeof bg_voc); }
static void bg_vocoder_process(int32_t *mod, const int32_t *car, uint32_t n, const int16_t *p)
{
    uint32_t mode=p[P_VBANDS]>=2 ? 2 : p[P_VBANDS]>0 ? 1 : 0;
    uint32_t i,j,bands=mode==2 ? 24 : mode ? 12 : 8;
    const float (*coef)[5]=mode==2 ?
#ifdef BGIRL_VOCODER_NARROW24
        BG_COEF24 :
#else
        BG_COEF24_WIDE :
#endif
        mode ? BG_COEF12 : BG_COEF8;
    /* A bank change invalidates all held and recursive state, even while frozen. */
    if(bg_voc.mode!=mode) { bg_vocoder_reset(); bg_voc.mode=mode; }
    float mr=p[P_VMTRIM]/64.f, cr=p[P_VCTRIM]/64.f, wet=p[P_VWET]/127.f;
    float release=p[P_VRANGE] ? 1.f/(44.1f*BG_REL_MS[p[P_VREL]<0 ? 0 : p[P_VREL]>127 ? 127 : p[P_VREL]]) : 1.f/(44.1f*(p[P_VREL]+1));
    if (p[P_VHOLD] && !bg_voc.hold)
        for (j=0;j<bands;j++) bg_voc.held[j]=bg_voc.env[j];
    bg_voc.hold=p[P_VHOLD]!=0;
    for(i=0;i<n;i++) {
        float x=mod[i]*(1.f/32768)*mr, c=car[i]*(1.f/32768)*cr, y=0;
        bg_voc.carrier_env += (bg_abs(c)-bg_voc.carrier_env)*.01f;
        bg_voc.hp += (x-bg_voc.hp)*.25f;
        for(j=0;j<bands;j++) {
            float a=bg_abs(bg_filter(&bg_voc.analysis[j],coef[j],x));
            float target, gain=(j<bands/3 ? p[P_VLOW] : j<2*bands/3 ? p[P_VMID] : p[P_VHIGH])/64.f;
            bg_voc.env[j] += (a-bg_voc.env[j])*(a>bg_voc.env[j] ? .025f : release);
            target=bg_voc.hold ? bg_voc.held[j] : bg_voc.env[j];
            bg_voc.applied[j] += (target-bg_voc.applied[j])*.01f;
            y += bg_filter(&bg_voc.synthesis[j],coef[j],c)*bg_voc.applied[j]*gain;
        }
        y*=8.f;
        /* Consonants require live carrier energy; Freeze never retains raw audio. */
        if (bg_abs(c)>.0001f)
            y+=(x-bg_voc.hp)*(p[P_VCONS]/127.f)*.35f;
        y=x*(1-wet)+y*wet;
        if (!(y>=-1.5f && y<=1.5f)) y=y>0 ? 1.5f : y<0 ? -1.5f : 0;
        mod[i]=(int32_t)(y*32768);
    }
}
#endif

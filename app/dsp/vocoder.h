/* SPDX-License-Identifier: MPL-2.0
 * Copyright (C) 2026 Track Forge */
#pragma once
#include <stdint.h>
#define VOICE_BANDS 12
/* Fixed state; coefficients initialized off the audio path, no heap or decoding. */
typedef struct { float b0,b2,a1,a2,x1,x2,y1,y2; } voice_filter;
typedef struct {
    voice_filter analysis[VOICE_BANDS], synthesis[VOICE_BANDS];
    float envelope[VOICE_BANDS], phase;
    uint32_t position;
    int active;
} vocoder;
void vocoder_init(vocoder *v);
void vocoder_trigger(vocoder *v);
void vocoder_stop(vocoder *v);
float vocoder_next(vocoder *v, float hz);

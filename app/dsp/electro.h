/* SPDX-License-Identifier: MPL-2.0
 * Copyright (C) 2026 Track Forge */
#pragma once
#include <stdint.h>
#include "vocoder.h"
#define ELECTRO_CLOCK 2646000u /* 44100 * 60; exact integer fractional step clock */
typedef struct {
    vocoder voice;
    uint32_t phase, steps, noise;
    uint8_t step, running, muted, pending, mode;
    int32_t note;
    float velocity, bass_phase, bass_env, bass_hz, bass_lp;
    float kick_phase, kick_env, snare_phase, snare_env, hat_env, noise_lp;
    float gain, drum_gain, voice_gain;
    uint32_t note_serial;
} electro;
void electro_init(electro *e);
void electro_transport(electro *e, int run);
void electro_panic(electro *e);
void electro_note(electro *e, int note, unsigned velocity, unsigned serial, int mode);
/* n frames at 44100 Hz, 24-bit dual mono; no I/O, allocation, locks or logging. */
void electro_render(electro *e,int32_t *lr,unsigned n,unsigned master,unsigned bpm,int mute,unsigned tone);

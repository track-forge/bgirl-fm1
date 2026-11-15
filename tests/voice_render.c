/* SPDX-License-Identifier: MPL-2.0
 * Copyright (C) 2026 Track Forge */
#include "../app/dsp/vocoder.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
/* Raw signed 16-bit mono, converted to WAV by tests/render_voice.sh. */
int main(int argc,char **argv) {
    vocoder v; unsigned i; float hz=argc>1?strtof(argv[1],0):130.8128f,peak=0;
    clock_t start=clock(); vocoder_init(&v); vocoder_trigger(&v);
    for(i=0;i<44100*8;i++) {
        float s=vocoder_next(&v,hz); short out;
        if(!isfinite(s)||fabsf(s)>1.0f) {fprintf(stderr,"voice out of bounds %f\n",s);return 1;}
        if(fabsf(s)>peak)peak=fabsf(s);
        out=(short)(s*24000); fwrite(&out,2,1,stdout);
    }
    vocoder_stop(&v);
    if(vocoder_next(&v,hz)!=0||peak<0.001f)return 2;
    fprintf(stderr,"voice state %zu B; peak %.4f; host render+stdio %.2f ms/8s\n",sizeof v,peak,1000.0*(clock()-start)/CLOCKS_PER_SEC);
}

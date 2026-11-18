/* SPDX-License-Identifier: GPL-3.0-only */
/* Mega Lab MIDI interpretation. Normal editable steps/motion; no render work.
 * Source/deviations: docs/MEGA_JAM_DEMO_VERIFICATION.md. */
static void bgirl_mega_note(uint32_t track, uint32_t at, uint32_t note, uint32_t vel)
{
    trk[track].step[at]=(step_t){.n=1,.note={(uint8_t)note},.vel=(uint8_t)vel,.time=ST_NOTE};
}
static void bgirl_mega_demo(void)
{
    static const uint8_t riff[][3]={
        {0,45,40},{1,45,87},{6,55,84},{7,55,61},{8,57,45},{9,57,73},
        {16,45,74},{17,55,84},{18,57,62},{19,57,53},{22,45,19},
        {23,55,60},{24,55,48},{25,53,65},{27,55,35},{28,55,112}
    };
    static const uint8_t kick[]={0,2,6,8,10,14,16,18,22,26,31};
    static const uint8_t kv[]={120,100,120,104,120,104,120,100,120,120,100};
    static const uint8_t closed[]={0,1,2,6,8,14,16,17,18,22,26,29,30,31};
    static const uint8_t open[]={4,5,10};
    static const uint8_t rim[]={29,30,31};
    uint32_t i,k,group=0;
    for(i=0;i<NTRK;i++)motion_reset(&trk[i]);
    undo_depth++;
    set_engine_of(&trk[0],ENGI_DRUM);apply_preset_to(&trk[0],2);
    set_engine_of(&trk[1],0);apply_preset_to(&trk[1],2);
    set_engine_of(&trk[2],6);apply_preset_to(&trk[2],0);
    set_engine_of(&trk[3],13);apply_preset_to(&trk[3],0);
    set_engine_of(&trk[4],8);apply_preset_to(&trk[4],0); /* CLOUD PAD, one voice */
    for(i=0;i<NTRK;i++) {
        track_t *t=&trk[i];
        memset(t->step,0,sizeof t->step);
        t->p[P_SLEN]=64;t->p[P_SDIV]=2;t->p[P_CHRD]=CH_OFF;
        t->p[P_CHOR]=t->p[P_DLY]=t->p[P_REV]=t->p[P_DIST]=0;
        t->p[P_AMODE]=0;
    }
    song.g[G_BPM]=128;
    trk[0].p[P_LEVEL]=92;
    /* Shared step velocity104; per-lane accents127 approximate the loud kick
     * and backbeat. Keep snare+clap as explicit two-lane layering. */
    for(k=0;k<64;k++)trk[0].step[k].vel=104;
    for(k=0;k<64;k+=32) {
        for(i=0;i<NELEM(kick);i++) {
            step_t *s=&trk[0].step[k+kick[i]];s->hit|=1;
            if(kv[i]==120)s->acc|=1;
        }
        for(i=4;i<32;i+=8) {trk[0].step[k+i].hit|=6;trk[0].step[k+i].acc|=6;}
        for(i=0;i<NELEM(closed);i++)trk[0].step[k+closed[i]].hit|=8;
        for(i=0;i<NELEM(open);i++)trk[0].step[k+open[i]].hit|=16;
        for(i=0;i<NELEM(rim);i++)trk[0].step[k+rim[i]].hit|=64;
        /* GM49 deliberately omitted: no unintended TOM/BELL grid remap. */
        for(i=0;i<NELEM(riff);i++)bgirl_mega_note(1,k+riff[i][0],riff[i][1],riff[i][2]);
    }
    /* A softer final rim, then a breath instead of the final kick/hat stack. */
    trk[0].step[62].vel=88;
    trk[0].step[63].hit=64;trk[0].step[63].acc=0;trk[0].step[63].vel=78;
    trk[1].p[P_LEVEL]=79;trk[1].p[P_VOICE]=V_MONO;
    trk[1].p[P_REL]=20;trk[1].p[P_SGATE]=100;
    /* SGATE is track-wide (not motion-lockable): short source gates are
     * deliberately quantized to this common gate rather than new timing DSP. */
    trk[2].p[P_LEVEL]=90;trk[2].p[P_VOICE]=V_MONO;
    for(k=0;k<64;k++)trk[2].step[k].time=ST_TIE;
    bgirl_mega_note(2,0,45,100);bgirl_mega_note(2,32,52,100);
    /* Find G6 by label. Generated WHOLE parent order mirrors the group order. */
    for(i=1;i<NELEM(BG_VOCAL_GROUPS);i++)
        if(!strcmp(BG_VOCAL_VIEWS[i],"G6"))group=i;
    trk[3].p[P_E0]=SLC_SRC_VOCALS;trk[3].p[P_E1]=0;
    trk[3].p[P_E2]=0;trk[3].p[P_ROOT]=0;
    trk[3].p[P_E4]=SLC_ONE;trk[3].p[P_E6]=127;
    trk[3].p[P_ATK]=trk[3].p[P_DEC]=0;trk[3].p[P_SUS]=127;
    trk[3].p[P_VOICE]=V_MONO;trk[3].p[P_LEVEL]=96;
    trk[3].p[P_VCARR]=3;trk[3].p[P_VMON]=0;trk[3].p[P_VBANDS]=2;
    trk[3].p[P_VRANGE]=1;trk[3].p[P_VREL]=bg_release_value(120);
    trk[3].p[P_VWET]=117;trk[3].p[P_VCONS]=0;
    slc_vocal_select(&trk[3]);
    if(group) {
        /* Whole parent at bars1/3; just two replies in each following bar.
         * Ordinary VIEW events precede the notes, repeat deterministically. */
        bgirl_mega_note(3,0,60+group-1,110);
        bgirl_mega_note(3,32,60+group-1,106);
        bgirl_mega_note(3,20,61,98);bgirl_mega_note(3,26,64,92);
        bgirl_mega_note(3,52,62,100);bgirl_mega_note(3,58,65,88);
        motion_set_event(&trk[3],0,P_E1,0);motion_set_event(&trk[3],20,P_E1,group);
        motion_set_event(&trk[3],32,P_E1,0);motion_set_event(&trk[3],52,P_E1,group);
        motion_set_enabled(&trk[3],1);
    }
    trk[4].p[P_LEVEL]=55;trk[4].p[P_VOICE]=V_MONO;
    trk[4].p[P_REV]=43;trk[4].p[P_REL]=35;trk[4].p[P_SGATE]=127;
    /* Quantized0/6.951958/8.203610 beats ->0/28/33 steps. Mono overlap
     * is intentionally collapsed; ties retain the long source contours. */
    for(k=0;k<62;k++)trk[4].step[k].time=ST_TIE;
    bgirl_mega_note(4,0,50,72);bgirl_mega_note(4,28,60,70);bgirl_mega_note(4,33,64,68);
    /* Canonical dormant FM6 state too: startup/explicit reset have different
     * inherited preset history, but the saved demo must be exactly identical. */
    fm6_init();
    undo_depth--;
}

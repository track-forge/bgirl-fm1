/* SPDX-License-Identifier: GPL-3.0-only */
#include "../felucca/web/emu/felucca_web.c"
#include <assert.h>
static void tick(uint32_t n) { for(uint32_t i=0;i<n;i+=CTL) web_render(CTL); }
static void setup(uint32_t view) {
    assert(!web_bgirl_vocals(view ? 2 : 0,BG_VOCAL_PACK_V1 ? view : (view ? view : 1)));
    tick(4410);
}
static uint32_t duration(uint32_t note) {
    trk_note_on(&trk[3],note,110); trk_note_off(&trk[3],note);
    uint32_t n=0;
    do { tick(CTL); n+=CTL; assert(n<FS*10); } while(trk[3].v[0].active);
    return n;
}
static void entry_tests(void) {
    uint32_t tested=0;
    slc_dec_t grid; slc_dec_at(&grid,0,0);
    for(uint32_t k=0;k<SLC_GRID;k++) {
        uint32_t pos=slc_gpos(&SLC_VOCALS,k);
        while(grid.pos<pos) slc_dec_next(&SLC_VOCALS,&grid);
        assert(slc_dec_st(&grid)==SLC_VOCALS.grid[k]);
        assert(slc_state_at(&SLC_VOCALS,pos)==slc_dec_st(&grid));
    }
    for(uint32_t g=0;g<NELEM(BG_VOCAL_GROUPS);g++) {
        setup(g);
        assert(slc_vocal_match(&trk[3]));
        assert(!strcmp(track_desc(&trk[3],P_E1)->label,"VIEW"));
        assert(!strcmp(track_desc(&trk[3],P_E3)->names[0],"READY"));
        for(uint32_t j=0;j<BG_VOCAL_GROUPS[g].n;j++) {
            uint32_t e=BG_VOCAL_GROUPS[g].first+j,a,b,st;
            slc_bounds(&SLC_VOCALS,g,j,&a,&b,&st);
            assert(a==BG_VOCAL_ENTRIES[e].a && b==BG_VOCAL_ENTRIES[e].b);
            slc_dec_t d,all; slc_dec_at(&d,a,st); slc_dec_at(&all,0,0);
            while(all.pos<a) slc_dec_next(&SLC_VOCALS,&all);
            for(uint32_t i=a;i<b;i++) assert(slc_dec_next(&SLC_VOCALS,&d)==slc_dec_next(&SLC_VOCALS,&all));
            uint32_t frames=duration(60+j);
            printf("duration g%u j%u frames%u expected%u pos%u\n",g,j,frames,(b-a)*2,trk[3].v[0].ph[0]);
            assert(abs((int)frames-(int)((b-a)*2))<=HALF_FRAMES*2);
            assert(trk[3].v[0].ph[0]==b);
            for(int32_t n=0;n<128;n++) {
                trk[3].p[P_E2]=j; trk[3].p[P_ROOT]=0;
                assert(slc_note_slice(trk[3].p,n,BG_VOCAL_GROUPS[g].n)==(uint32_t)((n-60+(int32_t)j+128*(int32_t)BG_VOCAL_GROUPS[g].n)%(int32_t)BG_VOCAL_GROUPS[g].n));
            }
            trk[3].p[P_E2]=0; tested++;
        }
    }
    assert(tested==NELEM(BG_VOCAL_ENTRIES));
    printf("vocal entries: %u exact boundaries, decoder checkpoints, note maps and tap-to-end durations PASS\n",tested);
}
static void lifetime_tests(void) {
    setup(0); track_t *t=&trk[3];
    /* Tap queued while changing engines is retained, newest pending wins. */
    set_engine_of(t,0);tick(4410);t->eng_req=13;
    t->p[P_E0]=6;t->p[P_E1]=0;t->p[P_ATK]=0;t->p[P_SUS]=127;
    trk_note_on(t,60,110);trk_note_off(t,60);
    trk_note_on(t,61,110);trk_note_off(t,61);tick(1024);
    assert(t->engine==13 && t->v[0].active && t->v[0].note==61);
    seq_stop();tick(1024);
    setup(0);
    uint32_t normal=duration(60);
    t->p[P_E3]=24;t->p[P_E4]=SLC_LOOP;t->p[P_E5]=1;t->p[P_E6]=0;
    t->p[P_M1SRC]=1; t->p[P_ED_PIT]=127;
    assert(abs((int)duration(60)-(int)normal)<=CTL*4);
    t->p[P_E3]=-24; song.g[G_BPM]=60; assert(abs((int)duration(60)-(int)normal)<=CTL*4);
    song.g[G_BPM]=180; assert(abs((int)duration(60)-(int)normal)<=CTL*4);
    trk_note_on(t,60,110);tick(2048);uint32_t before=t->v[0].ph[0];
    trk_note_on(t,61,110);trk_note_on(t,62,110);trk_note_off(t,62);tick(HALF_FRAMES*2);
    assert(t->v[0].note==62);for(uint32_t i=1;i<NVOICE;i++)assert(!t->v[i].active);
    assert(speech_fade[3]<=128-CTL);
    trk_note_on(t,60,110);tick(2048);before=t->v[0].ph[0];
    t->p[P_E1]=1;trk_note_off(t,60);tick(HALF_FRAMES*2);assert(t->v[0].ph[0]>before && t->v[0].ph[2]==BG_VOCAL_ENTRIES[0].b);
    t->p[P_E0]=0;trk_note_off(t,60);tick(HALF_FRAMES*2);assert(t->v[0].active);
    seq_stop();tick(HALF_FRAMES*2);assert(!t->v[0].active);
    setup(0);trk_note_on(t,60,110);tick(HALF_FRAMES*2);panic_req=8;tick(HALF_FRAMES*2);assert(!t->v[0].active);
    setup(0);trk_note_on(t,60,110);tick(HALF_FRAMES*2);t->p[P_E1]=7;trk_note_on(t,60,110);tick(HALF_FRAMES*2);assert(!t->v[0].active);
    setup(0);bg_vocal_expected[3][0]^=1;trk_note_on(t,60,110);tick(HALF_FRAMES*2);assert(!t->v[0].active);
    assert(!strcmp(track_desc(t,P_E3)->names[0],"MISS"));
    puts("vocal lifetime: original rate despite pitch/loop/reverse/decay/BPM, newest/no fallback, captured view/source, STOP/panic/missing/invalid silence PASS");
}
static void save_tests(void) {
    uint32_t view=NELEM(BG_VOCAL_GROUPS)>1 ? 1 : 0;
    setup(view);trk[3].p[P_E2]=1;trk[3].p[P_ROOT]=5;
    trk[3].step[4].note[0]=63;
    static project_t saved,decoded;project_store_t wire;
    project_capture(&saved);assert(!memcmp(saved.vocal_id[3],BG_VOCAL_ID,8));
    assert(proj_pack(&wire,&saved));uint32_t magic;memcpy(&magic,wire.raw,4);assert(magic==PROJ_MAGIC);
    assert(proj_import(&decoded,&wire,sizeof wire));assert(!memcmp(decoded.vocal_id[3],BG_VOCAL_ID,8));
    assert(!project_restore_runtime(&decoded));tick(1024);assert(trk[3].p[P_E0]==6 && trk[3].p[P_E1]==view && trk[3].p[P_E2]==1 && trk[3].p[P_ROOT]==5 && trk[3].step[4].note[0]==63);
    assert(!up_store_cat(0,"VOCAL",0));assert(up_rec(0)->ver==UP_VER_VOCAL);
    assert(!up_load(0));tick(1024);assert(slc_vocal_match(&trk[3]) && !trk[3].p[P_VCARR]);
    decoded.vocal_id[3][0]^=1;decoded.sum=proj_sum(&decoded);
    assert(!project_restore_runtime(&decoded));tick(1024);assert(!slc_vocal_match(&trk[3]));
    trk_note_on(&trk[3],60,110);tick(1024);assert(!trk[3].v[0].active);
    assert(!up_store_cat(1,"MISSING",0));assert(!up_load(1));tick(1024);assert(!slc_vocal_match(&trk[3]));
    project_capture(&saved);assert(!memcmp(saved.vocal_id[3],decoded.vocal_id[3],8));
    /* Source changes by playback/motion do NOT rebind a missing bank. */
    trk[3].p[P_E0]=0;tick(HALF_FRAMES*2);trk[3].p[P_E0]=6;tick(HALF_FRAMES*2);
    trk_note_on(&trk[3],60,110);tick(1024);assert(!trk[3].v[0].active && !slc_vocal_match(&trk[3]));
    /* Explicit control selection binds (same entrypoint as UI/MIDI/editor). */
    motion_capture(&trk[3],P_E0,6);tick(HALF_FRAMES*2);assert(slc_vocal_match(&trk[3]));
    /* Base BREAK + a VOCALS source event retains the bank identity as well. */
    trk[3].p[P_E0]=0;assert(!motion_set_event(&trk[3],0,P_E0,6));
    project_capture(&saved);assert(!memcmp(saved.vocal_id[3],BG_VOCAL_ID,8));assert(proj_pack(&wire,&saved));
    memcpy(&magic,wire.raw,4);assert(magic==PROJ_MAGIC);
    up_rec(0)->packed[UP_VOCAL_AT]^=1;assert(!up_load(0));tick(1024);assert(!slc_vocal_match(&trk[3]));
    assert(!web_bgirl_project(1));tick(1024);project_capture(&saved);assert(proj_pack(&wire,&saved));memcpy(&magic,wire.raw,4);
    assert(magic==PROJ_MAGIC);
    assert(!web_bgirl_project(0));tick(1024);project_capture(&saved);assert(proj_pack(&wire,&saved));memcpy(&magic,wire.raw,4);assert(magic==PROJ_MAGIC);
    puts("vocal persistence: B6S1 exact ID/view/offset/root/steps; version6 presets; mismatch survives save/reload silently; explicit rebinding; factory non-vocal semantics retained PASS");
}
static void sequence_tests(void) {
    for(uint32_t bpm=60;bpm<=180;bpm+=120) {
        uint32_t view=NELEM(BG_VOCAL_GROUPS)>1 ? 1 : 0;
        setup(view);track_t *t=&trk[3]; song.g[G_BPM]=bpm;
        memset(t->step,0,sizeof t->step);t->p[P_SLEN]=16;
        for(uint32_t s=0;s<16;s+=4) {uint8_t note[4]={(uint8_t)(60+s/4%BG_VOCAL_GROUPS[view].n),0,0,0};put_step(t,s,1,note,ST_NOTE,0);}
        transport_req=1;
        uint32_t events=0,last=999,positions[4],note=999;
        for(uint32_t frame=0;frame<FS*5 && events<4;frame+=CTL) {
            tick(CTL);
            if(t->v[0].active && t->v[0].note!=note) {note=t->v[0].note;positions[events++]=frame;}
        }
        assert(events==4);uint32_t expected=FS*60/bpm;
        for(uint32_t i=1;i<4;i++)assert(abs((int)(positions[i]-positions[i-1])-(int)expected)<=HALF_FRAMES*2);
        printf("vocal sequence BPM%u: quarter-note spacing ~%u frames PASS\n",bpm,expected);
        seq_stop();tick(HALF_FRAMES*2);
    }
}
int main(int argc,char **argv) {
    web_nor_erase();web_boot();tick(FS);
    entry_tests();lifetime_tests();save_tests();sequence_tests();
    setup(0);
    trk[3].p[P_E0]=0;
    for(uint32_t cc=0;cc<128;cc++) { midi_cc_set(&trk[3],P_E1,cc); assert(trk[3].p[P_E1]==(int16_t)((cc*5+63)/127)); }
    trk[3].p[P_E0]=6;motion_capture(&trk[3],P_E0,6);
    for(uint32_t cc=0;cc<128;cc++) { midi_cc_set(&trk[3],P_E1,cc); assert(trk[3].p[P_E1]==(int16_t)((cc*(NELEM(BG_VOCAL_GROUPS)-1)+63)/127)); }
    puts("vocal/ordinary SLICE: all128 learned-CC VIEW/DIV mappings use source-specific bounds PASS");
    printf("structs: undo=%zu project=%zu\n",sizeof undo,sizeof(project_t));
    printf("resources: audio=%zu entries=%zu groups=%zu source=%zu fingerprint=%zu const-UI-descriptors=%zu undo-ID=%zu project-ID=%zu track=%zu bytes\n",sizeof BG_VOCAL_DATA,sizeof BG_VOCAL_ENTRIES,sizeof BG_VOCAL_GROUPS,sizeof SLC_VOCALS,sizeof bg_vocal_expected, sizeof BG_VOCAL_STARTS, sizeof undo.vocal_id, sizeof ((project_t*)0)->vocal_id,sizeof(track_t));
    if(argc>1) {
        for(uint32_t mode=0;mode<4;mode++) {
            assert(!web_bgirl_vocals(mode,slc_factory_vocal_group()));tick(4410);
            char path[512];snprintf(path,sizeof path,"%s/vocal-%u.wav",argv[1],mode);FILE *f=fopen(path,"wb");assert(f);uint32_t frames=FS*8;wav_hdr(f,frames);transport_req=1;double energy=0;
            for(uint32_t n=0;n<frames;n+=CTL) {
                web_render(CTL);
                for(uint32_t i=0;i<CTL && n+i<frames;i++) {
                    assert(isfinite(out_l[i]) && fabsf(out_l[i])<=1);energy+=fabsf(out_l[i]);
                    wav_put(f,(int32_t)(out_l[i]*32767),(int32_t)(out_r[i]*32767));
                }
            }
            fclose(f);assert(energy>1);seq_stop();tick(1024);
        }
        puts("four whole/chop dry/vocoded 8s host renders: finite/nonzero PASS (not listening approval)");
    }
    return 0;
}

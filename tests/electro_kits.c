/* SPDX-License-Identifier: GPL-3.0-only */
#include "../felucca/web/emu/felucca_web.c"
#include <assert.h>
#include <time.h>

#define B(lane) (1u << DV_##lane)
static const uint8_t beat[32] = {
    B(KICK)|B(HATC), B(HATC), B(RIM), B(KICK)|B(HATC),
    B(SNARE), B(HATO), B(HATC), B(KICK),
    B(HATC), B(CLAP), B(KICK)|B(HATC), B(TOM),
    B(SNARE)|B(CLAP)|B(HATC), B(HATC), B(BELL), B(HATC),
    B(KICK)|B(HATC), B(HATC), B(KICK)|B(RIM), B(HATC),
    B(SNARE)|B(CLAP), B(HATO), B(HATC), B(KICK),
    B(HATC), B(TOM), B(KICK)|B(HATC), B(RIM),
    B(SNARE)|B(CLAP), B(HATC), B(BELL)|B(TOM), B(HATC)
};
static const uint8_t bass[32] = {
    0,36,0,0,0,36,0,43,0,0,0,0,0,39,0,34,
    0,36,0,0,0,36,0,43,0,0,0,41,0,39,0,34
};

static void tick(uint32_t frames)
{
    for (uint32_t i=0; i<frames; i+=128) web_render(128);
}

static void select_kit(int kit)
{
    assert(!web_bgirl_project(0));
    apply_preset_to(&trk[0],DRUM_PRESET_BG_BREAK);
    trk[0].p[P_E0]=kit;
    trk[0].p[P_DLY]=trk[0].p[P_REV]=0;
    tick(2048);
}

static void acceptance(int kit, unsigned preset)
{
    select_kit(kit);
    apply_preset_to(&trk[0],preset);
    tick(256);
    assert(trk[0].p[P_E0]==kit && !drum_solo(trk[0].p));
    assert(!up_store(0,"KIT CHECK"));
    trk[0].p[P_E0]=DK_STD;
    assert(!up_load(0));
    assert(trk[0].p[P_E0]==kit);
    for (unsigned l=0; l<NLANE; l++) {
        int32_t pitch;
        assert(drum_lane(DRUM_LANE_NOTE[l])==l);
        assert(drum_gm(trk[0].p,DRUM_LANE_NOTE[l],&pitch)==DV_KTYPE(drum_model(kit),l));
        web_midi(0x90,DRUM_LANE_NOTE[l],110);
        tick(128);
        assert(voices_busy()==1);
        int heard=0;
        for (unsigned i=0;i<128;i++) heard |= fabsf(out_l[i])>.0001f;
        assert(heard);
        web_midi(0x80,DRUM_LANE_NOTE[l],0);
        tick(FS*2);
        assert(!voices_busy());
    }
    trk_note_on(&trk[0],46,110);
    tick(256);
    drum_lane_t *lanes=drum_kit_of(&trk[0]);
    assert(lanes[DV_HATO].v.live);
    trk_note_on(&trk[0],42,110);
    tick(FS/40);
    assert(!lanes[DV_HATO].v.live);
    tick(FS*2);
    tick(FS*70);
    felucca_init();
    assert(autosave_boot(1));
    tick(2048);
    assert(trk[0].p[P_E0]==kit);
}

static void audition(const char *dir, const char *name, int kit, int with_bass)
{
    select_kit(kit);
    apply_preset_to(&trk[1],13);
    trk[1].p[P_DLY]=trk[1].p[P_REV]=0;
    trk[1].p[P_LEVEL]=99;
    song.g[G_BPM]=120;
    for (unsigned t=0;t<2;t++) {
        trk[t].p[P_SLEN]=32;
        trk[t].p[P_SDIV]=2;
    }
    for (unsigned i=0;i<32;i++) {
        uint8_t hit=beat[i];
        trk[0].step[i]=(step_t){.time=ST_NOTE,.hit=hit,
            .acc=hit&(B(KICK)|B(SNARE)|B(CLAP)|B(TOM)|B(BELL)),.vel=(i%2)?83:106};
        if (with_bass && bass[i])
            trk[1].step[i]=(step_t){.time=ST_NOTE,.n=1,.note={bass[i]},.vel=102};
    }
    char path[512];
    snprintf(path,sizeof path,"%s/%s-%s.f32",dir,name,with_bass?"bass":"solo");
    FILE *f=fopen(path,"wb");
    assert(f);
    const uint32_t frames=FS*4;
    unsigned maxbusy=0, kills=voice_kills;
    float peak=0;
    seq_start();
    for (unsigned i=0;i<frames;i+=128) {
        unsigned n=frames-i<128?frames-i:128;
        web_render(n);
        unsigned busy=voices_busy();
        if (busy>maxbusy) maxbusy=busy;
        for (unsigned k=0;k<n;k++) {
            float mono=(out_l[k]+out_r[k])*.5f;
            assert(isfinite(mono) && fabsf(out_l[k])<.99f && fabsf(out_r[k])<.99f);
            if (fabsf(mono)>peak) peak=fabsf(mono);
            assert(fwrite(&mono,sizeof mono,1,f)==1);
        }
    }
    seq_stop();
    fclose(f);
    assert(peak>.01f && maxbusy<=NVOICE);
    printf("%s %s peak %.4f, voice max %u, steals %u\n",name,with_bass?"bass":"solo",peak,maxbusy,voice_kills-kills);
}

static void stress(int kit)
{
    select_kit(kit);
    trk[0].p[P_E5]=127;
    clock_t start=clock();
    const unsigned kills=voice_kills;
    float peak=0;
    for (unsigned j=0;j<240;j++) {
        for (unsigned l=0;l<NLANE;l++) trk_note_on(&trk[0],DRUM_LANE_NOTE[l],127);
        for (unsigned n=0;n<2048;n+=128) {
            web_render(128);
            assert(voices_busy()<=NVOICE);
            for (unsigned i=0;i<128;i++) {
                assert(isfinite(out_l[i]) && fabsf(out_l[i])<.99f);
                if (fabsf(out_l[i])>peak) peak=fabsf(out_l[i]);
            }
        }
    }
    printf("dense kit %d: %.3f ms per audio second, peak %.4f, steals %u\n",kit,
        (clock()-start)*1000.*FS/(CLOCKS_PER_SEC*240.*2048.),peak,voice_kills-kills);
    tick(FS*3);
    assert(!voices_busy());
}

int main(int argc,char **argv)
{
    assert(argc==2);
    web_nor_erase();
    web_boot();
    tick(60032);
    assert(trk[0].p[P_E0]==DK_BG_BREAK);
    static const int kits[]={DK_BG_BREAK,DK_808_RAW,DK_COLD_WIRE,DK_80,DK_TIGHT};
    static const char *const names[]={"bg-break","808-raw","cold-wire","felucca-80","felucca-tight"};
    for (unsigned i=0;i<NELEM(kits);i++) {
        if (i<3) acceptance(kits[i],DRUM_PRESET_BG_BREAK+i);
        audition(argv[1],names[i],kits[i],0);
        if (i<3) audition(argv[1],names[i],kits[i],1);
        stress(kits[i]);
    }
    return 0;
}

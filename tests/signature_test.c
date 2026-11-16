/* SPDX-License-Identifier: GPL-3.0-only */
#include "../felucca/web/emu/felucca_web.c"
#include <assert.h>
static void tick(uint32_t frames) { for(uint32_t i=0;i<frames;i+=CTL) web_render(CTL); }
static void sound(uint32_t tr,uint32_t engine,uint32_t preset) {
    set_engine_of(&trk[tr],engine);apply_preset_to(&trk[tr],preset);tick(4410);
}
static uint32_t active(track_t *t) { uint32_t n=0;for(uint32_t i=0;i<NVOICE;i++)n+=!!t->v[i].active;return n; }
static double energy(uint32_t frames) {
    double e=0;
    for(uint32_t f=0;f<frames;f+=CTL) {
        web_render(CTL);
        for(uint32_t i=0;i<CTL;i++) {
            assert(isfinite(out_l[i]) && isfinite(out_r[i]));
            assert(fabsf(out_l[i])<.99 && fabsf(out_r[i])<.99);
            e+=out_l[i]*out_l[i]+out_r[i]*out_r[i];
        }
    }
    return e;
}
static void percussion(void) {
    for(uint32_t p=4;p<9;p++) {
        double first=0;
        for(uint32_t note=0;note<128;note++) {
            sound(0,10,p);
            trk_note_on(&trk[0],note,96);trk_note_off(&trk[0],note);
            double e=energy(4096);assert(e>.00001);
            if(!note)first=e;else { if(fabs(e-first)/first>.02) fprintf(stderr,"preset%u note%u first%g energy%g\n",p,note,first,e); assert(fabs(e-first)/first<.02); } /* every pitch SAME identity */
            assert(active(&trk[0])==1);
            trk_note_on(&trk[0],(note+1)%128,127);tick(CTL);assert(active(&trk[0])==1);
            seq_stop();tick(CTL*4);assert(!active(&trk[0]));
        }
        for(uint32_t vel=32;vel<=127;vel=vel==32?64:vel==64?96:127) {
            sound(0,10,p);trk_note_on(&trk[0],60,vel);
            assert(energy(4096)>.00001);
            trk_note_off(&trk[0],60);assert(active(&trk[0])==1);
            if(vel==127)break;
        }
        for(uint32_t x=0;x<2;x++) {
            sound(0,10,p);trk[0].p[P_E1]=x?127:0;trk[0].p[P_E3]=x?127:0;
            trk_note_on(&trk[0],24,110);energy(FS);
        }
    }
    sound(0,10,6);trk_note_on(&trk[0],60,100);tick(1024);
    assert(drum_kit_of(&trk[0])[DV_CLAP].v.live && drum_kit_of(&trk[0])[DV_SNARE].v.live);
    assert(active(&trk[0])==1); /* two layers, one shared voice */
    for(uint32_t b=0;b<2;b++) {
        sound(0,10,6);trk[0].p[P_LN1]=b?127:0;trk_note_on(&trk[0],60,100);assert(energy(8192)>0);
    }
    /* Real MIDI dispatch, including note-off after selecting another track. */
    for(uint32_t p=4;p<9;p++) for(uint32_t note=12;note<=108;note+=48) {
        sound(0,10,p);web_midi(0x90,note,96);tick(512);song.sel=1;
        web_midi(0x80,note,0);tick(512);assert(active(&trk[0])==1);seq_stop();
    }
    /* Panel keys and sequencer hit masks converge on selected identity too. */
    for(uint32_t p=4;p<9;p++) {
        sound(0,10,p);song.sel=0;web_keys(1u<<7);tick(1024);web_keys(0);tick(1024);
        assert(active(&trk[0])==1);seq_stop();tick(512);
        trk[0].step[0]=(step_t){.n=1,.note={101},.time=ST_NOTE,.vel=100};
        seq_start();tick(1024);assert(active(&trk[0])==1);seq_stop();tick(512);
        memset(trk[0].step,0,sizeof trk[0].step);
    }
    puts("drums: all128 pitches x5 identities, tap/velocity/extremes/retrigger/clap/MIDI/keys/sequence/STOP PASS");
}
static void choke(void) {
    for(uint32_t order=0;order<2;order++) {
        uint32_t c=order,o=1-order;
        sound(c,10,7);sound(o,10,8);
        trk_note_on(&trk[o],60,100);tick(4096);assert(active(&trk[o]));
        /* Put events through same render event block, in either track order. */
        web_midi(0x90+c,60,100);tick(2048);assert(!active(&trk[o]));
        seq_stop();tick(512);
        web_midi(0x90+o,61,100);web_midi(0x90+c,61,100);tick(2048);
        assert(!active(&trk[o]) && active(&trk[c]));
        seq_stop();tick(512);
        web_midi(0x90+c,62,100);web_midi(0x90+o,62,100);tick(2048);
        assert(!active(&trk[o]) && active(&trk[c]));
        seq_stop();tick(512);
        trk[o].p[P_E0]=DK_SIG_OPEN_FREE;
        web_midi(0x90+o,63,100);web_midi(0x90+c,63,100);tick(2048);assert(active(&trk[o]));
        seq_stop();tick(512);
        trk[o].p[P_E0]=DK_SIG_OPEN;
        web_midi(0x90+c,64,100);tick(512);web_midi(0x90+o,64,100);tick(512);
        assert(active(&trk[c]) && active(&trk[o])); /* OPEN never kills CLOSED; next-block OPEN allowed */
        panic_req=15;tick(512);assert(!active(&trk[o]));
    }
    puts("choke: previous tail, both same-block orders, track orders, opt-out, asymmetric, panic PASS");
}
static void persistence(void) {
    static project_t saved,decoded;project_store_t wire;
    for(uint32_t engine=10;engine<=12;engine+=2) {
        uint32_t first=engine==10?4:8,last=engine==10?9:13;
        for(uint32_t p=first;p<last;p++) {
            assert(!web_bgirl_project(0));tick(1024);sound(0,engine,p);song.sel=0;trk[0].p[P_E2]+=1;
            if(engine==12) { fm6_patch[0][FP_NAME]='X';fm6_pgen[0]++;fm6_adopt(0); }
            int16_t edited=trk[0].p[P_E2];uint8_t patch[155];memcpy(patch,fm6_patch[0],155);
            project_capture(&saved);assert(proj_pack(&wire,&saved));
            uint32_t magic;memcpy(&magic,wire.raw,4);assert(magic==(engine==10?PROJ_MAGIC_SIGNATURE:PROJ_MAGIC));
            assert(proj_import(&decoded,&wire,sizeof wire));assert(!project_restore_runtime(&decoded));tick(512);
            assert(trk[0].p[P_E2]==edited && trk[0].preset==p);
            assert(!memcmp(patch,fm6_patch[0],155));
            assert(!up_store(0,"SIG EDIT"));assert(!up_load(0));tick(512);
            assert(trk[0].p[P_E2]==edited);if(engine==12)assert(!memcmp(patch,fm6_patch[0],155));
        }
    }
    /* A project combining new drums and fingerprinted vocals retains both. */
    assert(!web_bgirl_project(1));tick(1024);sound(0,10,4);project_capture(&saved);
    assert(proj_pack(&wire,&saved));assert(proj_import(&decoded,&wire,sizeof wire));
    assert(!memcmp(saved.vocal_id,decoded.vocal_id,sizeof saved.vocal_id));
    assert(!project_restore_runtime(&decoded));tick(512);assert(slc_vocal_match(&trk[3]));
    puts("persistence: ten edited project/user sounds, OWN patches, FUND + vocal fingerprints PASS");
}
int main(void) {
    web_nor_erase();web_boot();tick(FS);assert(!web_bgirl_project(0));tick(4096);
    for(uint32_t i=0;i<NTRK;i++){trk_all_off(&trk[i]);trk[i].p[P_LEVEL]=0;}
    trk[0].p[P_LEVEL]=100;percussion();
    trk[1].p[P_LEVEL]=100;choke();
    for(uint32_t i=0;i<NTRK;i++)trk[i].p[P_LEVEL]=0;
    trk[0].p[P_LEVEL]=100;
    for(uint32_t p=8;p<13;p++) {
        sound(0,12,p);
        for(uint32_t note=p<10?24:48;note<=(p<10?52:72);note++) {
            trk_note_on(&trk[0],note,100);assert(energy(p<10?FS*8:FS/4)>.00001);
            trk_note_off(&trk[0],note);tick(FS*2);assert(!active(&trk[0]));
        }
    }
    puts("FM: sustained 8s every MIDI24..52 bass, every MIDI48..72 character, bounded/release PASS");
    persistence();
    puts("signature bank PASS");return 0;
}

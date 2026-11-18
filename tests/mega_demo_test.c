/* SPDX-License-Identifier: GPL-3.0-only */
#include "../felucca/web/emu/felucca_web.c"
#include <assert.h>
static project_t fresh, explicit, decoded;
static project_store_t wire;
static void tick(uint32_t frames) {for(uint32_t i=0;i<frames;i+=128)web_render(128);}
int main(void) {
    web_nor_erase();web_boot();tick(60032);
    project_capture(&fresh);
    assert(!web_bgirl_project(1));tick(4096);project_capture(&explicit);
    assert(!memcmp(&fresh,&explicit,sizeof fresh));
    assert(song.g[G_BPM]==128 && !song.playing && NVOICE==8);
    for(uint32_t t=0;t<6;t++)assert(trk[t].p[P_SLEN]==64 && trk[t].p[P_SDIV]==2);
    assert(trk[1].step[0].note[0]==21 && trk[1].step[25].note[0]==29 && trk[1].step[28].vel==112);
    assert(trk[0].step[4].hit==22 && trk[0].step[31].hit==72 && trk[0].step[63].hit==64);
    assert(trk[4].step[0].note[0]==50 && trk[4].step[28].note[0]==60 && trk[4].step[33].note[0]==64);
    assert(trk[5].eng_req==ENGI_DRUM && trk[5].preset==4 && trk[5].p[P_E0]==DK_SIG_KICK);
    assert(!strcmp(DRUM_PRESETS[trk[5].preset].name,"BG KICK"));
    static const uint8_t kick_steps[]={0,2,6,8,10,14,16,18,22,26,31};
    static const uint8_t kick_acc[]={1,0,1,0,1,0,1,0,1,1,0};
    for(uint32_t k=0;k<64;k++) {
        uint32_t expected=0,accent=0;
        for(uint32_t j=0;j<NELEM(kick_steps);j++)if(k%32==kick_steps[j] && k!=63){expected=1;accent=kick_acc[j];}
        assert(!(trk[0].step[k].hit&1) && !(trk[0].step[k].acc&1));
        assert(trk[5].step[k].hit==expected && trk[5].step[k].acc==accent);
        assert(!trk[5].step[k].n && trk[5].step[k].vel==104);
    }
    assert(trk[3].p[P_VBANDS]==2 && trk[3].p[P_VRANGE]==1 && trk[3].p[P_VWET]==84);
    assert(trk[3].p[P_VREL]==bg_release_value(120) && trk[3].p[P_VCARR]==3 && !trk[3].p[P_VMON]);
    assert(!strcmp(BG_VOCAL_VIEWS[1],"G6") && slc_vocal_match(&trk[3]));
    assert(motion.count==0 && !motion.on);
    assert(trk[3].p[P_E1]==0 && trk[3].p[P_E4]==SLC_ONE);
    assert(trk[2].eng_req==0 && trk[2].preset==12);
    assert(!strcmp(BG_ANALOG_PRESETS[trk[2].preset].name,"BG CARR SAW"));
    assert(trk[3].p[P_SLCR]==SL_GATE && trk[3].p[P_SLPAT]==3);
    assert(trk[3].p[P_SLRATE]==1 && trk[3].p[P_SLDEPTH]==127);
    assert(trk[3].p[P_DLY]==25);
    char delay_text[8];const char *delay_unit;
    param_format(&TP[P_DLY],trk[3].p[P_DLY],delay_text,&delay_unit);
    assert(!strcmp(delay_text,"20") && !strcmp(delay_unit,"%"));
    param_format(&TP[P_VWET],trk[3].p[P_VWET],delay_text,&delay_unit);
    assert(!strcmp(delay_text,"66") && !strcmp(delay_unit,"%"));
    for(uint32_t k=0;k<NSTEP;k++) {
        assert(trk[2].step[k].n==(k%16==0));
        if(k%16==0) {
            assert(trk[2].step[k].note[0]==(k%32==0?45:31));
            assert(trk[2].step[k].time==ST_NOTE);
        } else assert(trk[2].step[k].time==ST_TIE);
        assert(trk[3].step[k].n==(k==0 || k==32));
        if(trk[3].step[k].n)assert(trk[3].step[k].note[0]==60);
    }
    uint32_t bytes=project_bytes();assert(bytes<3000);
    assert(proj_pack(&wire,&fresh) && proj_import(&decoded,&wire,sizeof wire));
    assert(!memcmp(&fresh,&decoded,sizeof fresh));
    uint32_t start_kills=voice_kills,max_busy=0;uint64_t mod_energy=0;double energy=0;float peak=0;
    FILE *audio=fopen("build/host/mega-native.f32","wb");
    if(!audio)audio=fopen("felucca/build/host/mega-native.f32","wb");assert(audio);
    /* Four complete loops; capture the first two for exact WASM comparison. */
    web_buttons(1<<B_PLAY);web_render(128);web_buttons(0);web_render(128);assert(song.playing);
    for(uint32_t n=0;n<FS*30u;n+=128) {
        web_render(128);uint32_t busy=voices_busy();assert(busy<=8);if(busy>max_busy)max_busy=busy;
        assert(bg_bus.owner==3 && bg_bus.carrier==2);
        for(uint32_t k=0;k<CTL;k++)mod_energy+=(uint64_t)llabs(bg_bus.mod[k]);
        for(uint32_t i=0;i<128;i++) {
            float pair[2]={out_l[i],out_r[i]};
            if(n+i<FS*15u)assert(fwrite(pair,sizeof pair,1,audio)==1);
            for(uint32_t c=0;c<2;c++){float v=pair[c];assert(isfinite(v)&&fabsf(v)<=1);energy+=v*v;if(fabsf(v)>peak)peak=fabsf(v);}
        }
    }
    fclose(audio);assert(mod_energy && energy>1);
    printf("Mega: bytes=%u margin=%u maxvoices=%u steals=%u peak=%.8g RMS=%.8g mod=%llu\n",bytes,3840-bytes,max_busy,voice_kills-start_kills,peak,sqrt(energy/(FS*30u*2)),(unsigned long long)mod_energy);
    assert(voice_kills==start_kills);
    seq_stop();tick(FS*5);assert(!voices_busy() && !bg_voc.hold);
    for(uint32_t i=0;i<128;i++)assert(fabsf(out_l[i])+fabsf(out_r[i])<.0001f);
    seq_start();tick(FS);seq_stop();tick(FS*5);assert(!voices_busy());
    /* Exercise deliberate user overload, then normal panic/stop recovery. */
    trk[1].p[P_VOICE]=trk[4].p[P_VOICE]=V_POLY;
    for(uint32_t t=0;t<NTRK;t++)for(uint32_t j=0;j<8;j++)trk_note_on(&trk[t],60+j,100);
    tick(4096);assert(voices_busy()<=8 && voice_kills>start_kills);panic_req=(1u<<NTRK)-1;tick(FS*5);assert(!voices_busy());
    assert(!web_bgirl_project(1));tick(4096);
    trk[4].step[33].note[0]=65;trk[3].p[P_VWET]=111;
    trk[3].p[P_SLPAT]=5;trk[3].p[P_DLY]=30;
    assert(!project_save_as(0,"MEGA EDIT"));assert(!web_bgirl_project(0));
    for(uint32_t t=0;t<NTRK;t++)for(uint32_t k=0;k<NSTEP;k++)assert(!trk[t].step[k].n && !trk[t].step[k].hit);
    project_load(0);tick(4096);assert(trk[4].step[33].note[0]==65 && trk[3].p[P_VWET]==111);
    assert(trk[3].p[P_SLCR]==SL_GATE && trk[3].p[P_SLPAT]==5 && trk[3].p[P_DLY]==30);
    tick(FS*70);felucca_init();assert(autosave_boot(1));tick(4096);
    assert(trk[4].step[33].note[0]==65 && trk[3].p[P_VWET]==111 && !song.playing);
    assert(trk[3].p[P_SLCR]==SL_GATE && trk[3].p[P_SLPAT]==5 && trk[3].p[P_DLY]==30);
    puts("Mega: fresh/explicit equality, MIDI mapping, four loops/bounds/no steals, STOP/restart/overload, exact codec, edited save/autosave priority, blank PASS");
}

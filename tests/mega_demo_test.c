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
    assert(fresh.t[0].p[P_E0]==DK_BG_BREAK);
    assert(trk[0].p[P_E0]==DK_TIGHT);
    for (uint32_t t=0;t<NTRK;t++)
        assert(!memcmp(fresh.t[t].step,explicit.t[t].step,sizeof fresh.t[t].step));
    assert(fresh.t[5].p[P_E0]==DK_SIG_KICK);
    fresh=explicit; /* The named demo, not its fresh-start revoicing. */
    assert(song.g[G_BPM]==128 && !song.playing && NVOICE==8);
    for(uint32_t t=0;t<6;t++)assert(trk[t].p[P_SLEN]==64 && trk[t].p[P_SDIV]==2);
    assert(trk[1].step[0].note[0]==33 && trk[1].step[25].note[0]==41 && trk[1].step[28].vel==112);
    static const uint8_t bass_steps[]={0,1,6,7,8,9,16,17,18,19,22,23,24,25,27,28};
    static const uint8_t bass_notes[]={33,33,43,43,45,45,33,43,45,45,33,43,43,41,43,43};
    for(uint32_t step=0;step<64;step++) {
        uint32_t found=0;
        for(uint32_t i=0;i<NELEM(bass_steps);i++)if(step%32==bass_steps[i]) {
            found=1;assert(trk[1].step[step].note[0]==bass_notes[i]);
        }
        assert(trk[1].step[step].n==found);
    }
    assert(trk[1].eng_req==0 && trk[1].preset==13);
    assert(!strcmp(sound_group_name(&trk[1]),"SH-ISH"));
    assert(!strcmp(BG_ANALOG_PRESETS[13].name,"Crunchy Bass"));
    /* The demo uses the exact factory tone, with only track-owned mix/gate/FX setup. */
    int16_t demo_tone[P_COUNT];memcpy(demo_tone,trk[1].p,sizeof demo_tone);
    apply_preset_to(&trk[1],13);
    for(uint32_t p=P_ATK;p<P_COUNT;p++)if(p!=P_DLY && p!=P_REV)assert(trk[1].p[p]==demo_tone[p]);
    memcpy(trk[1].p,demo_tone,sizeof demo_tone); /* restore the demo's sends */
    char bass_value[8];const char *bass_unit;
    param_format(&TP[P_LEVEL],trk[1].p[P_LEVEL],bass_value,&bass_unit);
    assert(!strcmp(bass_value,"-6.0") && !strcmp(bass_unit,"dB"));
    param_format(&TP[P_DLY],trk[1].p[P_DLY],bass_value,&bass_unit);
    assert(!strcmp(bass_value,"13") && !strcmp(bass_unit,"%"));
    param_format(&TP[P_REV],trk[1].p[P_REV],bass_value,&bass_unit);
    assert(!strcmp(bass_value,"13") && !strcmp(bass_unit,"%"));
    assert(trk[0].step[4].hit==22 && trk[0].step[31].hit==72 && trk[0].step[63].hit==64);
    assert(trk[4].step[0].note[0]==38 && trk[4].step[28].note[0]==48 && trk[4].step[33].note[0]==52);
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
    assert(slc_vocal_match(&trk[3]));
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
    assert(trk[4].eng_req==ENGI_FM6 && trk[4].preset==4 && fm6_slot[4]==4);
    assert(!strcmp(FM6_PRESETS[4].name,"PAD") && trk[4].p[P_VOICE]==V_MONO);
    assert(trk[4].p[P_SLCR]==SL_GATE && trk[4].p[P_SLPAT]==1 && trk[4].p[P_SLRATE]==1);
    assert(trk[4].p[P_SLDEPTH]==95);
    param_format(&TP[P_SLDEPTH],trk[4].p[P_SLDEPTH],delay_text,&delay_unit);
    assert(!strcmp(delay_text,"75") && !strcmp(delay_unit,"%"));
    assert(trk[4].p[P_LEVEL]==99 && trk[3].p[P_LEVEL]==81);
    param_format(&TP[P_LEVEL],trk[4].p[P_LEVEL],delay_text,&delay_unit);
    assert(!strcmp(delay_text,"-6.5") && !strcmp(delay_unit,"dB"));
    param_format(&TP[P_LEVEL],trk[3].p[P_LEVEL],delay_text,&delay_unit);
    assert(!strcmp(delay_text,"-15.5") && !strcmp(delay_unit,"dB"));
    for(uint32_t k=0;k<NSTEP;k++) {
        assert(trk[2].step[k].n==(k%16==0));
        if(k%16==0) {
            assert(trk[2].step[k].note[0]==(k%32==0?45:31));
            assert(trk[2].step[k].time==ST_NOTE);
        } else assert(trk[2].step[k].time==ST_TIE);
        assert(trk[3].step[k].n==(k==0 || k==32));
        if(trk[3].step[k].n)assert(trk[3].step[k].note[0]==60+(BG_VOCAL_PACK_V1 ? (k ? BG_VOCAL_GROUPS[0].n-1 : 0) : slc_factory_vocal_group()-1));
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
    trk[4].step[33].note[0]=53;trk[3].p[P_VWET]=111;
    trk[3].p[P_SLPAT]=5;trk[3].p[P_DLY]=30;
    assert(!project_save_as(0,"MEGA EDIT"));assert(!web_bgirl_project(0));
    for(uint32_t t=0;t<NTRK;t++)for(uint32_t k=0;k<NSTEP;k++)assert(!trk[t].step[k].n && !trk[t].step[k].hit);
    project_load(0);tick(4096);assert(trk[4].step[33].note[0]==53 && trk[3].p[P_VWET]==111);
    assert(trk[3].p[P_SLCR]==SL_GATE && trk[3].p[P_SLPAT]==5 && trk[3].p[P_DLY]==30);
    tick(FS*70);felucca_init();assert(autosave_boot(1));tick(4096);
    assert(trk[4].step[33].note[0]==53 && trk[3].p[P_VWET]==111 && !song.playing);
    assert(trk[3].p[P_SLCR]==SL_GATE && trk[3].p[P_SLPAT]==5 && trk[3].p[P_DLY]==30);
    puts("Mega: fresh BREAK and original named split, MIDI mapping, four loops/bounds/no steals, STOP/restart/overload, exact codec, edited save/autosave priority, blank PASS");
}

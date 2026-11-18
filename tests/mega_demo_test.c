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
    assert(trk[0].step[4].hit==22 && trk[0].step[31].hit==73 && trk[0].step[63].hit==64);
    assert(trk[4].step[0].note[0]==50 && trk[4].step[28].note[0]==60 && trk[4].step[33].note[0]==64);
    assert(!trk[5].step[0].n && !trk[5].step[63].hit);
    assert(trk[3].p[P_VBANDS]==2 && trk[3].p[P_VRANGE]==1 && trk[3].p[P_VWET]==117);
    assert(trk[3].p[P_VREL]==bg_release_value(120) && trk[3].p[P_VCARR]==3 && !trk[3].p[P_VMON]);
    assert(!strcmp(BG_VOCAL_VIEWS[1],"G6") && slc_vocal_match(&trk[3]));
    assert(motion.count==0 && !motion.on);
    assert(trk[3].p[P_E1]==0 && trk[3].p[P_E4]==SLC_ONE);
    for(uint32_t k=0;k<NSTEP;k++) {
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
    assert(!project_save_as(0,"MEGA EDIT"));assert(!web_bgirl_project(0));
    for(uint32_t t=0;t<NTRK;t++)for(uint32_t k=0;k<NSTEP;k++)assert(!trk[t].step[k].n && !trk[t].step[k].hit);
    project_load(0);tick(4096);assert(trk[4].step[33].note[0]==65 && trk[3].p[P_VWET]==111);
    tick(FS*70);felucca_init();assert(autosave_boot(1));tick(4096);
    assert(trk[4].step[33].note[0]==65 && trk[3].p[P_VWET]==111 && !song.playing);
    puts("Mega: fresh/explicit equality, MIDI mapping, four loops/bounds/no steals, STOP/restart/overload, exact codec, edited save/autosave priority, blank PASS");
}

/* SPDX-License-Identifier: GPL-3.0-only */
#include "../felucca/web/emu/felucca_web.c"
#include <assert.h>
static void tick(uint32_t frames) { for(uint32_t i=0;i<frames;i+=CTL) web_render(CTL); }
static float block_peak(void) { float p=0;for(uint32_t i=0;i<CTL;i++){float a=fabsf(out_l[i])+fabsf(out_r[i]);if(a>p)p=a;assert(isfinite(a)&&a<=2);}return p; }
static void setup(uint32_t mode) { web_keys(0); web_buttons(0); assert(!bgirl_vocoder_setup(mode,0));tick(4410); }
static void trigger(uint32_t phrase) { trk_note_on(&trk[1],48,110);trk_note_on(&trk[3],60+phrase,110); }
int main(int argc,char **argv)
{
    web_nor_erase();web_boot();tick(44100);
    assert(P_COUNT==126 && P_E0==118);
    setup(0);trk_note_on(&trk[3],60,110);trk_note_off(&trk[3],60);tick(3200);assert(trk[3].v[0].active && trk[3].v[0].ph[0]>1000);
    setup(2);trk[3].p[P_E0]=0;trk[3].p[P_E4]=SLC_ONE;trk_note_on(&trk[3],60,110);tick(1024);seq_stop();tick(1024);assert(!trk[3].v[0].active);
    setup(0);trk[3].p[P_VOICE]=V_UNISON;trk_note_on(&trk[3],60,110);tick(3200);uint32_t a=trk[3].v[0].ph[0];
    setup(0);trk_note_on(&trk[3],64,110);tick(3200);assert(trk[3].v[0].ph[0]==a);
    trk_note_off(&trk[3],64);tick(3200);assert(trk[3].v[0].active && trk[3].v[0].ph[0]>a);
    trk_note_on(&trk[3],61,110);trk_note_on(&trk[3],62,110);trk_note_off(&trk[3],62);
    assert(trk[3].v[0].note==62);uint32_t voices=0;for(uint32_t i=0;i<NVOICE;i++)voices+=trk[3].v[i].active;assert(voices==1);
    puts("speech: original pitch, one-shot note-off, latest phrase/no fallback PASS");
    setup(2);trigger(0);tick(8820);assert(bg_bus.owner==3 && bg_bus.carrier==1 && bg_bus.mod_valid && bg_bus.car_valid);assert(block_peak()>.00001f);
    uint32_t ph=trk[3].v[0].ph[0];trk_note_on(&trk[1],55,110);assert(trk[3].v[0].ph[0]==ph);
    trk[3].p[P_VHOLD]=1;tick(1024);float held=bg_voc.held[4];assert(held>0);tick(88200);
    assert(bg_voc.hold && bg_voc.held[4]==held && !trk[3].v[0].active);
    trk[3].p[P_VHOLD]=0;tick(44100);assert(!bg_voc.hold && bg_voc.applied[4]<held*.01f);
    trk[3].p[P_VHOLD]=1;tick(1024);seq_stop();tick(1024);assert(!bg_voc.hold && !trk[3].p[P_VHOLD]);
    trk[3].p[P_VHOLD]=1;tick(1024);panic_req=15;tick(1024);assert(!bg_voc.hold);
    puts("vocoder: real carrier/pitch independence, Freeze beyond sample, release, stop/panic PASS");
    setup(2);trigger(1);tick(8820);trk[3].p[P_VCONS]=127;trk[3].p[P_VCARR]=4;tick(4410);assert(bg_bus.carrier==-1 && !bg_bus.car_valid);
    for(uint32_t i=0;i<CTL;i++)assert(bg_bus.mod[i]==0);
    assert(!strcmp(track_desc(&trk[3],P_VCARR)->label,"NO SRC"));
    trk[3].p[P_VCARR]=2;trk[1].p[P_MUTE]=1;tick(4410);for(uint32_t i=0;i<CTL;i++)assert(bg_bus.mod[i]==0 && bg_bus.car[i]==0); assert(block_peak()<.001f);
    trk[1].p[P_MUTE]=0;trk_note_on(&trk[1],48,110);trk_note_on(&trk[3],60,110);tick(8820);
    trk[3].p[P_VWET]=0; /* explicit dry speech, independent of carrier feed */
    tick(1024); assert(block_peak()>.00001f);
    trk[3].p[P_MUTE]=1;tick(4410);for(uint32_t i=0;i<CTL;i++)assert(bg_bus.mod[i]==0);assert(block_peak()<.001f);
    setup(2);trigger(0);tick(8820);trk[1].p[P_LEVEL]=0;tick(1024);
    int64_t carrierEnergy=0;for(uint32_t i=0;i<CTL;i++)carrierEnergy+=llabs(bg_bus.car[i]);assert(carrierEnergy>0 && block_peak()>.00001f);
    trk[1].p[P_LEVEL]=104;trk[1].p[P_CHOR]=100;tick(1024);
    for(uint32_t i=0;i<CTL;i++)assert(send_c[i]==0); /* carrier monitoring suppresses sends too */
    trk[3].p[P_PAN]=-64;tick(1024);
    for(uint32_t i=0;i<CTL;i++)assert(mix_r[i]==0);
    trk[3].p[P_LEVEL]=0;tick(1024);for(uint32_t i=0;i<CTL;i++)assert(mix_l[i]==0 && mix_r[i]==0);
    puts("routing: pre-level capture, carrier sends suppressed, speech level/pan PASS");
    puts("routing: invalid source, no stale/consonant bleed, mute, explicit dry balance PASS");
    setup(2);trk[3].p[P_VLOW]=90;trk[3].p[P_VHOLD]=1;tick(1024);
    static project_t saved;project_capture(&saved);assert(!saved.t[3].p[P_VHOLD]);
    assert(!project_save_as(0,"VOCODER"));assert(!web_bgirl_project(0));project_load(0);tick(4410);
    assert(trk[3].p[P_E0]==5 && trk[3].p[P_VCARR]==2 && trk[3].p[P_VLOW]==90 && !trk[3].p[P_VHOLD] && !bg_voc.hold);
    song.sel=3;assert(!up_store_cat(0,"SPEECH",0));assert(!up_load(0));tick(4410);assert(!trk[3].p[P_VCARR] && !trk[3].p[P_VHOLD]);
    uint32_t editSlot=999;assert(!slice_edit_begin(&editSlot) && editSlot==999);
    puts("storage: route/bank/controls roundtrip, load unfrozen, preset reference clearing PASS");
    FILE *legacy=fopen("tests/fixtures/bgirl-FUNA.bin","rb");
    if(!legacy)legacy=fopen("../tests/fixtures/bgirl-FUNA.bin","rb");assert(legacy);
    project_store_t old;assert(fread(&old,1,sizeof old,legacy)==sizeof old);fclose(legacy);
    assert(proj_import(&saved,&old,sizeof old));
    assert(!saved.t[3].p[P_VCARR] && !saved.t[3].p[P_VHOLD]);
    assert(!project_restore_runtime(&saved));tick(4410);
    assert(trk[1].p[P_E4]==42 && trk[1].step[1].note[0]==41 && !trk[3].p[P_VCARR]);
    puts("compatibility: authentic baseline FUNA fixture restores sounds/steps with vocoder OFF PASS");
    int32_t stressMod[CTL],stressCar[CTL];int16_t stress[P_COUNT];
    for(uint32_t k=0;k<P_COUNT;k++)stress[k]=TP[k].def;
    stress[P_VMTRIM]=stress[P_VCTRIM]=stress[P_VLOW]=stress[P_VMID]=stress[P_VHIGH]=127;
    for(uint32_t b=0;b<1000;b++) {
        for(uint32_t i=0;i<CTL;i++){stressMod[i]=(i&1)?1048576:-1048576;stressCar[i]=1048576;}
        bg_vocoder_process(stressMod,stressCar,CTL,stress);
        for(uint32_t i=0;i<CTL;i++)assert(stressMod[i]>=-49152 && stressMod[i]<=49152);
    }
    puts("DSP: 32000 stressed frames bounded and finite PASS");
    if(argc>1) {
        const char *names[]={"dry","8-band","12-band","12-consonants","12-chorus"};
        for(uint32_t mode=0;mode<5;mode++) {
            setup(mode==0 ? 0 : mode==1 ? 1 : 2);
            if(mode==3)trk[3].p[P_VCONS]=40;
            if(mode==4){trk[3].p[P_ITYPE]=8;trk[3].p[P_IMIX]=50;}
            char path[512];snprintf(path,sizeof path,"%s/%s.wav",argv[1],names[mode]);FILE*f=fopen(path,"wb");assert(f);uint32_t len=FS*9;wav_hdr(f,len);
            transport_req=1;
            for(uint32_t frame=0;frame<len;frame+=CTL) {web_render(CTL);block_peak();for(uint32_t i=0;i<CTL&&frame+i<len;i++)wav_put(f,(int32_t)(out_l[i]*32767),(int32_t)(out_r[i]*32767));}
            fclose(f);
        }
        printf("listening: five 9s comparable sequenced WAVs written; NOT human approval\n");
    }
    printf("state: DSP=%zu bytes; captures=%zu bytes; track=%zu bytes; bank=%zu bytes\n",sizeof bg_voc,sizeof bg_bus,sizeof(track_t),sizeof BG_SPEECH_DATA);
    assert(!web_bgirl_project(1));tick(4410);
    step_t electroSteps[3][NSTEP];int16_t electroSound[3][P_COUNT];
    for(uint32_t t=0;t<3;t++){memcpy(electroSteps[t],trk[t].step,sizeof trk[t].step);memcpy(electroSound[t],trk[t].p,sizeof trk[t].p);}
    for(uint32_t mode=1;mode<=2;mode++) {
        assert(!web_bgirl_vocoder(mode));tick(4410);
        assert(trk[2].engine==ENGI_FM6 && trk[2].preset==1 && trk[3].engine==13);
        assert(trk[3].p[P_VCARR]==3 && trk[3].p[P_VMON]==1 && trk[3].p[P_VBANDS]==(mode==2));
        for(uint32_t t=0;t<3;t++){assert(!memcmp(electroSteps[t],trk[t].step,sizeof trk[t].step));assert(!memcmp(electroSound[t],trk[t].p,sizeof trk[t].p));}
        assert(trk[3].step[0].note[0]==60 && trk[3].step[16].note[0]==61 && !trk[3].step[11].n);
        transport_req=1;tick(44100);assert(bg_bus.owner==3 && bg_bus.carrier==2 && block_peak()>.00001f);seq_stop();tick(4410);
    }
    uint32_t fx[4],nf=0;for(uint32_t i=0;i<NPAGES && nf<4;i++)if(PAGES[i].fam==FAM_FX)fx[nf++]=i;
    assert(nf==4 && !strcmp(PAGES[fx[0]].title,"FX") && !strcmp(PAGES[fx[1]].title,"VOCODER") && !strcmp(PAGES[fx[2]].title,"VOC INPUT") && !strcmp(PAGES[fx[3]].title,"VOC COLOR"));
    puts("demo: FX pages2-4 vocoder; electro drums/bass/bell preserved, T3 bell carrier/dry monitor ON, old VOX replaced PASS");
    return 0;
}

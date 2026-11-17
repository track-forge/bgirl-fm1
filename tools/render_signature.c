/* SPDX-License-Identifier: GPL-3.0-only
 * Native browser firmware renderer. Usage: render_signature OUTPUT_DIRECTORY
 * Compile using the same HAL/gen include paths as tests/signature_test.c. */
#include "../felucca/web/emu/felucca_web.c"
#include <assert.h>
#include <sys/stat.h>
static void tick(uint32_t n) { for(uint32_t i=0;i<n;i+=128)web_render(128); }
static void le16(FILE *f,uint32_t x){fputc(x&255,f);fputc((x>>8)&255,f);}
static void le32(FILE *f,uint32_t x){le16(f,x);le16(f,x>>16);}
static FILE *wav(const char *dir,const char *name,uint32_t frames){
    char path[1024];snprintf(path,sizeof path,"%s/%s.wav",dir,name);FILE *f=fopen(path,"wb");assert(f);
    fwrite("RIFF",1,4,f);le32(f,36+frames*4);fwrite("WAVEfmt ",1,8,f);le32(f,16);le16(f,1);le16(f,2);
    le32(f,FS);le32(f,FS*4);le16(f,4);le16(f,16);fwrite("data",1,4,f);le32(f,frames*4);return f;
}
static void block(FILE *f,uint32_t n){
    web_render(n);
    for(uint32_t i=0;i<n;i++) {
        assert(isfinite(out_l[i]) && isfinite(out_r[i]));
        assert(fabsf(out_l[i])<.99 && fabsf(out_r[i])<.99);
        le16(f,(uint16_t)(int16_t)lrintf(out_l[i]*32767));le16(f,(uint16_t)(int16_t)lrintf(out_r[i]*32767));
    }
}
int main(int argc,char **argv) {
    assert(argc==2);mkdir(argv[1],0755);web_nor_erase();web_boot();tick(FS);
    static const char *names[]={"sub-clean","sub-drive","kick","snare","clap","closed-hat","open-hat","iron-stab","electro-chord","chirp"};
    for(uint32_t s=0;s<10;s++)for(uint32_t wet=0;wet<2;wet++) {
        assert(!web_bgirl_signature(0));tick(FS*2);song.sel=0;
        assert(!web_bgirl_signature_sound(s));tick(FS);
        /* Sound-only MIDI100, bass36, pitched accents60. Wet does not obscure core. */
        trk[0].p[P_DLY]=wet?14:0;trk[0].p[P_REV]=wet?12:0;
        const uint32_t frames=FS*(s<2?10:4);char name[96];snprintf(name,sizeof name,"%02u-%s-%s",s,names[s],wet?"wet":"dry");FILE *f=wav(argv[1],name,frames);
        uint32_t note=s<2?36:60;
        if(s==8)web_midi(0x90,note,100);else trk_note_on(&trk[0],note,100);
        for(uint32_t k=0;k<frames;k+=128) {
            if(k< (s<2?FS*8:FS/8) && k+128>= (s<2?FS*8:FS/8)) {
                if(s==8)web_midi(0x80,note,0);else trk_note_off(&trk[0],note);
            }
            block(f,frames-k<128?frames-k:128);
        }
        fclose(f);printf("%s MIDI%u velocity100 frames%u\n",name,note,frames);
    }
    for(uint32_t mode=1;mode<=3;mode++)for(uint32_t bpm=90;bpm<=140;bpm=bpm==90?120:140) {
        assert(!web_bgirl_signature(mode));tick(FS*2);song.g[G_BPM]=bpm;
        uint32_t frames=(uint32_t)(FS*16.*60./bpm+.5);char name[80];snprintf(name,sizeof name,"demo-%c-%ubpm",'A'+mode-1,bpm);FILE *f=wav(argv[1],name,frames);
        seq_start();uint32_t maxvoices=0,kills=voice_kills;
        for(uint32_t k=0;k<frames;k+=128){block(f,frames-k<128?frames-k:128);uint32_t v=voices_busy();if(v>maxvoices)maxvoices=v;assert(v<=NVOICE);}
        seq_stop();fclose(f);printf("%s frames%u maxvoices%u steals%u\n",name,frames,maxvoices,voice_kills-kills);
        if(bpm==140)break;
    }
    return 0;
}

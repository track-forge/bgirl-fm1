/* SPDX-License-Identifier: GPL-3.0-only
 * Real fork/join rendering, all source orders/core masks; failed joins not replayed. */
#include "../felucca/tests/c1_host.h"
static uint32_t renders[6];
#define BG_RENDER_HOOK(t) (renders[(t)-trk]++)
#define main hostsim_main
#include "../felucca/tests/hostsim.c"
#undef main
#include <assert.h>
_Static_assert(sizeof renders / sizeof *renders == NTRK, "render hooks cover every track");
static uint64_t scene(int owner,int carrier,int mask,int bands,const char *failure) {
    host_tracks_init(); c1h_off=mask==0;
    char v[8];snprintf(v,sizeof v,"%d",mask);setenv("C1_MASK",v,1);
    if(failure) setenv("C1_FAILURE",failure,1); else unsetenv("C1_FAILURE");
    for(int k=0;k<6;k++) {host_preset(&trk[k],k==owner?13:0,0);trk[k].p[P_CHOR]=trk[k].p[P_DLY]=trk[k].p[P_REV]=0;trk[k].p[P_SUS]=127;trk[k].p[P_ED_FLT]=0;trk[k].p[P_E4]=127;trk[k].p[P_E5]=0;}
    if(getenv("CARRIER_ENGINE")) {
        int e=atoi(getenv("CARRIER_ENGINE"));host_preset(&trk[carrier],e,0);
        trk[carrier].p[P_CHOR]=trk[carrier].p[P_DLY]=trk[carrier].p[P_REV]=0;
    }
    trk[owner].p[P_E0]=5;trk[owner].p[P_E4]=0;trk[owner].p[P_VCARR]=carrier+1;trk[owner].p[P_VBANDS]=bands;trk[owner].p[P_VMON]=0;
    if(getenv("VOCAL_VIEW")) {trk[owner].p[P_E0]=6;trk[owner].p[P_E1]=atoi(getenv("VOCAL_VIEW"));slc_vocal_select(&trk[owner]);}
    trk[owner].p[P_M1SRC]=MS_LFO;trk[owner].p[P_M1DST]=MD_PAN;trk[owner].p[P_M1AMT]=30;
    trk[carrier].p[P_M2SRC]=MS_SH;trk[carrier].p[P_M2DST]=MD_DIST;trk[carrier].p[P_M2AMT]=10;
    trk_note_on(&trk[owner],60,110);trk_note_on(&trk[carrier],48,110);
    uint64_t h=1469598103934665603ull,vocalEnergy=0;int32_t out[CTL*2];
    for(int f=0;f<FS/4;f+=CTL) {
        memset(renders,0,sizeof renders);mix_block(out,CTL);
        for(int k=0;k<6;k++) assert(renders[k]<=1);
        if(!failure)for(int k=0;k<6;k++)assert(renders[k]==1);
        if(getenv("VOCAL_VIEW") && !failure)for(int i=0;i<CTL;i++)vocalEnergy+=(uint64_t)llabs(bg_bus.mod[i]);
        for(int i=0;i<CTL*2;i++) {assert(out[i]>=-32768 && out[i]<=32767);h=(h^(uint32_t)out[i])*1099511628211ull;}
    }
    if(failure && strcmp(failure,"reject")) {assert(c1h_off); assert(!bg_bus.mod_valid || bg_bus.owner==owner);}
    if(getenv("VOCAL_VIEW") && !failure)assert(vocalEnergy>0);
    return h;
}
static uint64_t isolated(int o,int c,int mask,int b,const char *failure) {
    int fd[2],status;assert(!pipe(fd));pid_t p=fork();assert(p>=0);
    if(!p) {close(fd[0]);uint64_t h=scene(o,c,mask,b,failure);assert(write(fd[1],&h,sizeof h)==sizeof h);_exit(0);}
    close(fd[1]);uint64_t h;assert(read(fd[0],&h,sizeof h)==sizeof h);close(fd[0]);waitpid(p,&status,0);assert(WIFEXITED(status)&&!WEXITSTATUS(status));return h;
}
int main(void) {
    int comparisons=0;
    for(int b=0;b<3;b++) for(int o=0;o<4;o++)for(int c=0;c<4;c++)if(o!=c) {
        uint64_t h=isolated(o,c,0,b,0);
        for(int m=1;m<15;m++){assert(h==isolated(o,c,m,b,0));comparisons++;}
    }
    int extraComparisons=0;
    for(int b=0;b<3;b++)for(int o=0;o<6;o++)for(int c=0;c<6;c++)if(o!=c && (o>=4 || c>=4)) {
        uint64_t ref=isolated(o,c,0,b,0);int masks[]={1,16,32,48,63};
        for(int m=0;m<5;m++){assert(ref==isolated(o,c,masks[m],b,0));extraComparisons++;}
        assert(ref==isolated(o,c,63,b,"reject"));isolated(o,c,63,b,"partial");isolated(o,c,63,b,"complete");
    }
    printf("six-track vocoder: %d extra ordered-pair/core-mask comparisons and T5/T6 failure recovery PASS\n",extraComparisons);
    int vocalComparisons=0;
    for(int view=0;view<2;view++) {
        setenv("VOCAL_VIEW",view ? "1" : "0",1);
        for(int b=0;b<3;b++)for(int o=0;o<6;o++) {
            if(o==1 || o==2)continue;
            int c=o<4?3-o:9-o;uint64_t ref=isolated(o,c,0,b,0);
            int masks[]={16,32,48,63};
            for(int m=0;m<(o<4?14:4);m++){assert(ref==isolated(o,c,o<4?m+1:masks[m],b,0));vocalComparisons++;}
            assert(ref==isolated(o,c,15,b,"reject"));
            isolated(o,c,15,b,"partial");isolated(o,c,15,b,"complete");
        }
    }
    unsetenv("VOCAL_VIEW");
    printf("vocal whole/chop dual: %d bit-identical ordered-pair/mask comparisons, 16 each reject/partial/complete failures, including T5/T6 and nonzero vocal input PASS\n",vocalComparisons);
    int engineComparisons=0;
    for(int e=0;e<NENGINES;e++) if(eng_ok(e)) {
        char ev[8];snprintf(ev,sizeof ev,"%d",e);setenv("CARRIER_ENGINE",ev,1);
        uint64_t ref=isolated(3,0,0,1,0);
        for(int m=1;m<15;m++){assert(ref==isolated(3,0,m,1,0));engineComparisons++;}
    }
    unsetenv("CARRIER_ENGINE");
    printf("eligible carrier engines: %d additional split comparisons PASS\n",engineComparisons);
    for(int o=0;o<4;o++)for(int c=0;c<4;c++)if(o!=c) {
        assert(isolated(o,c,0,1,0)==isolated(o,c,15,1,"reject"));
        isolated(o,c,15,1,"partial");isolated(o,c,15,1,"complete");
    }
    printf("vocoder dual: %d bit-identical 8/12/24-band ordered-pair/mask comparisons; 12 each rejected/partial/completed-failed jobs; <=1 render/track/block PASS\n",comparisons);
}

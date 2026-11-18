/* SPDX-License-Identifier: GPL-3.0-only
 * CUT sample/slice/lane presentation; existing decode caches and controls only. */
static void cut_missing(const char *message)
{
    cv_icon_mid(12,graph_ht/2,16,ICON_X_NOFILE,T_DIM,T_BG);
    cv_text(38,graph_ht/2-7,&AF_C,message,T_DIM);
}
static void cut_sample_wave(void)
{
    const smp_zone_t *z=sample_wave.zone;
    if(!z || !z->n){cut_missing("SAMPLE NOT FOUND");return;}
    if(!sample_wave.ready){cv_text(12,graph_ht/2-7,&AF_C,"DECODING SAMPLE",T_SUPPORT);return;}
    int32_t cy=(graph_ht-18)/2,a=cy-3;
    cv_rect(12,cy,216,1,T_RAISE);
    for(uint32_t i=0;i<SAMPLE_WAVE_COLS;i++){
        int32_t x=12+(int32_t)i*215/(SAMPLE_WAVE_COLS-1);
        cv_line(x,cy-sample_wave.hi[i]*a/sample_wave.peak,x,cy-sample_wave.lo[i]*a/sample_wave.peak,T_THEME);
    }
    if(z->looped && TSEL->p[P_E3]){
        int32_t x0=12+(int32_t)(z->ls*215u/z->n),x1=12+(int32_t)(z->le*215u/z->n);
        cv_rect(x0,0,1,2*cy,T_RHYTHM);cv_rect(x1,0,1,2*cy,T_RHYTHM);
        cv_rect(x0,2*cy,x1-x0+1,2,T_RHYTHM);
    }
    cv_text(12,graph_ht-12,&AF_C,z->looped&&TSEL->p[P_E3]?"LOOP REGION":"ONE SHOT",T_SUPPORT);
    char note[8];note_name(note,last_note);cv_text_r(228,graph_ht-12,&AF_C,note,T_TEXT,T_BG);
}
static void cut_slicer(const track_t *t)
{
    uint32_t pat=sl_pattern(t),mode=(uint32_t)t->p[P_SLCR],cur=sl[trk_index(t)].idx;
    int32_t h=graph_ht-23;
    for(uint32_t i=0;i<16;i++){
        int32_t x=14+(int32_t)i*13,open=h-t->p[P_SLDEPTH]*h/127;
        cv_rect(x,2,10,h,T_RAISE);
        if((pat>>i)&1u)cv_rect(x,2,10,h,mode==SL_OFF?T_DIM:T_RHYTHM);
        else if(mode==SL_STUT)for(int32_t y=3;y<h;y+=4)cv_rect(x,y,10,1,T_SUPPORT);
        else if(open>0)cv_rect(x,h-open+2,10,open,mode==SL_OFF?T_DIM:T_SUPPORT);
        if(mode!=SL_OFF && song.playing && i==cur){cv_frame(x-1,0,12,h+4,T_PLAY);cv_rect(x,h+6,10,2,T_PLAY);}
    }
    cv_text(12,graph_ht-12,&AF_C,mode==SL_OFF?"BYPASS":mode==SL_STUT?"LIVE / REPEAT":"OPEN / GATE",mode==SL_OFF?T_DIM:T_SUPPORT);
}
static void cut_lanes(const page_t *pg,const track_t *t)
{
    int32_t rh=graph_ht/8;uint32_t base=pg->id[0]==P_LN4?4:0;
    for(uint32_t k=0;k<8;k++){
        int32_t y=(int32_t)k*rh,v=t->p[P_LN0+k];
        int muted=t->p[P_MUTE] || ((drum_mute[trk_index(t)]>>k)&1u);
        int hot=ui.hot_t && k==base+ui.hot_col;
        uint16_t ink=muted||!v?T_DIM:hot?T_ACCENT:T_RHYTHM;
        cv_text_fit(12,y,&AF_C,drum_lane_name(t,k),k>=base&&k<base+4?T_TEXT:T_SUPPORT,T_BG,52);
        cv_rect(72,y+3,100,2,T_RAISE);if(v)cv_rect(72,y+2,v*100/127,4,ink);
        if(hot)cv_frame(70,y,104,8,T_ACCENT);
        if(muted)cv_text_r(228,y,&AF_C,"M",T_DIM,T_BG);
        else{char b[20];const char *unit;param_format(track_desc(t,P_LN0+k),v,b,&unit);str_cpy(b+str_len(b),unit,sizeof b-str_len(b));cv_text_r(228,y,&AF_C,b,v?T_TEXT:T_DIM,T_BG);}
    }
}
#if FELUCCA_SLICE
static void cut_slice_source(const track_t *t)
{
    uint32_t src,div;const slc_src_t *s=slice_src(&src,&div);uint32_t n=s?slice_count():0;
    if(!s || !n){cut_missing(src==SLC_SRC_VOCALS?"BANK MISSING":"SAMPLE NOT FOUND");return;}
    uint32_t sel=slc_note_slice(t->p,last_note,n);char b[20];
    cv_text(12,0,&AF_C,N_SLC_SRC[src],T_TEXT);
    fmt_int(b,n);str_cpy(b+str_len(b),n==1?" REGION":" REGIONS",sizeof b-str_len(b));cv_text_r(228,0,&AF_C,b,T_SUPPORT,T_BG);
    int32_t y=19,h=graph_ht-40;
    cv_rect(12,y+h/2,216,1,T_LINE);
    for(uint32_t i=0;i<n;i++){
        uint32_t a,e,st;slc_bounds(s,div,i,&a,&e,&st);
        int32_t x0=12+(int32_t)((uint64_t)a*215/s->len),x1=12+(int32_t)((uint64_t)e*215/s->len);
        if(x1<=x0)x1=x0+1;
        cv_rect(x0,y+3,x1-x0,h-6,T_RAISE);
        cv_rect(x0,y+3,1,h-6,T_RHYTHM);
        if(i==sel)cv_frame(x0,y,x1-x0,h,T_ACCENT);
    }
    cv_text(12,graph_ht-12,&AF_C,"KEY REGION",T_SUPPORT);
    fmt_int(b,(int32_t)sel+1);cv_text_r(228,graph_ht-12,&AF_C,b,T_ACCENT,T_BG);
}
static void cut_slices(void)
{
    uint32_t a,b,len,n=slice_count(),j=slice_sel(),src,div,ma,mb;
    if(!slice_src(&src,&div)||!n){cut_missing("SAMPLE NOT FOUND");return;}
    slice_view(&a,&b,&len);slice_env(a,b);ma=slice_mark(0);mb=slice_mark(n);
    int32_t top=1,bot=graph_ht-30,cy=(top+bot)/2,amp=(bot-top)/2-2;
#define CSX(p) (12+(int32_t)((uint64_t)((p)-a)*215/(b-a)))
    if(j<n){int32_t x0=CSX(slice_mark(j)),x1=CSX(slice_mark(j+1));cv_rect(x0,top,x1-x0,bot-top,T_RAISE);}
    for(uint32_t i=0;i<SP_COLS;i++){
        uint32_t p=a+i*(b-a)/SP_COLS;
        cv_line(12+(int32_t)i,cy-sp.hi[i]*amp/64,12+(int32_t)i,cy-sp.lo[i]*amp/64,p>=ma&&p<mb?T_THEME:T_DIM);
    }
    for(uint32_t i=0;i<=n;i++){uint32_t p=slice_mark(i);if(p>=a&&p<=b)cv_rect(CSX(p),top,1,bot-top,T_SUPPORT);}
    int32_t mark=CSX(slice_mark(j));if(mark>226)mark=226;
    cv_rect(mark,0,2,bot+1,T_ACCENT);cv_rect(mark-2,0,6,2,T_ACCENT);
#undef CSX
    int32_t oy=bot+7;cv_rect(12,oy,216,2,T_LINE);
    for(uint32_t i=0;i<=n;i++)cv_rect(12+(int32_t)((uint64_t)slice_mark(i)*215/len),oy-2,1,6,i==j?T_ACCENT:T_SUPPORT);
    int32_t x0=12+(int32_t)((uint64_t)a*215/len),x1=12+(int32_t)((uint64_t)b*215/len);
    cv_rect(x0,oy+5,x1>x0?x1-x0:1,2,T_RHYTHM);
    char t[24],u[16];str_cpy(t,"LEN ",sizeof t);slice_time(u,j<n?slice_mark(j+1)-slice_mark(j):len-mb);str_cpy(t+4,u,sizeof t-4);str_cpy(t+str_len(t)," S",sizeof t-str_len(t));
    cv_text(12,graph_ht-12,&AF_C,t,T_SUPPORT);cv_text_r(228,graph_ht-12,&AF_C,N_SLC_SRC[src],T_TEXT,T_BG);
}
#endif
static void graph_cut_samples(const page_t *pg,const track_t *t)
{
    cv_oy=26;graph_ht=(int32_t)graph_h()-30;
    if(pg->graph==GR_SLCR)cut_slicer(t);
    else if(pg->id[0]==P_LN0||pg->id[0]==P_LN4)cut_lanes(pg,t);
#if FELUCCA_SLICE
    else if(pg->graph==GR_SLICES)cut_slices();
    else if(pg->scope==SC_ENGINE && ENGINES[t->eng_req%NENGINES]==&ENG_SLICE)cut_slice_source(t);
#endif
    else cut_sample_wave();
    graph_ht=100;cv_oy=0;
    section_heading(ICON_BG_CUT,"CUT",T_RHYTHM);
    const char *title=pg->scope==SC_ENGINE?ENGINES[t->eng_req%NENGINES]->page_title[pg->id[0]!=P_E0]:pg->title;
    cv_text_r(228,5,&AF_C,title,T_SUPPORT,T_BG);
}

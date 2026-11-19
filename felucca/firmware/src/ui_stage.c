/* SPDX-License-Identifier: GPL-3.0-only
 * STAGE presentation of existing performance controls and published data.
 * No musical state ownership; MIXER retains the existing peak-meter consumer. */
static void stage_heading(const char *page)
{
    cv_oy=0;section_heading(ICON_BG_STAGE,"STAGE",T_STAGE);
    cv_text_r(228,5,&AF_C,page,T_SUPPORT,T_BG);
}
static void stage_tracks(void)
{
    if(ui.force){
        cv_begin(240,26,T_BG);cv_bg=T_BG;stage_heading(home_levels()?(mix_base()?"LEVELS 5-6":"LEVELS 1-4"):"TRACKS");cv_blit(0,graph_y());
        lcd_fill(0,graph_y()+26,240,2,T_BG);
        lcd_fill(0,graph_y()+28,8,graph_h()-28,T_BG);lcd_fill(232,graph_y()+28,8,graph_h()-28,T_BG);
        for(uint32_t k=0;k<NTRK;k++)lcd_fill(8,graph_y()+42+k*15,224,1,T_BG);
        lcd_fill(8,graph_y()+118,224,graph_h()-118,T_BG);
    }
    for(uint32_t k=0;k<NTRK;k++){
        const track_t *t=&trk[k];const step_t *st=seq_steps(t);
        uint32_t sel=k==song.sel,mute=t->p[P_MUTE]!=0,arm=(song.rec>>k)&1u,len=(uint32_t)clamp(t->p[P_SLEN],1,NSTEP);
        uint32_t bank=song.playing?t->seq_idx/16:sel?ui.cursor/16:0;if(bank*16>=len)bank=0;
        char name[16];trk_short_name(k,name);
        uint32_t sig=str_hash(steps_hash(t),name)^sel*7919u^mute*40503u^arm*104729u^bank*613u;
        sig^=(song.playing?(t->seq_idx+1u)*65537u:0u)+len*131u+(uint32_t)song.playing*15485863u+(uint32_t)midi_lit(k)*977u;
        if(!ui.force && sig==ts.col[k])continue;ts.col[k]=sig;
        cv_begin(224,14,T_BG);cv_bg=T_BG;cv_oy=0;
        if(sel)cv_frame(0,0,224,14,T_ACCENT);
        char n[2]={(char)('1'+k),0};cv_text(4,2,&AF_C,n,midi_lit(k)?T_STAGE:sel?T_ACCENT:T_SUPPORT);
        cv_free_text(19,2,&AF_C,name,mute?T_DIM:T_TEXT,T_BG,80);
        if(arm){if(song.playing)cv_rect(103,3,4,4,T_REC);else cv_frame(103,3,4,4,T_REC);}
        if(mute)cv_rect(103,10,4,1,T_DIM);
        for(uint32_t i=0;i<16 && bank*16+i<len;i++){
            const step_t *s=&st[bank*16+i];int on=s->n||s->hit;
            int32_t x=115+(int32_t)i*6;uint16_t ink=mute?T_DIM:T_RHYTHM;
            cv_rect(x,on?5:10,4,on?6:1,ink);
            if(song.playing&&t->seq_idx==bank*16+i){cv_rect(x+1,2,1,10,T_PLAY);cv_rect(x,1,4,1,T_PLAY);}
        }
        if(len>16)for(uint32_t i=0;i<(len+15)/16;i++)cv_rect(215+(int32_t)i*2,3,1,5,i==bank?T_SUPPORT:T_LINE);
        cv_blit(8,graph_y()+28+k*15);
    }
}
static void stage_mixer(void)
{
    if(ui.force){
        cv_begin(240,26,T_BG);cv_bg=T_BG;stage_heading(mix_base()?"MIXER 5-6":"MIXER 1-4");cv_blit(0,graph_y());
        lcd_fill(0,graph_y()+26,8,graph_h()-26,T_BG);
        for(uint32_t c=0;c<4;c++)lcd_fill(60+c*56,graph_y()+26,c==3?12:4,graph_h()-26,T_BG);
    }
    int32_t h=(int32_t)graph_h()-26;
    for(uint32_t c=0;c<4;c++){
        uint32_t k=mix_base()+c;
        if(k>=NTRK){if(ui.force){cv_begin(52,h,T_BG);cv_bg=T_BG;cv_text_in(0,2,52,&AF_C,"--",T_DIM,T_BG);cv_blit(8+56*c,graph_y()+26);}continue;}
        track_t *t=&trk[k];uint32_t sel=k==song.sel,mute=t->p[P_MUTE]!=0,arm=(song.rec>>k)&1u,hot=sel&&ui.hot_t?ui.hot_col+1u:0u;
        uint32_t level=trk_level(k);int32_t pan=clamp(t->p[P_PAN],-64,63),rev=clamp(t->p[P_REV],0,127);
        int32_t m=mute?0:meter_px(t->peak);t->peak=0; /* Existing MIXER telemetry consumption. */
        if(m<(int32_t)ts.meter[k]-1)m=(int32_t)ts.meter[k]-1;ts.meter[k]=(uint8_t)(m<0?0:m);
        char name[16];trk_short_name(k,name);
        uint32_t sig=str_hash(1+sel+mute*2+arm*4+hot*16,name)+level*7919u+(uint32_t)(pan+64)*40503u+(uint32_t)rev*104729u+ts.meter[k]*131u+(uint32_t)song.playing*977u+(uint32_t)midi_lit(k)*65537u;
        if(!ui.force && sig==ts.col[k])continue;ts.col[k]=sig;
        cv_begin(52,h,T_BG);cv_bg=T_BG;cv_oy=0;if(sel)cv_frame(0,0,52,h,T_ACCENT);
        char b[20]={'T',(char)('1'+k),0};cv_text(4,1,&AF_C,b,midi_lit(k)?T_STAGE:sel?T_ACCENT:T_SUPPORT);
        if(arm){if(song.playing)cv_rect(33,4,4,4,T_REC);else cv_frame(33,4,4,4,T_REC);}
        if(mute)cv_text_r(48,1,&AF_C,"M",hot==4?T_ACCENT:T_DIM,T_BG);
        cv_free_text(4,14,&AF_C,name,mute?T_DIM:T_TEXT,T_BG,44);
        const char *unit;param_format(&TP[P_LEVEL],level,b,&unit);str_cpy(b+str_len(b),unit,sizeof b-str_len(b));cv_text_in(2,27,48,&AF_C,b,mute?T_DIM:T_TEXT,T_BG);
        cv_rect(4,41,44,2,T_RAISE);int32_t lx=4+(int32_t)level*43/127;
        if(level)cv_rect(4,40,lx-3,4,mute?T_DIM:T_THEME);cv_frame(lx-1,38,3,8,hot==1?T_ACCENT:T_TEXT);
        cv_rect(4,49,44,2,T_RAISE);if(ts.meter[k])cv_rect(4,49,ts.meter[k]*44/(TS_MH-2),2,T_PLAY);
        int32_t py=h-24,ry=h-12;cv_text(4,py,&AF_C,"P",T_SUPPORT);cv_rect(17,py+4,30,1,T_LINE);cv_rect(32,py+1,1,7,T_SUPPORT);
        cv_frame(16+(pan+64)*30/127,py+1,3,7,hot==2?T_ACCENT:mute?T_DIM:T_RHYTHM);
        cv_text(4,ry,&AF_C,"R",T_SUPPORT);cv_rect(17,ry+3,30,2,T_RAISE);if(rev)cv_rect(17,ry+2,rev*30/127,4,hot==3?T_ACCENT:mute?T_DIM:T_RHYTHM);
        cv_blit(8+c*56,graph_y()+26);
    }
}
static void stage_params(const page_t *pg)
{
    int32_t rh=graph_ht/4;
    for(uint32_t c=0;c<4;c++){
        int16_t *v;const param_desc_t *d=page_desc(pg,c,&v);if(!d||d->label[0]=='-')continue;
        int32_t y=(int32_t)c*rh,range=d->max-d->min,w=range?clamp(*v-d->min,0,range)*132/range:0;
        cv_text_fit(12,y,&AF_C,d->label,T_TEXT,T_BG,62);cv_rect(84,y+5,140,2,T_RAISE);
        if(w)cv_rect(84,y+4,w,4,T_RHYTHM);cv_frame(82+w,y+2,5,8,ui.hot_t&&ui.hot_col==c?T_ACCENT:T_TEXT);
    }
}
static void stage_arp(const track_t *t)
{
    char b[20];const char *unit;
    param_format(&TP[P_AMODE],t->p[P_AMODE],b,&unit);cv_text(12,0,&AF_V,b,t->p[P_AMODE]?T_TEXT:T_DIM);
    fmt_int(b,t->nheld);str_cpy(b+str_len(b)," HELD",sizeof b-str_len(b));cv_text_r(228,2,&AF_C,b,T_SUPPORT,T_BG);
    int32_t n=clamp(t->p[P_AOCT],1,4),h=graph_ht-35;
    for(int32_t i=0;i<n;i++){
        int32_t x=16+i*52,y=22+(n-1-i)*(h-12)/(n>1?n-1:1);cv_frame(x,y,40,10,t->p[P_AMODE]?T_STAGE:T_DIM);
        char oct[4];fmt_int(oct,i+1);cv_text_in(x,y,40,&AF_C,oct,t->p[P_AMODE]?T_TEXT:T_DIM,T_BG);
        if(i)cv_line(x-12,y+5,x-1,y+5,T_SUPPORT);
    }
    cv_text(12,graph_ht-12,&AF_C,"OCTAVE SPAN",T_SUPPORT);param_format(&TP[P_AGATE],t->p[P_AGATE],b,&unit);str_cpy(b+str_len(b),unit,sizeof b-str_len(b));
    cv_text_r(228,graph_ht-12,&AF_C,b,T_SUPPORT,T_BG);
}
static void stage_scale(const track_t *t)
{
    static const uint8_t black[12]={0,1,0,1,0,0,1,0,1,0,1,0};uint32_t mask=scale_mask(t),root=(uint32_t)t->p[P_ROOT]%12;
    int32_t h=graph_ht-20;
    for(uint32_t i=0;i<12;i++){
        int32_t x=13+(int32_t)i*18,y=black[i]?h/3:0;uint32_t deg=(i+12-root)%12;int on=(mask>>deg)&1u;
        cv_rect(x,y,14,h-y,on?(i==root?T_STAGE:T_RHYTHM):T_RAISE);
        cv_text_in(x-2,h+5,18,&AF_C,N_NOTE[i],on?T_TEXT:T_DIM,T_BG);
    }
}
static void stage_chord(const track_t *t)
{
    if(!t->p[P_CHRD]){cv_text(12,graph_ht/2-10,&AF_C,"CHORD KEYS OFF",T_DIM);return;}
    if(chord_kit(t)){cv_text(12,graph_ht/2-10,&AF_C,"NO CHORDS ON KITS",T_DIM);return;}
    uint8_t notes[CHORD_MAX];int32_t root;uint16_t mask;uint32_t n,k=trk_index(t);const char *cap="ON ROOT";
    if(chord_last[k].n){n=chord_last[k].n;root=chord_last[k].root;mask=chord_last[k].mask;memcpy(notes,chord_last[k].note,n);cap="LAST";}
    else n=chord_make(t,60+t->p[P_ROOT],notes,&root,&mask);
    if(!n)return;if(trk_vmode(t)!=V_POLY)cap="ROOT ONLY";
    char b[12];chord_name(b,root,mask);cv_text(12,0,&AF_V,b,T_TEXT);cv_text_r(228,2,&AF_C,cap,T_SUPPORT,T_BG);
    uint32_t lo=notes[0]-notes[0]%12,cnt=notes[n-1]-lo<24?24:36,w=216/cnt;int32_t h=graph_ht-40;
    for(uint32_t i=0;i<cnt;i++){
        uint32_t note=lo+i,on=0;for(uint32_t j=0;j<n;j++)on|=notes[j]==note;
        int black=KEY_BLACK[note%12];int32_t y=black?25:32;
        cv_rect(13+i*w,y,w-2,h-(black?0:7),!on?T_RAISE:note%12==(uint32_t)root%12?T_STAGE:T_RHYTHM);
    }
    note_name(b,lo);cv_text(12,graph_ht-10,&AF_C,b,T_SUPPORT);note_name(b,lo+cnt-1);cv_text_r(228,graph_ht-10,&AF_C,b,T_SUPPORT,T_BG);
}
static void stage_clock(void)
{
    char b[20];const char *unit;param_format(&GP[G_CLOCK],song.g[G_CLOCK],b,&unit);cv_text(12,0,&AF_C,b,T_TEXT);
    cv_text_r(228,0,&AF_C,song.playing?"RUN":"STOP",song.playing?T_PLAY:T_SUPPORT,T_BG);
    int32_t h=graph_ht-40;for(uint32_t i=0;i<16;i++){
        int32_t x=14+i*13;cv_rect(x,23,10,h,i%4?T_RAISE:T_RHYTHM);
        if(song.playing&&(clk_step&15u)==i){cv_frame(x-1,20,12,h+6,T_PLAY);cv_rect(x,19,10,2,T_PLAY);}
    }
    cv_text(12,graph_ht-12,&AF_C,"4 QUARTERS / 16 STEPS",T_SUPPORT);
}
static void graph_stage(const page_t *pg,const track_t *t)
{
    if(pg->graph==GR_DANCE){graph_dance(t,ACC);return;}
    cv_oy=26;graph_ht=(int32_t)graph_h()-30;
    if(ui.home){cv_oy=0;graph_scope(T_THEME);cv_text(12,(int32_t)graph_h()-12,&AF_C,"OUTPUT / AUTO SCALE",T_SUPPORT);}
    else if(pg->graph==GR_SCALE)stage_scale(t);
    else if(pg->graph==GR_CHORD)stage_chord(t);
    else if(str_eq(pg->title,"CLOCK"))stage_clock();
    else if(str_eq(pg->title,"ARP"))stage_arp(t);
    else stage_params(pg);
    graph_ht=100;cv_oy=0;stage_heading(ui.home?"SCOPE":pg->title);
}

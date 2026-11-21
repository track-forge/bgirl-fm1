/* SPDX-License-Identifier: GPL-3.0-only
 * CUT sequencing presentation. Reuses existing rows, timing, layout/follow and
 * persistence summaries; no input/navigation, musical state or DSP ownership. */
static void cut_sequence_heading(const page_t *pg,const track_t *t)
{
    char b[32];cv_oy=0;
    section_heading(ICON_BG_CUT,"CUT",T_RHYTHM);
    if(pg->graph==GR_ROLL){
        str_cpy(b,detail_on()?"DETAIL ":"STEP ",sizeof b);
        fmt_int(b+str_len(b),(int32_t)ui.bank*16+1);str_cpy(b+str_len(b),"-",2);
        int32_t end=(int32_t)(ui.bank+1)*16;if(end>t->p[P_SLEN])end=t->p[P_SLEN];
        fmt_int(b+str_len(b),end);str_cpy(b+str_len(b),"/",2);fmt_int(b+str_len(b),t->p[P_SLEN]);
    }else if(pg->graph==GR_EVENTS){
        uint16_t rw[EV_ROWS];uint32_t n=ev_list(rw);
        str_cpy(b,"AUTO ",sizeof b);fmt_int(b+5,(int32_t)ev_row(n)+1);
        str_cpy(b+str_len(b),"/",2);fmt_int(b+str_len(b),(int32_t)n);
    }else{
        song_lay_t L;song_layout(&L);str_cpy(b,"SONG / ",sizeof b);
        fmt_int(b+7,(int32_t)((L.u0[L.n]+SG_BARU-1)/SG_BARU));str_cpy(b+str_len(b)," B",3);
    }
    cv_text_r(228,5,&AF_C,b,T_SUPPORT,T_BG);
}
static int32_t cut_event_value(int32_t y,const char *val,const char *unit,uint16_t col)
{
    int32_t x=226,py=y+CAP_IN(V,18);
    if(unit && unit[0])x=cv_text_r(x,py+AF_V.asc-AF_C.asc,&AF_C,unit,col,T_BG)-2;
    return cv_text_r(x,py,&AF_V,val,col,T_BG);
}
static void graph_cut_events(void)
{
    const track_t *t=TSEL;uint16_t rw[EV_ROWS];uint32_t n=ev_list(rw),cur=ev_row(n);
    int32_t first=clamp((int32_t)cur-2,0,n>5?(int32_t)n-5:0);
    for(uint32_t row=0;row<5 && (uint32_t)first+row<n;row++){
        uint32_t idx=(uint32_t)first+row,code=rw[idx],kind=code>>8;
        int sel=idx==cur;int32_t y=28+(int32_t)row*18;
        if(sel)cv_frame(8,y,224,18,T_ACCENT);
        uint16_t label=sel?T_ACCENT:T_SUPPORT;
        if(kind==EVK_ADD){
            ev_fix();const char *name=ui.ev_id==EV_CHANCE?"+ ADD CHANCE":ui.ev_id==EV_RATCH?"+ ADD RATCH":ui.ev_id==EV_NUDGE?"+ ADD NUDGE":ui.ev_id==EV_VEL?(ui.ev_vel?"+ HIDE VEL":"+ SHOW VEL"):"+ ADD LOCK";
            cv_text(14,y+CAP_IN(C,18),&AF_C,name,label);break;
        }
        if(kind==EVK_TOP || kind==EVK_QNT){
            char b[28];cv_text(14,y+CAP_IN(C,18),&AF_C,kind==EVK_TOP?"PLAY":"QNTZ",label);
            int on=kind==EVK_TOP?motion_enabled(t):t->p[P_SQNT];
            cv_text(56,y+CAP_IN(C,18),&AF_C,on?"ON":"OFF",on?T_THEME:T_DIM);
            if(kind==EVK_TOP){str_cpy(b,"EVENT ",sizeof b);fmt_int(b+6,(int32_t)(motion_count(t)-motion_lock_count(t)));str_cpy(b+str_len(b)," LOCK ",7);fmt_int(b+str_len(b),(int32_t)motion_lock_count(t));}
            else{str_cpy(b,"NUDGED ",sizeof b);fmt_int(b+7,(int32_t)ev_nudged(t));}
            cv_text_r(226,y+CAP_IN(C,18),&AF_C,b,T_TEXT,T_BG);continue;
        }
        char val[20],name[12],step[8];const char *unit="";uint16_t vc=T_TEXT;
        uint32_t si;
        if(kind==EVK_REC){
            const motion_event_t *e=&motion.event[code&255u];uint32_t id=MOTION_ID(e);
            si=e->place&63u;ev_name(t,id,name);param_format(motion_visual(id)?motion_desc(t,id):track_desc(t,id),e->value,val,&unit);
            cv_text(14,y+CAP_IN(C,18),&AF_C,e->param&MOTION_LOCK?"LOCK":"AUTO",e->param&MOTION_LOCK?T_ACCENT:T_RHYTHM);
        }else{
            si=code&255u;const step_t *s=&t->step[si];int32_t v=ev_sval(s,kind);
            str_cpy(name,kind==EVK_CHANCE?(v>100?"COND":"CHANCE"):kind==EVK_RATCH?"RATCH":kind==EVK_VEL?"VEL":"NUDGE",sizeof name);
            if(kind==EVK_NUDGE && !ev_nudge_plays(t,s))vc=T_DIM;
            if(kind==EVK_VEL && !ev_vel_plays(s))vc=T_DIM;
            if(kind==EVK_CHANCE)unit=ev_chance_fmt(val,v);
            else if(kind==EVK_NUDGE){ev_nudge_fmt(val,v);unit="/16";}
            else if(kind==EVK_RATCH){val[0]='x';val[1]=(char)('0'+v);val[2]=0;}
            else fmt_int(val,v);
            cv_text(14,y+CAP_IN(C,18),&AF_C,"STEP",label);
        }
        fmt_int(step,(int32_t)si+1);cv_text(56,y+CAP_IN(C,18),&AF_C,step,T_SUPPORT);
        int32_t vx=cut_event_value(y,val,unit,vc);
        cv_free_text(86,y+CAP_IN(C,18),&AF_C,name,vc==T_DIM?T_DIM:T_TEXT,T_BG,vx-92);
    }
}
#define CS_Y 28
#define CS_LH 11
#define CS_PITCH 13
static void graph_cut_song(void)
{
    song_lay_t L;song_layout(&L);int32_t off=song_follow(&L),px=song_playx(&L),sp=px>=0?SG_X0+px-off:-100;
    uint32_t cur=song_cur(),lane=ui.song_trk%NTRK;const int32_t hi=SG_X0+SG_W;
    for(uint32_t k=0;k<NTRK;k++){
        int32_t y=CS_Y+(int32_t)k*CS_PITCH;char tr[3]={'T',(char)('1'+k),0};
        cv_text_fit(4,y+1-AF_C_CAP_Y,&AF_C,tr,k==lane?T_ACCENT:T_SUPPORT,T_BG,19);
        for(uint32_t i=0,j;i<L.n;i=j){
            uint32_t s=chain_config.row[i].slot[k];for(j=i+1;j<L.n && chain_config.row[j].slot[k]==s;j++);
            int32_t bx=SG_X0+L.x[i]-off,ex=SG_X0+L.x[j]-off-SG_GAP,x0=bx>SG_X0?bx:SG_X0,x1=ex<hi?ex:hi;
            if(x1<=x0)continue;
            if(s>=4){cv_rect(x0,y+CS_LH/2,x1-x0,1,T_LINE);continue;}
            int saved=graph_song_used(s);
            if(saved)cv_rect(x0,y,x1-x0,CS_LH,T_RAISE);else sg_frame(bx,ex-bx,y,CS_LH,1,SG_X0,hi,T_DIM);
            if(saved){
                for(uint32_t q=i;q<j;q++){
                    int32_t sx=SG_X0+L.x[q]-off,wt=L.x[q+1]-L.x[q]-(q+1==j?SG_GAP:0);
                    uint32_t plen=graph_plen[s][k],nc=graph_pcell[s][k],den=graph_pden[s][k];
                    uint32_t reps=chain_config.row[q].repeat?chain_config.row[q].repeat:1,unit=song_unit(&chain_config.row[q]);
                    if(!plen || !nc || wt<=0)continue;
                    int32_t start=sx<SG_X0?SG_X0:sx;
                    uint32_t lastpass=start>sx?sg_muldiv((uint32_t)(start-sx-1),reps*unit,(uint32_t)wt)/unit:0;
                    for(int32_t x=start;x<sx+wt && x<hi;x++){
                        uint32_t u=sg_muldiv((uint32_t)(x-sx),reps*unit,(uint32_t)wt),cell=(u%plen)*nc/plen;
                        uint32_t pass=u/unit,h=(den>>(2u*cell))&3u;
                        if(pass!=lastpass && x>sx){lastpass=pass;continue;}
                        if(x<x0+9 || (k==lane && cur>=i && cur<j && x>=SG_X0+L.x[cur]-off && x<SG_X0+L.x[cur]-off+9))continue;
                        int32_t bh=(int32_t)h*7/3;
                        if(bh)cv_rect(x,y+CS_LH-2-bh,1,bh,T_RHYTHM);
                    }
                }
            }
            char a[2]={(char)('A'+s),0};if(x1-x0>=10)cv_text_on(x0+2,y+1-AF_C_CAP_Y,&AF_C,a,saved?T_TEXT:T_DIM,saved?T_RAISE:T_BG);
            if(k==lane && cur>=i && cur<j){
                int32_t cx=SG_X0+L.x[cur]-off;
                if(cx>x0+9 && cx+9<x1)cv_text_on(cx+2,y+1-AF_C_CAP_Y,&AF_C,a,saved?T_TEXT:T_DIM,saved?T_RAISE:T_BG);
            }
        }
        if(cur<L.n && k==lane)sg_frame(SG_X0+L.x[cur]-off,L.x[cur+1]-L.x[cur]-SG_GAP,y,CS_LH,1,SG_X0,hi,T_ACCENT);
    }
    if(L.add){
        int32_t bx=SG_X0+L.x[L.n]-off;
        for(int32_t y=CS_Y;y<CS_Y+NTRK*CS_PITCH-2;y+=3){
            if(bx>=SG_X0 && bx<hi)cv_rect(bx,y,1,2,cur==L.n?T_ACCENT:T_DIM);
            if(bx+SG_ADDW-1>=SG_X0 && bx+SG_ADDW-1<hi)cv_rect(bx+SG_ADDW-1,y,1,2,cur==L.n?T_ACCENT:T_DIM);
        }
        if(bx>=SG_X0 && bx+SG_ADDW<=hi)cv_text_in(bx,CS_Y+29,SG_ADDW,&AF_V,"+",cur==L.n?T_ACCENT:T_DIM,T_BG);
    }
    cv_rect(SG_X0,107,SG_W,1,T_LINE);
    /* Actual bar positions, using the same nonuniform section-time mapping. */
    for(uint32_t i=0;i<L.n;i++){
        int32_t sx=SG_X0+L.x[i]-off,w=L.x[i+1]-L.x[i];
        if(sx>=hi || sx+w<SG_X0)continue;
        uint32_t u=L.u0[i];
        if(sx<SG_X0)u+=sg_muldiv((uint32_t)(SG_X0-sx),L.u0[i+1]-u,(uint32_t)w);
        for(uint32_t bar=(u+SG_BARU-1)/SG_BARU;bar*SG_BARU<L.u0[i+1];bar++){
            int32_t x=SG_X0+song_ux(&L,bar*SG_BARU)-off;
            if(x>=hi)break;
            if(x>=SG_X0)cv_rect(x,108,1,bar%4?1:3,bar%4?T_DIM:T_SUPPORT);
        }
    }
    if(sp>=SG_X0 && sp<hi-1){cv_rect(sp,CS_Y,2,111-CS_Y,T_PLAY);cv_rect(sp-1,23,4,2,T_PLAY);}
    if(cur<L.n){
        uint32_t s=chain_config.row[cur].slot[lane];const char *name=s>=4?"SILENT":!graph_song_used(s)?"NOT SAVED":graph_project_name(s);
        cv_free_text(12,110,&AF_C,name,s>=4||!graph_song_used(s)?T_DIM:T_SUPPORT,T_BG,216);
    }else{
        uint32_t s;for(s=0;s<4 && !graph_song_used(s);s++);
        cv_free_hint(12,107,s<4?"[K3] ADD SECTION":"[SAVE] PROJECT A-D",T_SUPPORT,T_BG,216);
    }
}

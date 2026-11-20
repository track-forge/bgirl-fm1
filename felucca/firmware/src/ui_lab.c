/* SPDX-License-Identifier: GPL-3.0-only
 * LAB continuation: presentation of real parameters, not an audio analyzer.
 * Shares the approved graph/knob seam; no new UI or persistence state. */
static void lab_params(const page_t *pg)
{
    int32_t rh=graph_ht/4;
    for(uint32_t k=0;k<4;k++) {
        int16_t *v;const param_desc_t *d=page_desc(pg,k,&v);
        if(!d || !d->label || d->label[0]=='-') continue;
        int32_t y=(int32_t)k*rh,range=d->max-d->min;
        int32_t w=range?clamp(*v-d->min,0,range)*134/range:0;
        cv_text_fit(12,y,&AF_C,d->label,T_TEXT,T_BG,64);
        cv_rect(84,y+5,140,2,T_RAISE);
        if(w)cv_rect(84,y+4,w,4,T_THEME);
        cv_frame(82+w,y+2,5,8,T_TEXT);
    }
}
static void lab_mod(const track_t *t)
{
    int32_t rh=graph_ht/NMSLOT;
    for(uint32_t k=0;k<NMSLOT;k++) {
        const int16_t *p=&t->p[P_M1SRC+3u*k];int32_t y=(int32_t)k*rh;
        int on=p[0]&&p[1]&&p[2],sel=k==mod_ui_slot;
        uint16_t col=on?T_THEME:T_DIM;
        char b[12];const char *unit;
        if(sel)cv_frame(8,y,224,rh-1,T_ACCENT);
        b[0]=(char)('1'+k);b[1]=0;
        cv_text(12,y+2,&AF_C,b,sel?T_ACCENT:T_SUPPORT);
        cv_text_fit(28,y+2,&AF_C,N_MSRC[clamp(p[0],0,MS_N-1)],col,T_BG,52);
        cv_line(83,y+8,98,y+8,col);cv_line(94,y+5,98,y+8,col);cv_line(94,y+11,98,y+8,col);
        cv_text_fit(106,y+2,&AF_C,mod_dst_name(t,p[1]),col,T_BG,64);
        param_format(&TP[P_M1AMT],p[2],b,&unit);
        cv_text_r(224,y+2,&AF_C,b,on?T_TEXT:T_DIM,T_BG);
    }
}
/* Same ownership decision as audio; two lines also fit the LARGE strip. */
static void graph_voc_route(const track_t *t)
{
    int owner = voc_owner();
    char status[] = "VOCODER IN USE: T1";
    char guidance[] = "SELECT T1 TO EDIT";
    cv_oy = 0;
    if (owner < 0) {
        cv_text(12, 3, &AF_C, "VOCODER OFF", T_DIM);
        cv_text(12, 17, &AF_C, "SET CARR ON VOCODER PAGE", T_SUPPORT);
        return;
    }
    const track_t *mod = &trk[owner];
    int carrier = voc_carrier(mod, mod->p[P_VCARR]);
    if (t != mod) {
        status[sizeof status - 2] = (char)('1' + owner);
        guidance[8] = (char)('1' + owner);
        cv_text(12, 3, &AF_C, status, T_TEXT);
        cv_text(12, 17, &AF_C, guidance, T_SUPPORT);
        if (carrier >= 0 && t == &trk[carrier])
            cv_text_r(228, 17, &AF_C, "CARRIER", T_ACCENT, cv_bg);
        return;
    }
    char route[] = "T1 MODULATOR   T1 CARRIER";
    route[1] = (char)('1' + owner);
    route[16] = (char)('1' + carrier);
    if (carrier < 0 || voc_missing_modulator(mod))
        cv_text(12, 3, &AF_C, "VOCODER: NO SOURCE", T_ACCENT);
    else {
        cv_text(12, 3, &AF_C, t->p[P_VHOLD] ? "VOCODER: HOLD" : "VOCODER ACTIVE", T_TEXT);
        cv_text(12, 17, &AF_C, route, T_SUPPORT);
    }
}
/* WHEEL keeps all nine real drawbar levels, including percussion cancellation. */
static void lab_wheel(const track_t *t)
{
    static const char *const feet[9]={"16","5.3","8","4","2.7","2","1.6","1.3","1"};
    int32_t h=graph_ht-20;
    for(uint32_t k=0;k<9;k++){
        int32_t x=13+(int32_t)k*25,level=drw_level(t->p,k);
        if(t->p[P_E4]&&k==8)level=0;
        int32_t bh=level*h/8;
        cv_rect(x+5,0,2,h,T_RAISE);
        if(bh)cv_rect(x+1,h-bh,10,bh,T_THEME);
        cv_frame(x,h-bh,12,3,T_TEXT);
        cv_text_in(x-4,h+5,20,&AF_C,feet[k],T_SUPPORT,T_BG);
    }
}
static void graph_lab_more(const page_t *pg,const track_t *t)
{
    cv_oy=26;graph_ht=(int32_t)graph_h()-30;
    if(pg->graph==GR_LFO)graph_lfo(t,T_THEME);
    else if(pg->graph==GR_INS)graph_insert(t,T_THEME);
    else if(pg->graph==GR_MOD)lab_mod(t);
    else if(pg->graph==GR_FMENV && fop_ok())graph_fm6env(t,T_THEME);
    else if(pg->scope==SC_ENGINE && ENGINES[t->eng_req%NENGINES]==&ENG_WHEEL)lab_wheel(t);
    else lab_params(pg);
    graph_ht=100;cv_oy=0;
    section_heading(ICON_BG_LAB,"LAB",T_THEME);
    const char *title=pg->scope==SC_ENGINE?ENGINES[t->eng_req%NENGINES]->page_title[pg->id[0]!=P_E0]:pg->title;
    cv_text_r(228,5,&AF_C,title,T_SUPPORT,T_BG);
}

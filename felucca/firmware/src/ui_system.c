/* SPDX-License-Identifier: GPL-3.0-only
 * SYSTEM presentation. Uses the existing browser positions and storage queries;
 * never changes a selection, preset, project, pattern or confirmation action. */
static void system_heading(const char *label)
{
    cv_oy=0;
    section_heading(ICON_X_COG,"SYSTEM",T_SUPPORT);
    cv_text_r(228,5,&AF_C,label,T_SUPPORT,T_BG);
}
/* Full glyph ink plus two outline pixels: seven 14px rows are not safe.
 * Six 16px rows leave a 26px identity band within the existing 124px canvas. */
static void system_row(int32_t y,int selected,const char *tag,const char *name,
                       uint16_t color,int32_t right)
{
    if(selected)cv_frame(8,y,224,16,T_ACCENT);
    cv_text(13,y,&AF_C,tag,selected?T_ACCENT:T_SUPPORT);
    cv_free_text(61,y,&AF_C,name,color,T_BG,right-61);
}
static void system_note(const char *title,const char *detail)
{
    cv_text_in(8,44,224,&AF_V,title,T_TEXT,T_BG);
    cv_text_in(8,72,224,&AF_C,detail,T_SUPPORT,T_BG);
}
static void graph_system(const page_t *pg)
{
    system_heading(pg->title);
    if(pg->graph==GR_USER){
        int32_t first=clamp((int32_t)ui.uslot-2,0,UP_SLOTS-6);
        for(int32_t row=0;row<6;row++){
            uint32_t k=first+row;char tag[4],name[13];int used=up_used(k);
            up_slot_label(tag,k);if(used)up_name(k,name);else str_cpy(name,"EMPTY",sizeof name);
            system_row(26+16*row,k==ui.uslot,tag,name,used?T_TEXT:T_DIM,226);
        }
    }else if(pg->graph==GR_BROWSE){
        uint32_t total,cur=preset_pos(&total);
        if(!total){
            char title[24]="NO ";
            if(list_mode()==1u)str_cpy(title,"NO FAVORITES",sizeof title);
            else{str_cpy(title+3,list_name(list_mode()),sizeof title-3);str_cpy(title+str_len(title)," SOUNDS",sizeof title-str_len(title));}
            system_note(title,"K4 / ANOTHER LIST");return;
        }
        int32_t first=clamp((int32_t)cur-2,0,total>6?(int32_t)total-6:0);
        for(int32_t row=0;row<6&&(uint32_t)(first+row)<total;row++){
            uint32_t k,e=preset_at(first+row,&k);int sel=(uint32_t)(first+row)==cur;
            char tag[6],name[13],pt[4],pn[13];int32_t hint=sel?preset_pat_hint():-1,right=210;
            if(e==NENGINES){up_slot_label(tag,k);up_name(k,name);}
            else{str_cpy(tag,eng_abbr(ENGINES[e]->name),sizeof tag);str_cpy(name,ENGINES[e]->presets[k].name,sizeof name);}
            if(hint>=0){pat_label(hint,pt,pn);right=204-text_w(&AF_C,pt)-5;}
            int32_t y=26+16*row;system_row(y,sel,tag,name,T_TEXT,right);
            if(hint>=0)cv_text_r(204,y,&AF_C,pt,T_SUPPORT,T_BG);
            if(favorite_has(e,k))cv_icon_in(214,y,0,16,12,ICON_X_STAR,T_ACCENT,T_BG);
        }
    }else if(pg->graph==GR_PATS){
        uint32_t n=pat_count(),cur=pat_pick();int32_t first=clamp((int32_t)cur-2,0,n>6?(int32_t)n-6:0);
        for(int32_t row=0;row<6&&(uint32_t)(first+row)<n;row++){
            uint32_t k=first+row;char tag[4],name[13];pat_label(k,tag,name);
            system_row(26+16*row,k==cur,tag,name,T_TEXT,226);
        }
    }else if(pg->graph==GR_SLOTS){
        for(uint32_t k=0;k<4;k++){
            int32_t y=28+23*k;char tag[2]={(char)('A'+k),0};int used=graph_project_used(k);
            const char *name=graph_project_name(k);
            system_row(y,(int32_t)k+1==song.g[G_SLOT],tag,!used?"EMPTY":name[0]?name:"USED",used?T_TEXT:T_DIM,226);
        }
    }else{
        /* Same four actions as the knob register. No new confirmation paths. */
        static const char *const actions[]={"CLEAR PATTERN","INIT SOUND","DELETE SECTION","CLEAR SONG"};
        for(uint32_t k=0;k<4;k++){
            char tag[3]={'K',(char)('1'+k),0};
            int ready=!chain_busy() && (k==0 ? !seq_is_empty(TSEL)||motion_count(TSEL) : k==1 ? 1 : k==2 ? ui.song_row<chain_config.count : chain_config.count!=0);
            system_row(28+23*k,ui.act==k+1,tag,k==1&&ui.tl_clean?"CLEAN INIT":actions[k],ready?T_TEXT:T_DIM,226);
        }
    }
}

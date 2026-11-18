/* SPDX-License-Identifier: GPL-3.0-only
 * One selected dancer. No musical state mutations and no other-track evaluation. */
#include "../../../assets/crew/windmill/bgirl_windmill.h"
#include "../../../assets/crew/reference-02/bgirl_reference_02.h"
#include "../../../assets/crew/wave-ish/bgirl_wave_ish.h"
#include "../../../assets/crew/worm/bgirl_worm.h"
static const uint8_t (*const crew_masks[])[1152] = {
    bg_windmill_masks,bg_reference_02_masks,bg_wave_ish_masks,bg_worm_masks,bg_dance_masks
};
static const char *const crew_names[]={"WINDMILL","STEP / TURN","WAVE-ISH","WORM","FLOORWORK"};
static const uint8_t crew_moves[NTRK]={0,1,2,3,4,0};
static uint32_t crew_graph_sig, crew_graph_ms, crew_graph_track=~0u, crew_graph_word;
static uint32_t crew_graph_paints;
/* One four-beat cycle: 0..11,10..1, then 0. No duplicated endpoint hold.
 * Quantize only in the visible selected-track UI; audio publication is unchanged. */
static uint32_t crew_pose(uint32_t phase)
{
    uint32_t step=(phase&255u)*22u/256u;
    return step<12u ? step : 22u-step;
}
static void draw_crew(void)
{
    uint32_t word=crew_read(song.sel), move=crew_moves[song.sel];
    uint32_t pose=crew_pose(word);
    uint32_t sig=pose | (word&~255u) | song.sel<<16 | palette_cur()<<20 | (ui_style&1u)<<28;
    uint32_t urgent=ui.force || crew_graph_track!=song.sel || ((crew_graph_word^word)&(CREW_MUTED|CREW_SEEN));
    if(!urgent && ((uint32_t)(fm1_ms-crew_graph_ms)<67u || sig==crew_graph_sig)) return;
    crew_graph_ms=fm1_ms;crew_graph_sig=sig;crew_graph_track=song.sel;crew_graph_word=word;
    cv_begin(240,H_GRAPH,T_BG);
    cv_rect(3,0,234,H_GRAPH,T_BG);cv_bg=T_BG;cv_oy=0;
    cv_text_on(10,0,&AF_H,"STAGE",T_TEXT,T_BG);
    char label[9]={'T','R','A','C','K',' ',(char)('1'+song.sel),0};
    cv_text_on(10,30,&AF_V,label,T_THEME,T_BG);
    cv_text_fit(10,54,&AF_C,crew_names[move],T_MID,T_BG,116);
    const char *state=word&CREW_MUTED ? "MUTE" : word&CREW_ACTIVE ? "MOVE" : word&CREW_SETTLE ? "SETTLE" : "IDLE";
    cv_text_on(10,79,&AF_C,state,T_TEXT,T_BG);
    /* Accepted-note activity, not a meter. The square changes shape when active. */
    if(word&(CREW_ACTIVE|CREW_SETTLE))cv_rect(10,103,7,7,T_THEME);
    else cv_frame(10,103,7,7,T_MID);
    for(uint32_t i=0;i<3;i++)cv_line_t(26+(int32_t)i*9,110,32+(int32_t)i*9,103,T_MID,2);
    uint16_t color=word&CREW_MUTED ? T_DIM : (word>>11&3u) ? T_ACCENT : (word&(CREW_ACTIVE|CREW_SETTLE)) ? T_THEME : T_MID;
    for(uint32_t y=0;y<96;y++) for(uint32_t x=0;x<96;x++)
        if(crew_masks[move][pose][y*12u+x/8u] & (128u>>(x&7u))) cv_pset(132+(int32_t)x,22+(int32_t)y,color);
    cv_blit(0,graph_y());crew_graph_paints++;
}

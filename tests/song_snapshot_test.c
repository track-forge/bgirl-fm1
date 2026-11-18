/* SPDX-License-Identifier: GPL-3.0-only */
#include "../felucca/web/emu/felucca_web.c"
#include <assert.h>
#include <stdio.h>
static project_t before,after,check;
static project_store_t saved[4];
static int32_t wire_copy[SNAP_WORDS];
static uint32_t capture(void) { uint32_t n=web_snapshot_capture();assert(n>0);memcpy(wire_copy,snap_wire,n*4);return n; }
static void unchanged(void) { project_capture(&after);assert(!memcmp(&before,&after,sizeof before));assert(!memcmp(saved,proj_slot,sizeof saved)); }
int main(void) {
 web_nor_erase();web_boot();for(unsigned i=0;i<600;i++)web_render(128);
 /* Two genuinely different slot sources; multi-lane SONG. */
 trk[0].step[63].n=1;trk[0].step[63].time=ST_NOTE;trk[0].step[63].note[0]=44;
 assert(!project_save_as(0,"SOURCE A"));
 trk[0].step[63].note[0]=67;assert(!project_save_as(1,"SOURCE B"));
 chain_config.count=2;chain_config.row[0]=chain_row_of(0,2);chain_config.row[0].slot[5]=1;
 chain_config.row[1]=chain_row_of(1,1);chain_config.row[1].slot[4]=0;
 motion.on=63;assert(!motion_put(&trk[5],63,P_PAN,12,0));
 trk[5].p[P_PAN]=17;trk[4].step[63].n=1;trk[4].step[63].note[0]=55;
 fm6_patch[4][FP_R1]=71; fm6_adopt(4);
 uint32_t n=capture();assert(snap_wire[0]==3);assert(!web_snapshot_validate(n));
 assert(snap_stage[0].t[5].p[P_PAN]==17);
 assert(snap_stage[0].t[4].step[63].note[0]==55);assert(snap_stage[0].motion.count==1);
 /* Change destination slots before import: sources must remain portable. */
 trk[0].step[63].note[0]=91;trk[0].p[P_SLEN]=8;assert(!project_save_as(0,"LOCAL A"));
 trk[0].step[63].note[0]=94;trk[0].p[P_SLEN]=16;assert(!project_save_as(1,"LOCAL B"));
 memcpy(saved,proj_slot,sizeof saved);project_capture(&before);
 int32_t keep=snap_wire[1];snap_wire[1]=999;
 assert(web_snapshot_apply(n)==2);unchanged();snap_wire[1]=keep;
 snap_wire[0]=0;assert(web_snapshot_apply(n)==2);unchanged();memcpy(snap_wire,wire_copy,n*4);
 assert(web_snapshot_apply(n-1)==2);unchanged();memcpy(snap_wire,wire_copy,n*4);
 assert(!web_snapshot_apply(n));assert(portable_mask==3&&portable_session);assert(!song.playing);
 assert(!memcmp(saved,proj_slot,sizeof saved));
 char name[13];uint16_t lengths[NTRK];
 assert(song_source_info(0,name,lengths,0,0));assert(!strcmp(name,"SOURCE A"));assert(lengths[0]==len48_of(64,2));
 assert(project_info(0,name,lengths,0,0));assert(!strcmp(name,"LOCAL A"));assert(lengths[0]==len48_of(8,2));
 ui.frame++;ui.force=1;assert(graph_song_used(0));assert(!strcmp(graph_project_name(0),"SOURCE A"));assert(graph_plen[0][0]==len48_of(64,2));
 assert(graph_project_used(0));assert(!strcmp(graph_project_name(0),"LOCAL A"));assert(graph_plen[0][0]==len48_of(8,2));
 assert(!chain_prepare());
 assert(chain.source[0].step[0][63].note[0]==44);assert(chain.source[1].step[0][63].note[0]==67);
 transport_req=0;seq_stop();web_render(128);
 assert(project_save(2)!=0);assert(!memcmp(saved,proj_slot,sizeof saved));
 assert(web_snapshot_capture()==n);assert(!memcmp(wire_copy,snap_wire,n*4));
 /* Manual/new/load ownership lifecycle cannot reuse old dependencies. */
 assert(!project_new());assert(!portable_mask&&!portable_session);
 for(unsigned i=0;i<30;i++)web_render(128);
 /* Dense live state exceeds B6S1 but remains complete in logical interchange. */
 assert(!web_bgirl_six_project(1));for(unsigned i=0;i<30;i++)web_render(128);
 n=capture();assert(web_snapshot_bytes()>3840);assert(!web_snapshot_validate(n));assert(!web_snapshot_apply(n));
 assert(!memcmp(saved,proj_slot,sizeof saved));
 /* Queued note-ons / encoder events cleared only after complete validation. */
 web_midi(0x90,60,100);web_enc(0,1);web_keys(1);
 assert(!web_snapshot_apply(n));assert(mi_r==mi_w&&!web_notes_pressed);
 for(unsigned i=0;i<7;i++)assert(!web_enc_steps[i]);
 puts("song snapshot: unsaved six-track/motion, two-slot SONG portability, atomic reject, saved-slot preservation, dense FULL, lifecycle, queued-input release PASS");
 printf("browser snapshot static memory: %u bytes\n",web_snapshot_memory());
}

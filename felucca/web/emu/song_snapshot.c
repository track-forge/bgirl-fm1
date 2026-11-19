/* SPDX-License-Identifier: GPL-3.0-only */
/* Browser-only logical interchange. All calls are message/main-loop owned,
 * outside web_render. Wire is int32 LE values, not C ABI/padding or flash bytes.
 * JSON labels/IDs live in snapshot-layout.mjs, checked against core.h. */
#define SNAP_WORDS 65536u
static int32_t snap_wire[SNAP_WORDS];
static project_t snap_stage[5];
static uint32_t snap_n, snap_pos, snap_read, snap_bad, snap_mask;
static int32_t snap_value(int32_t v, int32_t lo, int32_t hi) {
    if (snap_pos >= SNAP_WORDS || (snap_read && snap_pos >= snap_n)) { snap_bad=1; return 0; }
    if (snap_read) v=snap_wire[snap_pos]; else snap_wire[snap_pos]=v;
    snap_pos++;
    if (v<lo || v>hi) { snap_bad=1; return lo; }
    return v;
}
#define SV(v,lo,hi) do { (v)=snap_value((v),(lo),(hi)); } while(0)
static void snap_project(project_t *p) {
    p->magic=PROJ_MAGIC; p->size=sizeof *p; p->parts=NPART; p->phys=PROJ_PHYS;
    SV(p->sel,0,NTRK-1);
    uint32_t ended=0;
    for (uint32_t i=0;i<PROJ_NAME_LEN;i++) {
        SV(p->name[i],0,126);
        if(!p->name[i])ended=1;
        else if(ended || p->name[i]<32 || (p->name[i]>='a' && p->name[i]<='z'))snap_bad=1;
    }
    for (uint32_t i=0;i<G_COUNT;i++) {
        if(i==G_SLOT || i==G_LOAD || i==G_SAVE) {
            if(!snap_read)p->g[i]=GP[i].def; /* UI actions are not musical state */
            SV(p->g[i],GP[i].def,GP[i].def);
        } else SV(p->g[i],GP[i].min,GP[i].max);
    }
    for (uint32_t t=0;t<NTRK;t++) {
        proj_trk_t *q=&p->t[t];
        SV(q->engine,0,NENGINES-1); SV(q->preset,0,255); SV(p->mch[t],0,MCH_N-1);
        for (uint32_t i=0;i<P_COUNT;i++) {
            SV(q->p[i],-64,127);
            if(snap_read && param_fit(param_desc_of(q->engine,i),q->p[i])!=q->p[i]) snap_bad=1;
        }
        for (uint32_t i=0;i<FM6_PACKED;i++) SV(p->fm6[t][i],0,127);
        for (uint32_t i=0;i<8;i++) SV(p->vocal_id[t][i],0,255);
        for (uint32_t i=0;i<NSTEP;i++) {
            step_t *s=&q->step[i];
            for (uint32_t j=0;j<4;j++) SV(s->note[j],0,127);
            SV(s->n,0,4); SV(s->time,0,ST_REST); SV(s->flags,0,255); SV(s->vel,0,127);
            SV(s->hit,0,255); SV(s->acc,0,255); SV(s->probability,0,COND_FIRST+NCOND-1);
        }
    }
    SV(p->chain.count,0,CHAIN_ROWS);
    for (uint32_t i=0;i<CHAIN_ROWS;i++) {
        for (uint32_t t=0;t<NTRK;t++) SV(p->chain.row[i].slot[t],0,CHAIN_SILENT);
        SV(p->chain.row[i].repeat,0,16);
    }
    SV(p->motion.on,0,(1u<<NTRK)-1); SV(p->motion.count,0,MOTION_MAX);
    for (uint32_t i=0;i<p->motion.count;i++) {
        SV(p->motion.event[i].place,0,NTRK*NSTEP-1);
        SV(p->motion.event[i].param,0,255); SV(p->motion.event[i].value,-64,127);
    }
    for(uint32_t t=0;t<NTRK;t++) if(p->t[t].p[P_VHOLD])snap_bad=1;
    p->sum=proj_sum(p);
    if (!proj_values_ok(p)) snap_bad=1;
}
#undef SV
EXPORT int32_t *web_snapshot_buffer(void) { return snap_wire; }
EXPORT uint32_t web_snapshot_capacity(void) { return SNAP_WORDS; }
EXPORT uint32_t web_snapshot_restore_allowed(void) { return !(ui_prefs & PREF_RESTORE_OFF); }
EXPORT uint32_t web_snapshot_active(void) { return portable_session; }
EXPORT uint32_t web_snapshot_memory(void) { return sizeof snap_wire+sizeof snap_stage+sizeof portable_source; }
EXPORT uint32_t web_snapshot_capture(void) {
    if (transport_busy() || fm1_in.notes || fm1_in.buttons) return 0;
    if(web_pressed || web_released || web_notes_pressed || mi_w!=mi_r) return 0;
    for(uint32_t i=0;i<7;i++) if(web_enc_steps[i]) return 0;
    project_capture(&snap_stage[0]);
    snap_mask=0;
    for (uint32_t r=0;r<chain_config.count;r++) for (uint32_t t=0;t<NTRK;t++) {
        uint32_t s=chain_config.row[r].slot[t]; if (s<4) snap_mask|=1u<<s;
    }
    for (uint32_t s=0;s<4;s++) if ((snap_mask>>s)&1u) {
        if ((portable_mask>>s)&1u) snap_stage[s+1]=portable_source[s];
        else if (!proj_import(&snap_stage[s+1],&proj_slot[s],sizeof(project_store_t))) return 0;
    }
    snap_pos=snap_bad=snap_read=0;
    snap_value(snap_mask,0,15);
    snap_project(&snap_stage[0]);
    for (uint32_t s=0;s<4;s++) if ((snap_mask>>s)&1u) snap_project(&snap_stage[s+1]);
    return snap_bad ? 0 : snap_pos;
}
static uint32_t snap_resource_ok(const project_t *p) {
    for (uint32_t t=0;t<NTRK;t++) {
        if(p->t[t].engine==4 || p->t[t].engine==8) {
            if(p->t[t].p[P_E0]>=SMP_NSETS) return 0;
            for(uint32_t i=0;i<p->motion.count;i++) {
                const motion_event_t *e=&p->motion.event[i];
                if((e->place>>6)==t && MOTION_ID(e)==P_E0 && e->value>=SMP_NSETS) return 0;
            }
        }
        if (p->t[t].engine==13) {
            int src=p->t[t].p[P_E0];
            uint32_t uses_vocals=src==SLC_SRC_VOCALS;
            for(uint32_t i=0;i<p->motion.count;i++) {
                const motion_event_t *e=&p->motion.event[i];
                if((e->place>>6)==t && MOTION_ID(e)==P_E0 && e->value==SLC_SRC_VOCALS)uses_vocals=1;
            }
            if(uses_vocals) {
                if(!slc_count(&SLC_VOCALS,(uint32_t)p->t[t].p[P_E1])) return 0;
                for(uint32_t i=0;i<p->motion.count;i++) {
                    const motion_event_t *e=&p->motion.event[i];
                    if((e->place>>6)==t && MOTION_ID(e)==P_E1 && !slc_count(&SLC_VOCALS,(uint32_t)e->value))return 0;
                }
            }
            if (src>=1 && src<=3) return 0; /* no portable identity for private user samples */
            if (src==SLC_SRC_VOCALS && memcmp(p->vocal_id[t],BG_VOCAL_ID,8)) return 0;
            for(uint32_t i=0;i<p->motion.count;i++) {
                const motion_event_t *e=&p->motion.event[i];
                if ((e->place>>6)==t && MOTION_ID(e)==P_E0) {
                    if(e->value>=1 && e->value<=3) return 0;
                    if(e->value==SLC_SRC_VOCALS && memcmp(p->vocal_id[t],BG_VOCAL_ID,8)) return 0;
                }
            }
        }
    }
    return 1;
}
/* Stage/validate without touching music, control latches, slots or save-fit flags.
 * Status: 1 busy,2 malformed,3 unresolved resources,4 missing source. */
EXPORT uint32_t web_snapshot_validate(uint32_t n) {
    if (!n || n>SNAP_WORDS) return 2;
    snap_n=n; snap_pos=snap_bad=0; snap_read=1;
    memset(snap_stage,0,sizeof snap_stage);
    snap_mask=snap_value(0,0,15);
    snap_project(&snap_stage[0]);
    for(uint32_t s=0;s<4;s++) if((snap_mask>>s)&1u) snap_project(&snap_stage[s+1]);
    if(snap_bad || snap_pos!=n) return 2;
    for(uint32_t r=0;r<snap_stage[0].chain.count;r++) for(uint32_t t=0;t<NTRK;t++) {
        uint32_t s=snap_stage[0].chain.row[r].slot[t];
        if(s<4 && !((snap_mask>>s)&1u)) return 4;
    }
    if(!snap_resource_ok(&snap_stage[0])) return 3;
    for(uint32_t s=0;s<4;s++) if((snap_mask>>s)&1u && !snap_resource_ok(&snap_stage[s+1])) return 3;
    return 0;
}
EXPORT uint32_t web_snapshot_bytes(void) { return proj_packed_size(&snap_stage[0]); }
EXPORT uint32_t web_snapshot_apply(uint32_t n) {
    uint32_t err=web_snapshot_validate(n); if(err) return err;
    if(transport_busy()) return 1;
    /* validation is complete before input release or any mutation */
    web_keys(0); web_buttons(0);
    web_pressed=web_released=web_notes_pressed=0;
    memset(web_enc_steps,0,sizeof web_enc_steps);
    mi_r=mi_w; midi_in_overflow=0; /* queued external note-ons belong to old music */
    if(project_restore_runtime(&snap_stage[0])) return 2;
    for(uint32_t s=0;s<4;s++) if((snap_mask>>s)&1u) portable_source[s]=snap_stage[s+1];
    portable_mask=snap_mask; portable_session=1;
    name_close(); ui.confirm=0; ui.menu=0; ui.lock=ui.ly=ui.layer=0; go_home();
    ui.home=1; ui.force=1;
    return 0;
}

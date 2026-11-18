/* SPDX-License-Identifier: GPL-3.0-only
 * Called only by core0 AFTER join: workers have stopped touching voices. */
static void crew_publish(uint32_t n)
{
#if BGIRL_CREW_TELEMETRY
    uint32_t quarter=beat_samples();
    for(uint32_t i=0;i<NTRK;i++) {
        track_t *t=&trk[i];
        uint32_t old=crew_read(i), word=old & (255u|CREW_SEEN), live=0, admitted=0;
        uint32_t oneshot=ENGINES[t->engine]->oneshot;
#if FELUCCA_SLICE
        if(t->engine==13 && (slc_vocal_source(slc_src_of(t->p)) || t->p[P_E4]==SLC_ONE)) oneshot=1;
#endif
        uint32_t muted=t->p[P_MUTE] || (pf.mute>>i&1u);
        for(uint32_t v=0;v<NVOICE;v++) {
            voice_t *p=&t->v[v];
            if(p->active && p->stage && p->stage!=4) {
                admitted=1;
                if(p->gate && !oneshot) live=1;
            }
        }
        if(muted) {
            crew_rt[i].pulse=crew_rt[i].settle=crew_rt[i].accent=0;
            crew_rt[i].blocked=1;
            word=CREW_MUTED; /* parked and no inherited accent */
        } else {
            if(crew_rt[i].pending && admitted) {
                crew_rt[i].blocked=0;
                crew_rt[i].pulse=oneshot ? FS/4u : 0u;
                crew_rt[i].accent=FS*90u/1000u;
                crew_rt[i].strength=1u+crew_rt[i].pending/64u;
                word|=CREW_SEEN;
            }
            if(crew_rt[i].blocked) live=0;
            if(!admitted) crew_rt[i].pulse=0;
            if(live || crew_rt[i].pulse) {
                crew_rt[i].settle=quarter;
                word=(word&CREW_SEEN) | (crew_phase>>24) | CREW_ACTIVE;
            } else if(crew_rt[i].settle) {
                word=(word&CREW_SEEN) | (crew_phase>>24) | CREW_SETTLE;
            }
            if(crew_rt[i].accent && (word&(CREW_ACTIVE|CREW_SETTLE)))
                word|=(uint32_t)crew_rt[i].strength<<11;
            crew_rt[i].pulse=crew_sub(crew_rt[i].pulse,n);
            crew_rt[i].settle=crew_sub(crew_rt[i].settle,n);
            crew_rt[i].accent=crew_sub(crew_rt[i].accent,n);
        }
        crew_rt[i].pending=0;
        __atomic_store_n(&crew_state[i],word,__ATOMIC_RELAXED);
    }
#else
    (void)n;
#endif
}

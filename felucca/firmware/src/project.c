/* SPDX-License-Identifier: GPL-3.0-only
 * Copyright (C) 2026 Leo Kuroshita (@kurogedelic), Hügelton Instruments
 * Portions Copyright (C) 2026 Juan Ignacio Antonucci (Juan Jiant)
 * BG6 compact codec: sparse map / conditional patch ideas adapted from
 * juanjiant-bit/JIANT da8baa1eed7c4e75f1ed9bcf433797b3aeace107 project.c.
 * BGIRL's distinct format, frozen defaults, exact dormant fields and LE wire
 * records intentionally do not share JIANT's FUNC format. See docs/UPSTREAM.md. */
#define PROJ_MAGIC 0x31533642u /* bytes B6S1; no legacy readers */
#define PROJ_NAME_LEN 12u
#define PROJ_STORE_SIZE 3840u
#define PROJ_PHYS 2u
#define PROJ_DEF_SOUND 0xFFu
#define PROJ_DEF_KEEP 0xFEu
#define PROJ_MAP ((P_COUNT + 7u) / 8u)
#define PROJ_END (PROJ_STORE_SIZE - 4u)
typedef struct {
    int16_t p[P_COUNT];
    uint8_t engine, preset;
    step_t step[NSTEP];
} proj_trk_t;
typedef struct {
    uint32_t magic, size;
    int16_t g[G_COUNT];
    uint8_t sel, parts, phys, rsv;
    proj_trk_t t[NTRK];
    chain_config_t chain;
    motion_store_t motion;
    uint8_t fm6[NTRK][FM6_PACKED], vocal_id[NTRK][8], mch[NTRK];
    char name[PROJ_NAME_LEN];
    uint32_t sum;
} project_t;
typedef union {
    uint32_t align;
    uint8_t raw[PROJ_STORE_SIZE];
} project_store_t;
static uint8_t proj_old_slots; /* recognized incompatible NOR slots; never erase on read */
project_store_t proj_slot[4] __attribute__((section(".noinit")));
#include "project_defaults.h"
_Static_assert(P_COUNT == 127 && G_COUNT == 27 && NSTEP == 64, "B6S1 clarity count adapter must be reviewed on change");
static uint8_t proj_full;
static uint32_t proj_hash(const void *p, uint32_t n) {
    const uint8_t *b = p;
    uint32_t h = 2166136261u;
    while (n--)
        h = (h ^ *b++) * 16777619u;
    return h;
}
static uint32_t proj_sum(const project_t *p) { return proj_hash(p, sizeof *p - 4); }
static int proj_engines_ok(const project_t *q) {
    for (uint32_t t = 0; t < NTRK; t++)
        if (q->t[t].engine >= NENGINES || q->t[t].engine == ENGI_DIGITAL)
            return 0;
    return 1;
}
static void proj_fm6_init(project_t *q) {
    for (uint32_t t = 0; t < NTRK; t++)
        memcpy(q->fm6[t], FM6_INIT, FM6_PACKED);
    q->sum = proj_sum(q);
}
static int proj_ok(const project_t *q) {
    return q->magic == PROJ_MAGIC && q->size == sizeof *q && q->sum == proj_sum(q) && q->sel < NTRK &&
           q->parts == NPART && chain_valid(&q->chain) && motion_valid(&q->motion);
}
static uint32_t proj_name_get(char *d, const uint8_t *s) {
    uint32_t n = 0;
    memset(d, 0, PROJ_NAME_LEN + 1);
    while (n < PROJ_NAME_LEN && s[n]) {
        if (s[n] < 32 || s[n] > 126) {
            memset(d, 0, PROJ_NAME_LEN + 1);
            return 0;
        }
        d[n] = (char)(s[n] >= 'a' && s[n] <= 'z' ? s[n] - 32 : s[n]);
        n++;
    }
    return n;
}
static uint32_t proj_rd(const uint8_t *b, uint32_t n) {
    uint32_t v = 0;
    for (uint32_t i = 0; i < n; i++)
        v |= (uint32_t)b[i] << (8 * i);
    return v;
}
static void proj_wr(uint8_t *b, uint32_t v, uint32_t n) {
    for (uint32_t i = 0; i < n; i++)
        b[i] = (uint8_t)(v >> (8 * i));
}
static int proj_step_empty(const step_t *s) {
    return !s->n && s->time == ST_REST && !s->flags && !s->vel && !s->hit && !s->acc && !s->probability &&
           !(s->note[0] | s->note[1] | s->note[2] | s->note[3]);
}
static uint32_t proj_step_x(const step_t *s, uint32_t *nn) {
    uint32_t dormant = 0;
    for (uint32_t i = s->n; i < 4; i++)
        dormant |= s->note[i];
    *nn = dormant ? 4 : s->n;
    return ((s->hit || s->acc) ? 1 : 0) | (s->probability ? 2 : 0) | (dormant ? 4 : 0) | ((s->flags & ~3u) ? 8 : 0);
}
static uint32_t proj_step_size(const step_t *s) {
    uint32_t nn, x = proj_step_x(s, &nn);
    return 2 + nn + (x ? 1 : 0) + (x & 1 ? 2 : 0) + (x & 2 ? 1 : 0) + (x & 8 ? 1 : 0);
}
/* Frozen v1 defaults: only the additive range discriminator is new. */
static int16_t proj_default(uint32_t e, uint32_t id) {
    return id == P_VRANGE ? 0 : BG6_DEFAULTS[e][id > P_VRANGE ? id-1 : id];
}
static uint32_t proj_clarity_id(uint32_t id, uint32_t count) {
    return count == 126 && id >= P_VRANGE ? id+1 : id;
}
static int proj_fm_kept(const project_t *q, uint32_t t) { return memcmp(q->fm6[t], BG6_FM6_INIT, FM6_PACKED) != 0; }
static uint32_t proj_packed_size(const project_t *q) {
    uint32_t n = 20 + G_COUNT * 2 + PROJ_NAME_LEN + 4 + CHAIN_ROWS * (NTRK + 1) + 4 + q->motion.count * 4 + 4;
    for (uint32_t t = 0; t < NTRK; t++) {
        n += 3 + PROJ_MAP + 8 + 8 + 1 + (proj_fm_kept(q, t) ? FM6_PACKED : 0);
        uint32_t e = q->t[t].engine;
        if (e >= NENGINES)
            return PROJ_STORE_SIZE + 1;
        for (uint32_t i = 0; i < P_COUNT; i++)
            n += q->t[t].p[i] != proj_default(e,i);
        for (uint32_t i = 0; i < NSTEP; i++)
            if (!proj_step_empty(&q->t[t].step[i]))
                n += proj_step_size(&q->t[t].step[i]);
    }
    return n;
}
static int proj_values_ok(const project_t *q) {
    if (q->sel >= NTRK || q->parts != NPART || q->phys != PROJ_PHYS || !proj_engines_ok(q) || !chain_valid(&q->chain) ||
        !motion_valid(&q->motion))
        return 0;
    for (uint32_t t = 0; t < NTRK; t++) {
        if (q->mch[t] >= MCH_N)
            return 0;
        for (uint32_t i = 0; i < P_COUNT; i++)
            if (q->t[t].p[i] < -64 || q->t[t].p[i] > 127)
                return 0;
        for (uint32_t i = 0; i < FM6_PACKED; i++)
            if (q->fm6[t][i] > 127)
                return 0;
        for (uint32_t i = 0; i < NSTEP; i++) {
            const step_t *s = &q->t[t].step[i];
            if (s->n > 4 || s->time > ST_REST || s->vel > 127 || s->probability >= COND_FIRST + NCOND ||
                (s->acc & ~s->hit))
                return 0;
            for (uint32_t j = 0; j < 4; j++)
                if (s->note[j] > 127)
                    return 0;
        }
    }
    return 1;
}
static int proj_pack(project_store_t *out, const project_t *q) {
    proj_full = 0;
    if (!proj_values_ok(q))
        return 0;
    uint32_t used = proj_packed_size(q), pos = 20;
    if (used > PROJ_STORE_SIZE) {
        proj_full = 1;
        return 0;
    } /* no destination mutation on failure */
    uint8_t *b = out->raw;
    memset(b, 0, PROJ_STORE_SIZE);
    proj_wr(b, PROJ_MAGIC, 4);
    proj_wr(b + 4, PROJ_STORE_SIZE, 4);
    proj_wr(b + 8, used, 2);
    b[10] = 1;
    b[11] = NTRK;
    b[12] = P_COUNT;
    b[13] = G_COUNT;
    b[14] = q->sel;
    b[15] = q->parts;
    b[16] = q->phys;
    for (uint32_t i = 0; i < G_COUNT; i++) {
        proj_wr(b + pos, (uint16_t)q->g[i], 2);
        pos += 2;
    }
    memcpy(b + pos, q->name, PROJ_NAME_LEN);
    pos += PROJ_NAME_LEN;
    for (uint32_t t = 0; t < NTRK; t++) {
        uint32_t e = q->t[t].engine, fm = proj_fm_kept(q, t);
        b[pos++] = e;
        b[pos++] = q->t[t].preset;
        b[pos++] = fm;
        if (fm) {
            memcpy(b + pos, q->fm6[t], FM6_PACKED);
            pos += FM6_PACKED;
        }
        memcpy(b + pos, q->vocal_id[t], 8);
        pos += 8;
        b[pos++] = q->mch[t];
        uint32_t map = pos;
        pos += PROJ_MAP;
        for (uint32_t i = 0; i < P_COUNT; i++)
            if (q->t[t].p[i] != proj_default(e,i)) {
                b[map + i / 8] |= 1u << (i & 7);
                b[pos++] = q->t[t].p[i] + 64;
            }
        map = pos;
        pos += 8;
        for (uint32_t i = 0; i < NSTEP; i++) {
            const step_t *s = &q->t[t].step[i];
            if (proj_step_empty(s))
                continue;
            uint32_t nn, x = proj_step_x(s, &nn);
            b[map + i / 8] |= 1u << (i & 7);
            b[pos++] = s->n | (s->time << 3) | ((s->flags & 3) << 5) | (x ? 128 : 0);
            if (x)
                b[pos++] = x;
            for (uint32_t j = 0; j < nn; j++)
                b[pos++] = s->note[j];
            b[pos++] = s->vel;
            if (x & 1) {
                b[pos++] = s->hit;
                b[pos++] = s->acc;
            }
            if (x & 2)
                b[pos++] = s->probability;
            if (x & 8)
                b[pos++] = s->flags & ~3u;
        }
    }
    b[pos++] = q->chain.count;
    pos += 3;
    for (uint32_t i = 0; i < CHAIN_ROWS; i++) {
        for (uint32_t t = 0; t < NTRK; t++)
            b[pos++] = q->chain.row[i].slot[t];
        b[pos++] = q->chain.row[i].repeat;
    }
    b[pos++] = q->motion.count;
    b[pos++] = q->motion.on;
    pos += 2;
    for (uint32_t i = 0; i < q->motion.count; i++) {
        const motion_event_t *m = &q->motion.event[i];
        proj_wr(b + pos, m->place, 2);
        pos += 2;
        b[pos++] = m->param;
        b[pos++] = (uint8_t)m->value;
    }
    if (pos + 4 != used)
        return 0; /* invariant checked by boundary tests */
    proj_wr(b + PROJ_END, proj_hash(b, PROJ_END), 4);
    return 1;
}
/* Bounds at every field; two-pass import validates without touching the caller.
 * Only one decoded project scratch is needed (the caller's); no project on stack. */
static int proj_decode(project_t *q, const uint8_t *b, uint32_t st) {
    if (st != PROJ_STORE_SIZE || proj_rd(b, 4) != PROJ_MAGIC || proj_rd(b + 4, 4) != st || b[10] != 1 ||
        b[11] != NTRK || (b[12] != P_COUNT && b[12] != 126) || b[13] != G_COUNT || b[14] >= NTRK || b[15] != NPART ||
        b[16] != PROJ_PHYS || b[17] || b[18] || b[19] || proj_rd(b + PROJ_END, 4) != proj_hash(b, PROJ_END))
        return 0;
    uint32_t count=b[12], pmap=(count+7u)/8u;
    uint32_t used = proj_rd(b + 8, 2), pos = 20;
    if (used < 24 || used > PROJ_STORE_SIZE)
        return 0;
    uint32_t end = used - 4;
    for (uint32_t i = end; i < PROJ_END; i++)
        if (b[i])
            return 0;
#define TAKE(n)                                                                                                        \
    do {                                                                                                               \
        if (pos > end || (uint32_t)(n) > end - pos)                                                                    \
            return 0;                                                                                                  \
    } while (0)
    if (q) {
        memset(q, 0, sizeof *q);
        q->magic = PROJ_MAGIC;
        q->size = sizeof *q;
        q->sel = b[14];
        q->parts = b[15];
        q->phys = b[16];
    }
    TAKE(G_COUNT * 2 + PROJ_NAME_LEN);
    for (uint32_t i = 0; i < G_COUNT; i++) {
        if (q)
            q->g[i] = (int16_t)proj_rd(b + pos, 2);
        pos += 2;
    }
    if (q)
        memcpy(q->name, b + pos, PROJ_NAME_LEN);
    pos += PROJ_NAME_LEN;
    for (uint32_t t = 0; t < NTRK; t++) {
        TAKE(3);
        uint32_t e = b[pos++], preset = b[pos++], fm = b[pos++];
        if (e >= NENGINES || e == ENGI_DIGITAL || fm > 1)
            return 0;
        if (q) {
            q->t[t].engine = e;
            q->t[t].preset = preset;
            memcpy(q->fm6[t], BG6_FM6_INIT, FM6_PACKED);
            for (uint32_t i = 0; i < P_COUNT; i++)
                q->t[t].p[i] = proj_default(e,i);
        }
        if (fm) {
            TAKE(FM6_PACKED);
            for (uint32_t i = 0; i < FM6_PACKED; i++)
                if (b[pos + i] > 127)
                    return 0;
            if (q)
                memcpy(q->fm6[t], b + pos, FM6_PACKED);
            pos += FM6_PACKED;
        }
        TAKE(9 + pmap);
        if (q)
            memcpy(q->vocal_id[t], b + pos, 8);
        pos += 8;
        if (b[pos] >= MCH_N)
            return 0;
        if (q)
            q->mch[t] = b[pos];
        pos++;
        uint32_t map = pos;
        pos += pmap;
        for (uint32_t i = count; i < pmap * 8; i++)
            if (b[map + i / 8] & (1u << (i & 7)))
                return 0;
        for (uint32_t i = 0; i < count; i++)
            if (b[map + i / 8] & (1u << (i & 7))) {
                TAKE(1);
                if (b[pos] > 191)
                    return 0;
                if (q)
                    q->t[t].p[proj_clarity_id(i,count)] = (int16_t)b[pos] - 64;
                pos++;
            }
        TAKE(8);
        map = pos;
        pos += 8;
        for (uint32_t i = 0; i < NSTEP; i++) {
            step_t s = {0};
            s.time = ST_REST;
            if (b[map + i / 8] & (1u << (i & 7))) {
                TAKE(1);
                uint32_t meta = b[pos++], x = 0, nn;
                s.n = meta & 7;
                s.time = (meta >> 3) & 3;
                s.flags = (meta >> 5) & 3;
                if (s.n > 4 || s.time > ST_REST)
                    return 0;
                if (meta & 128) {
                    TAKE(1);
                    x = b[pos++];
                    if (!x || (x & ~15u))
                        return 0;
                }
                nn = x & 4 ? 4 : s.n;
                TAKE(nn + 1);
                for (uint32_t j = 0; j < nn; j++) {
                    s.note[j] = b[pos++];
                    if (s.note[j] > 127)
                        return 0;
                }
                s.vel = b[pos++];
                if (s.vel > 127)
                    return 0;
                if (x & 1) {
                    TAKE(2);
                    s.hit = b[pos++];
                    s.acc = b[pos++];
                    if (s.acc & ~s.hit)
                        return 0;
                }
                if (x & 2) {
                    TAKE(1);
                    s.probability = b[pos++];
                    if (s.probability >= COND_FIRST + NCOND)
                        return 0;
                }
                if (x & 8) {
                    TAKE(1);
                    if (b[pos] & 3)
                        return 0;
                    s.flags |= b[pos++];
                }
            }
            if (q)
                q->t[t].step[i] = s;
        }
    }
    chain_config_t chain = {0};
    motion_store_t mo = {0};
    TAKE(4 + CHAIN_ROWS * (NTRK + 1) + 4);
    chain.count = b[pos++];
    if (b[pos] || b[pos + 1] || b[pos + 2])
        return 0;
    pos += 3;
    for (uint32_t i = 0; i < CHAIN_ROWS; i++) {
        for (uint32_t t = 0; t < NTRK; t++)
            chain.row[i].slot[t] = b[pos++];
        chain.row[i].repeat = b[pos++];
    }
    mo.count = b[pos++];
    mo.on = b[pos++];
    if (b[pos] || b[pos + 1])
        return 0;
    pos += 2;
    if (mo.count > MOTION_MAX)
        return 0;
    TAKE(mo.count * 4);
    for (uint32_t i = 0; i < mo.count; i++) {
        mo.event[i].place = proj_rd(b + pos, 2);
        pos += 2;
        uint32_t raw=b[pos++];
        if ((raw & 127u) >= count) return 0;
        mo.event[i].param = proj_clarity_id(raw & 127u,count) | (raw & MOTION_LOCK);
        mo.event[i].value = (int8_t)b[pos++];
    }
    if (pos != end || !chain_valid(&chain) || !motion_valid(&mo))
        return 0;
    if (q) {
        q->chain = chain;
        q->motion = mo;
        q->sum = proj_sum(q);
    }
    return 1;
#undef TAKE
}
static int proj_unpack(project_t *q, const uint8_t *b, uint32_t st) {
    return proj_decode(0, b, st) && proj_decode(q, b, st);
}
static int proj_import(project_t *q, const void *b, int n) {
    return n == PROJ_STORE_SIZE && proj_unpack(q, b, (uint32_t)n);
}

#ifndef PROJ_HOST
/* Decoded and serialized workspaces belong to the main loop, never the audio ISR. */
static project_t proj_scratch               /* decoded main-loop work, never audio ISR */
    __attribute__((section(".pool")));
#if FELUCCA_WEB
/* Browser interchange sources belong to the current runtime, never named slots. */
static project_t portable_source[4];
static uint8_t portable_mask, portable_session;
#endif
static char proj_name[PROJ_NAME_LEN + 1u]    /* the name of the music as it is now (loaded, saved, the editor's */
    __attribute__((section(".pool")));       /* runtime restore); "" = none. A save takes it unless one is given */
#define PROJ_NO_SLOT 0xFFu
static uint8_t proj_cur = PROJ_NO_SLOT;      /* the slot the music was loaded from or last saved to (a rename of it
                                              * renames the music too); PROJ_NO_SLOT none (the editor's restore) */
static union {                               /* serialized main-loop work; no retained expansion */
    project_store_t s;
    uint8_t raw[3840];                         /* (the staging of a backup object, up to a storage object: editor_backup.c) */
} proj_wire_u __attribute__((section(".pool")));
#define proj_wire (proj_wire_u.s)
static uint8_t proj_wire_gen;                /* +1 whenever proj_wire is rewritten (a backup's runtime copy lives there) */

static void proj_steps(step_t *s)            /* a loaded sequence stays inside its fixed fields */
{
    uint32_t i, j;
    for (i = 0; i < NSTEP; i++) {
        if (s[i].n > 4u) s[i].n = 4;
        if (s[i].time > ST_REST) s[i].time = ST_REST;
        for (j = 0; j < 4u; j++) s[i].note[j] &= 127u;
        s[i].acc &= s[i].hit;
    }
}

/* an imported older format may hold values FUN7 cannot pack: keep them inside the fields and ranges */
static void proj_bound(project_t *q)
{
    uint32_t t, i;
    for (t = 0; t < NTRK; t++) {
        proj_steps(q->t[t].step);
        for (i = 0; i < NSTEP; i++) {
            step_t *s = &q->t[t].step[i];
            s->flags &= 3u | SF_RATCH | SF_NUDGE;
            s->vel &= 127u;
            if (s->probability >= COND_FIRST + NCOND) s->probability = 0;
        }
        for (i = 0; i < P_COUNT; i++) {
            const param_desc_t *d = param_desc_of(q->t[t].engine % NENGINES, i);
            q->t[t].p[i] = (int16_t)param_fit(d, q->t[t].p[i]);   /* (a retired KIT: the kit it plays) */
        }
    }
    q->sum = proj_sum(q);
}

#if FELUCCA_FLASH
/* slot from flash into RAM (format 7, or format 6 / 5 / 4 / 3 / 2 / 1 converted) */
static void proj_fetch(uint32_t slot)
{
    project_store_t *q = &proj_slot[slot & 3u];
    int n;
    proj_wire_gen++;
    n = st_load(OBJ_PROJECT0 + (slot & 3u), &proj_wire, sizeof proj_wire);
    proj_old_slots &= (uint8_t)~(1u << (slot & 3u));
    if (!proj_import(&proj_scratch, &proj_wire, n)) {
        if (n > 0 && proj_rd(proj_wire.raw,4)!=PROJ_MAGIC) proj_old_slots |= (uint8_t)(1u << (slot & 3u));
        memset(q->raw, 0, 4);
    }
    else {
        proj_bound(&proj_scratch);
        if (!proj_pack(q, &proj_scratch))
            memset(q->raw, 0, 4);
    }
}
#endif

static void project_capture(project_t *p)
{
    uint32_t i;
    uint32_t f = motion_guard();
    memset(p, 0, sizeof *p);
    p->magic = PROJ_MAGIC;
    p->size = sizeof *p;
    for (i = 0; i < G_COUNT; i++)
        p->g[i] = song.g[i];
    p->sel = song.sel;
    p->parts = NPART;
    p->phys = PROJ_PHYS;
    p->chain = chain_config;
    for (i = 0; i < NTRK; i++) {
        for (uint32_t j = 0; j < P_COUNT; j++) p->t[i].p[j] = motion_base_value(&trk[i], j);
        p->t[i].p[P_VHOLD] = 0;
        p->t[i].engine = trk[i].eng_req;
        p->t[i].preset = trk[i].preset;
        memcpy(p->t[i].step, trk[i].step, sizeof trk[i].step);
        fm6_pack(fm6_patch[i], p->fm6[i]);
#if FELUCCA_SLICE
        if(p->t[i].engine==13) {
            uint32_t uses=p->t[i].p[P_E0]==SLC_SRC_VOCALS;
            for(uint32_t j=0;j<motion.count;j++) {
                const motion_event_t *e=&motion.event[j];
                if((e->place>>6)==i && MOTION_ID(e)==P_E0 && e->value==SLC_SRC_VOCALS) uses=1;
            }
            if(uses) memcpy(p->vocal_id[i],bg_vocal_expected[i],8);
        }
#endif
    }
    p->motion = motion;
    memcpy(p->mch, song.mch, sizeof p->mch);
    motion_unguard(f);
    memcpy(p->name, proj_name, str_len(proj_name));
    p->sum = proj_sum(p);
}

/* the music as it is now -> slot, named `name` (0: the current name; "" none); 0 saved, nonzero refused or failed.
 * The current name becomes the saved one */
static void proj_kept_now(void);
static int project_save_as(uint32_t slot, const char *name)
{
    project_t *p = &proj_scratch;
#if FELUCCA_WEB
    if (portable_mask) { ui_message("EXPORT SONG FILE"); return 1; }
#endif
    if (transport_busy()) {                            /* a flash erase silences the audio and stalls the */
        ui_message("STOP TO SAVE");                     /* sequencer (storage_hw.c): only while stopped */
        return 1;
    }
    project_capture(p);
    if (name) {
        memset(p->name, 0, sizeof p->name);
        memcpy(p->name, name, str_len(name) < PROJ_NAME_LEN ? str_len(name) : PROJ_NAME_LEN);
    }
    proj_wire_gen++;
    if (!proj_pack(&proj_wire, p)) { ui_message(proj_full ? "PROJECT FULL" : "SAVE FORMAT ERROR"); return 2; }

#if FELUCCA_FLASH
    if (flash_ok) {
        if (st_save(OBJ_PROJECT0 + (slot & 3u), &proj_wire, sizeof proj_wire)) {
            ui_message("SAVE ERROR");
            return 2;
        }
        memcpy(&proj_slot[slot & 3u], &proj_wire, sizeof proj_wire);
    proj_old_slots &= (uint8_t)~(1u << (slot & 3u));
        proj_name_get(proj_name, (const uint8_t *)p->name);
        proj_cur = (uint8_t)(slot & 3u);
        pat_from_set((slot & 3u) + 1u);
        proj_kept_now();
        ui_message("SAVED");
        return 0;
    }
#endif
    memcpy(&proj_slot[slot & 3u], &proj_wire, sizeof proj_wire);
    proj_old_slots &= (uint8_t)~(1u << (slot & 3u));
    proj_name_get(proj_name, (const uint8_t *)p->name);
    proj_cur = (uint8_t)(slot & 3u);
    pat_from_set((slot & 3u) + 1u);
    proj_kept_now();
    ui_message("SAVED (RAM)");
    return 0;
}
static uint32_t project_bytes(void) { project_capture(&proj_scratch); return proj_packed_size(&proj_scratch); }
static int project_save(uint32_t slot) { return project_save_as(slot, 0); }
static void project_cur_name(char *b) { str_cpy(b, proj_name, PROJ_NAME_LEN + 1u); }   /* b: 13 bytes */

/* slot's name -> b (PROJ_NAME_LEN + 1 bytes); 0 = an empty slot (b ""). Uses proj_scratch */
static int project_name(uint32_t slot, char *b) { return project_info(slot, b, 0, 0, 0); }
/* .. and, len48 not 0, the length of each track's pattern as SONG plays it (song_chain.c chain_tick: the clock track's
 * LEN steps of its DIV, clamped as chain_prepare does) in 1/48 quarter notes (a 4/4 bar: 192; 0 = an empty slot):
 * len48[0..NTRK-1] */
static uint32_t len48_of(int32_t len, int32_t div)
{
    uint32_t n = (uint32_t)clamp(len, TP[P_SLEN].min, TP[P_SLEN].max), d = (uint32_t)clamp(div, TP[P_SDIV].min, TP[P_SDIV].max);
    return n * (d < 6u ? 48u / DIV_DEN[d] : d < 10u ? 48u << (d - 5u) : 48u / DIV_DEN[d % 6u]);   /* (fx.c div_samples) */
}
/* SONG's picture of a pattern (ui_graph.c graph_song): its LEN steps in n = min(LEN, 16) cells (a cell: steps
 * c * LEN / n .. (c + 1) * LEN / n - 1), 2 bits a cell, cell c at bits 2c: a step weighs 0 (rest, empty, CHANCE 0),
 * 1 (a tie), 2 (a note or hit), 3 (accented); a cell's level is its steps' mean, rounded up (0..3) */
static uint32_t proj_density(const proj_trk_t *t, uint8_t *n)
{
    uint32_t len = (uint32_t)clamp(t->p[P_SLEN], 1, NSTEP), nc = len < 16u ? len : 16u, c, i, d = 0;
    for (c = 0; c < nc; c++) {
        uint32_t i0 = c * len / nc, i1 = (c + 1u) * len / nc, w = 0;
        for (i = i0; i < i1; i++) {
            const step_t *s = &t->step[i];
            if (s->time == ST_TIE)
                w += 1u;
            else if (s->time == ST_NOTE && (s->n || s->hit) && step_chance(s))
                w += (s->flags & SF_ACCENT) || s->acc ? 3u : 2u;
        }
        w = (w + (i1 - i0) - 1u) / (i1 - i0);
        d |= (w < 3u ? w : 3u) << (2u * c);
    }
    *n = (uint8_t)nc;
    return d;
}
/* .. and, den not 0, each track's picture (proj_density: den[k], its cells n[k]; 0 for an empty slot) */
static int project_info(uint32_t slot, char *b, uint16_t *len48, uint32_t *den, uint8_t *n)
{
    uint32_t k;
    b[0] = 0;
    if (len48)
        memset(len48, 0, NTRK * sizeof *len48);
    if (den) {
        memset(den, 0, NTRK * sizeof *den);
        memset(n, 0, NTRK);
    }
    if (!proj_import(&proj_scratch, &proj_slot[slot & 3u], sizeof(project_store_t)))
        return 0;
    proj_name_get(b, (const uint8_t *)proj_scratch.name);
    for (k = 0; len48 && k < NTRK; k++)
        len48[k] = (uint16_t)len48_of(proj_scratch.t[k].p[P_SLEN], proj_scratch.t[k].p[P_SDIV]);
    for (k = 0; den && k < NTRK; k++)
        den[k] = proj_density(&proj_scratch.t[k], &n[k]);
    return 1;
}

/* SONG metadata follows the same source precedence as playback. The PROJECT
 * browser deliberately keeps project_info's real named-slot metadata. */
static int song_source_info(uint32_t slot, char *b, uint16_t *len48, uint32_t *den, uint8_t *n)
{
#if FELUCCA_WEB
    if ((portable_mask >> (slot & 3u)) & 1u) {
        const project_t *p=&portable_source[slot & 3u];
        proj_name_get(b,(const uint8_t *)p->name);
        for(uint32_t k=0;len48 && k<NTRK;k++)
            len48[k]=(uint16_t)len48_of(p->t[k].p[P_SLEN],p->t[k].p[P_SDIV]);
        for(uint32_t k=0;den && k<NTRK;k++) den[k]=proj_density(&p->t[k],&n[k]);
        return 1;
    }
#endif
    return project_info(slot,b,len48,den,n);
}

/* a stored project renamed in place (nothing else of it changes; the music playing is not touched, but the slot
 * it was loaded from / saved to: its name is the new one, the next SAVE's prefill): 0 done, 1 refused (playing,
 * an empty slot), 2 failed (the slot as it was) */
static int project_rename(uint32_t slot, const char *name)
{
    project_t *p = &proj_scratch;
    if (transport_busy()) {
        ui_message("STOP TO SAVE");
        return 1;
    }
    if (!proj_import(p, &proj_slot[slot & 3u], sizeof(project_store_t))) {
        ui_message("EMPTY SLOT");
        return 1;
    }
    memset(p->name, 0, sizeof p->name);
    memcpy(p->name, name, str_len(name) < PROJ_NAME_LEN ? str_len(name) : PROJ_NAME_LEN);
    proj_wire_gen++;
    if (!proj_pack(&proj_wire, p)) { ui_message(proj_full ? "PROJECT FULL" : "SAVE FORMAT ERROR"); return 2; }
#if FELUCCA_FLASH
    if (flash_ok && st_save(OBJ_PROJECT0 + (slot & 3u), &proj_wire, sizeof proj_wire)) {
        ui_message("SAVE ERROR");
        return 2;
    }
#endif
    memcpy(&proj_slot[slot & 3u], &proj_wire, sizeof proj_wire);
    proj_old_slots &= (uint8_t)~(1u << (slot & 3u));
    if (proj_cur == (slot & 3u))
        proj_name_get(proj_name, (const uint8_t *)p->name);
#if FELUCCA_FLASH
    if (flash_ok) { ui_message("RENAMED"); return 0; }
#endif
    ui_message("RENAMED (RAM)");
    return 0;
}

/* 1.5.5 (#199): a stored project erased (SAVE > PROJECT, KNOB 2 ERASE, the dialog): the slot empty. Written as a save
 * is (st_save: the other copy of the A/B pair erased and an empty record committed last, as a full backup's restore
 * writes an empty slot), so a power cut leaves the project there or the slot empty, never half of either. The music
 * playing is not touched: it keeps its name; were it loaded from or last saved to this slot it now has none (the
 * header's slot letter goes). 0 erased, 1 refused (playing, an empty slot), 2 failed (the slot as it was) */
static int project_erase(uint32_t slot)
{
    uint32_t k;
    slot &= 3u;
    if (transport_busy()) {                            /* (a flash erase silences the audio: project_save_as) */
        ui_message("STOP TO ERASE");
        return 1;
    }
    if (!project_used(slot)) {
        ui_message("EMPTY SLOT");
        return 1;
    }
#if FELUCCA_FLASH
    if (flash_ok && st_save(OBJ_PROJECT0 + slot, &proj_wire, 0)) {
        ui_message("ERASE ERROR");
        return 2;
    }
#endif
    memset(&proj_slot[slot], 0, sizeof proj_slot[slot]);
    proj_old_slots &= (uint8_t)~(1u << slot);
    if (proj_cur == slot)
        proj_cur = PROJ_NO_SLOT;
    for (k = 0; k < NTRK; k++)
        if (pat_from[k] == slot + 1u)
            pat_from[k] = 0;
#if FELUCCA_FLASH
    if (flash_ok) {
        char b[20] = "PROJECT A ERASED";
        b[8] = (char)('A' + slot);
        ui_message(b);
        return 0;
    }
#endif
    ui_message("ERASED (RAM)");
    return 0;
}

static int project_restore_runtime(const project_t *input)
{
    project_t *p = &proj_scratch;
    uint32_t i, k;
    if (!proj_ok(input) || !proj_engines_ok(input)) return 1;
#if FELUCCA_WEB
    portable_mask = portable_session = 0; /* every successful new/load/restore detaches old sources */
#endif
    mom_back(0);                                  /* (ui_input.c MOMENTARY: the values as they were, then the load) */
    memset(drum_mute, 0, sizeof drum_mute);       /* (1.4.1: the DRUM lanes' mutes, performance state) */
    if (p != input) memcpy(p, input, sizeof *p);
    transport_req = 2;
    panic_req = (1u << NTRK) - 1u;
    fm1_irq_off();                                      /* the audio ISR must not see half a project */
    seq_stop();
    transport_req = 0;
    chain_config = p->chain;
    motion = p->motion;
    memset(motion_active, 0, sizeof motion_active);
    motion_base_valid = 0;
    ui.song_row = 0;
    for (i = 0; i < G_COUNT; i++)
        if (i != G_SLOT && i != G_LOAD && i != G_SAVE)
            song.g[i] = (int16_t)clamp(p->g[i], GP[i].min, GP[i].max);
    for (k = 0; k < NTRK; k++)                          /* MIDI IN TRACK's channels (1.5.5; 0, channels 1..4, before) */
        song.mch[k] = (uint8_t)(p->mch[k] < MCH_N ? p->mch[k] : 0u);
    midi_route = MIDI_ROUTE_AGAIN;                      /* (events_block: what the channels no longer hear let go) */
    for (k = 0; k < NTRK; k++) {
        track_t *t = &trk[k];
        dance_reset(k);
        crew_reset(k);
        const proj_trk_t *s = &p->t[k];
        uint32_t e = s->engine;                         /* (< NENGINES: proj_engines_ok) */
        t->eng_req = (uint8_t)e;
        t->user = 0;                                    /* (no user preset slot is saved) */
        for (i = 0; i < P_COUNT; i++) {                 /* every value back inside its range (param_fit) */
            const param_desc_t *d = param_desc_of(e, i);
            t->p[i] = (int16_t)param_fit(d, s->p[i]);
        }
        t->p[P_VHOLD] = 0;
        memcpy(bg_vocal_expected[k],p->vocal_id[k],8);
        t->preset = (uint8_t)(ENGINES[e]->npresets ? (s->preset) % ENGINES[e]->npresets : 0u);
        memcpy(t->step, s->step, sizeof t->step);
        proj_steps(t->step);
        {   /* the project's own FM6 patch, never reloaded from SLOT: F n if it is that factory patch, else OWN */
            uint8_t v[FP_SIZE + 1u];
            fm6_unpack(p->fm6[k], v);
            fm6_set_patch(k, v);
            fm6_adopt(k);
        }
    }
    song.sel = (uint8_t)(p->sel < NTRK ? p->sel : 0u);
    fm1_irq_on();
    proj_name_get(proj_name, (const uint8_t *)p->name);
    proj_cur = PROJ_NO_SLOT;                            /* (project_load: its slot) */
    pat_from_set(0);                                    /* (project_load: its slot; the header's letter) */
    undo.trk = 0;                                       /* (ui.c) the undo copy belongs to the old project */
    undo_depth++;                                       /* and these loads take none */
    for (k = 0; k < NTRK; k++) {                        /* the power-on sounds: format 1 (tracks 2..4), old drums */
        track_t *t = &trk[k];
        int16_t keep[P_COUNT];
        pat_sig[k] = ~steps_sig(t);                     /* a project's steps are the user's */
    }
    undo_depth--;
    sync_reload = 1;
    ui.force = 1;
    proj_kept_now();
    ui_message("LOADED");
    memset(snd_said, 0, sizeof snd_said);               /* a missing sample is said again, after LOADED */
    return 0;
}
/* MENU > SYSTEM > CLEAR ALL (1.8, #266): the music as a new project, loaded as one is: every track its power-on
 * engine and sound with no steps or automation, the song order, BPM and the rest of the project's values their
 * defaults, the FM6 init patch, no name and no slot. The stored projects, user presets, samples, settings and MIDI
 * LEARN stay (an autosave follows as after any change). 0 done, 1 refused (as project_restore_runtime) */
static int project_new(void)
{
    project_t *q = &proj_scratch;
    uint32_t i, k;
    memset(q, 0, sizeof *q);
    q->magic = PROJ_MAGIC;
    q->size = sizeof *q;
    q->parts = NPART;
    for (i = 0; i < G_COUNT; i++)
        q->g[i] = GP[i].def;
    chain_defaults(&q->chain);
    for (i = 0; i < NTRK; i++) {
        for (k = 0; k < P_COUNT; k++)
            q->t[i].p[k] = param_desc_of(trk_def_engine(i), k)->def;
        q->t[i].engine = (uint8_t)trk_def_engine(i);
        q->t[i].preset = PROJ_DEF_SOUND;                /* (project_restore_runtime: the power-on sound, no steps) */
        for (k = 0; k < NSTEP; k++)
            q->t[i].step[k].time = ST_REST;
    }
    proj_fm6_init(q);                                   /* (and the sum) */
    if (project_restore_runtime(q))
        return 1;
    undo_depth++;
    for (i = 0; i < NTRK; i++) { apply_preset_to(&trk[i], TRK_DEF[i][1]); track_defaults_steps(&trk[i]); }
    undo_depth--;undo.trk=0;proj_kept_now();
    ui_message("ALL CLEARED");
    return 0;
}
static void project_load(uint32_t slot)
{
#if FELUCCA_FLASH
    if (flash_ok && !proj_import(&proj_scratch, &proj_slot[slot & 3u], sizeof(project_store_t))) proj_fetch(slot);
#endif
    if (!proj_import(&proj_scratch, &proj_slot[slot & 3u], sizeof(project_store_t))) { ui_message((proj_old_slots & (1u<<(slot&3u))) ? "OLD FORMAT: OVERWRITE" : "EMPTY SLOT"); return; }
    proj_bound(&proj_scratch);                          /* a retained RAM slot of an older format: as from flash */
    if (!project_restore_runtime(&proj_scratch)) {
        proj_cur = (uint8_t)(slot & 3u);
        pat_from_set((slot & 3u) + 1u);
    }
}

/* settings + learned panel table: one flash object. The flash copy wins at
 * boot (the .noinit copies are garbage after a power-off). */
#include "settings_persist.c"
#if FELUCCA_FLASH && FELUCCA_SLICE
#include "slice_store.c"                          /* SLICE's MAN slices, kept in the user slots */
#endif
#if FELUCCA_FLASH
static persist_t persist_saved __attribute__((section(".pool")));
static uint8_t persist_pending;                 /* 1 requested, 2 waiting after a flash error */
static uint32_t persist_retry_ms;
#endif

static void persist_boot(void)                    /* before settings_init / panel_init */
{
#if FELUCCA_FLASH
    persist_t p;
    uint32_t f = irq_save();
    flash_ok = FL_FAR(fl_jedec_ram)() == 0x856014u;       /* the expected 1 MiB part, else stay RAM-only */
    irq_restore(f);
    if (!flash_ok)
        return;
    fl_plain_window_init();                        /* flash above 0x93000 reads as plaintext through XIP
                                                    * (user sample sets are played from there) */
#if FELUCCA_SLICE
    slc_store_boot();                              /* (the scans read each slot's stored slices) */
#endif
    {
        uint32_t k;
        for (k = 0; k < SMP_USER_SLOTS; k++)
            smp_user_scan(k);
    }
    {
        int n = st_load(OBJ_SETTINGS, &p, sizeof p);
        if (settings_import(&p, n))
            persist_saved = p;
    }
    {   /* projects: fill empty RAM slots from flash, so the slot list is right after power-on */
        uint32_t i;
        for (i = 0; i < 4u; i++)
            if (!proj_import(&proj_scratch, &proj_slot[i], sizeof(project_store_t)))
                proj_fetch(i);
    }
    up_boot();                                     /* user presets */
#endif
}

static int project_used(uint32_t slot) { return proj_import(&proj_scratch, &proj_slot[slot & 3u], sizeof(project_store_t)); }

/* Main loop only: no flash access or copies when the ISR changes rows. */
static uint32_t chain_prepare(void)
{
    uint32_t i, k, j, used = 0;
    if (transport_busy())
        return 2;
    if (!chain_valid(&chain_config) || !chain_config.count)
        return 1;
    for (i = 0; i < chain_config.count; i++)            /* the slots the lanes play (not the silent ones) */
        for (k = 0; k < NTRK; k++) {
            uint32_t s = chain_config.row[i].slot[k];
            if (s >= 4u || ((used >> s) & 1u))
                continue;
#if FELUCCA_WEB
            if ((portable_mask >> s) & 1u) { used |= 1u << s; continue; }
#endif
            if (!project_used(s))
                return 3u + s;
            used |= 1u << s;
        }
    chain.config = chain_config;
    for (i = 0; i < 4u; i++)
        if ((used >> i) & 1u) {
            project_t *p = &proj_scratch;
#if FELUCCA_WEB
            if ((portable_mask >> i) & 1u) memcpy(p, &portable_source[i], sizeof *p);
            else
#endif
            if (!proj_import(p, &proj_slot[i], sizeof(project_store_t))) return 3u + i;
            chain.source[i].motion = p->motion;
            /* A song keeps its current instruments. Engine-specific motion from
             * another instrument would change a kit/wave/algorithm unexpectedly. */
            {
                motion_store_t *m = &chain.source[i].motion; uint32_t n = 0;
                for (uint32_t e = 0; e < m->count; e++) {
                    const motion_event_t *v = &m->event[e]; uint32_t owner = v->place >> 6;
                    if (MOTION_ID(v) >= P_FM1_ATK && p->t[owner].engine != trk[owner].eng_req) continue;
                    m->event[n++] = *v;
                }
                m->count = (uint8_t)n;
            }
            for (k = 0; k < NTRK; k++) {
                memcpy(chain.source[i].step[k], p->t[k].step, sizeof p->t[k].step);
                proj_steps(chain.source[i].step[k]);
                chain.source[i].qnt[k] = p->t[k].p[P_SQNT] != 0;
                for (j = 0; j < 4u; j++)
                    chain.source[i].timing[k][j] = (int16_t)clamp(p->t[k].p[P_SLEN + j],
                        TP[P_SLEN + j].min, TP[P_SLEN + j].max);
            }
        }
    RING_PUBLISH();
    chain.armed = 1;
    transport_req = 1;
    return 0;
}

static void settings_poll(void)
{
#if FELUCCA_FLASH
    persist_t p;
#if FELUCCA_SLICE
    slc_store_poll();                              /* SLICE's slices edited on the SLICES page */
#endif
    if (!persist_pending || !flash_ok || transport_busy() ||
        (persist_pending == 2u && (uint32_t)(fm1_ms - persist_retry_ms) < 1000u))
        return;
    p = persist_saved;
    settings_export(&p);
    if (!memcmp(&p, &persist_saved, sizeof p)) {
        persist_pending = 0;
        return;                                    /* unchanged: no erase cycle */
    }
    if (st_save(OBJ_SETTINGS, &p, sizeof p) == 0) {
        persist_saved = p;
        persist_pending = 0;
    } else {
        persist_pending = 2;
        persist_retry_ms = fm1_ms;
    }
#endif
}

static void settings_save(void)
{
#if FELUCCA_FLASH
    persist_pending = 1;
#endif
    settings_poll();
}

/* The autosave (1.2, Discussion #130): the music as it is (what a project holds: every track's sound, steps, LEN DIV
 * SWING GATE, mix, the song, the automation and locks, the FM6 patches, the name) kept in its own storage object
 * (OBJ_AUTOSAVE, A/B at 0xE5000 / 0xE6000, never one of the four project slots), and put back at power-on
 * (autosave_boot) with MENU > SYSTEM > RESTORE LAST ON (the default). Flash wear: a 4 KiB erase per write, the two
 * sectors taking turns, so a write only when
 *   - something a project holds changed since the last one (autosave_sig: a hash of those values, no copy; not the
 *     selected track, nor the PROJECT page's own controls),
 *   - and it has stayed the same for AS_IDLE_MS (10 s) with the transport stopped, no key or button down, no voice
 *     sounding and no editor transfer (a backup or a sample: autosave_hold); an erase stops the audio for its
 *     ~40 ms, as every save does, so it never happens while anything plays,
 *   - and at least AS_GAP_MS (60 s) after the one before (AS_RETRY_MS after a failed write).
 * So at most one write a minute while editing, none while playing or idle: about 100,000 erase cycles per sector
 * give over 200,000 writes, a write a minute for 8 hours a day for over a year, far more with real pauses.
 * RESTORE LAST OFF writes none (and restores none). Never after a crash, a hang or a failed boot (main.c): the music
 * that was playing then could be what crashed. Checked every AS_POLL_MS; the main loop only (never the audio ISR) */
#define AS_POLL_MS 250u
#define AS_IDLE_MS 10000u
#define AS_GAP_MS 60000u
#define AS_RETRY_MS 30000u
static struct {
    uint32_t saved, seen;                        /* the signature written last (or loaded at power-on), seen last */
    uint32_t t, last, poll;                      /* fm1_ms: seen changed / busy, the last write, the last check */
    uint32_t writes;                             /* (written since power-on: the host tests count them) */
    uint32_t full_sig;
    uint8_t full_blocked;
    uint8_t wrote, err;                          /* a write since power-on (last is valid); the last one failed */
} as __attribute__((section(".pool")));

static uint32_t as_mix(uint32_t h, const void *p, uint32_t n)   /* FNV-1a, continued */
{
    const uint8_t *b = p;
    while (n--) {
        h ^= *b++;
        h *= 16777619u;
    }
    return h;
}

/* what an autosave would hold, hashed (no copy, no RAM): project_capture's values */
static uint32_t autosave_sig(void)
{
    uint32_t h = 2166136261u, i, j;
    for (i = 0; i < NTRK; i++) {
        const track_t *t = &trk[i];
        for (j = 0; j < P_COUNT; j++) {
            int16_t v = motion_base_value(t, j);
            h = as_mix(h, &v, sizeof v);
        }
        if(t->eng_req==13) h=as_mix(h,bg_vocal_expected[i],8);
        h = as_mix(h, &t->eng_req, 1);
        h = as_mix(h, &t->preset, 1);
        h = as_mix(h, t->step, sizeof t->step);
        h = as_mix(h, fm6_patch[i], sizeof fm6_patch[i]);
    }
    for (i = 0; i < G_COUNT; i++)
        if (i != G_SLOT && i != G_NAME && i != G_LOAD && i != G_SAVE)
            h = as_mix(h, &song.g[i], sizeof song.g[i]);
    h = as_mix(h, song.mch, sizeof song.mch);           /* (1.5.5: MIDI IN TRACK's channels, the project's) */
    h = as_mix(h, &motion, sizeof motion);
    h = as_mix(h, &chain_config, sizeof chain_config);
    return as_mix(h, proj_name, sizeof proj_name);
}

/* 1.8: the music as a stored project holds it, when it last was one (a PROJECT load or save, CLEAR ALL, power-on):
 * PROJECT LOAD asks first when it has changed since (ui_input.c CF_LOAD_PROJ) */
static uint32_t proj_kept;
static void proj_kept_now(void) { proj_kept = autosave_sig(); }
static int proj_changed(void) { return autosave_sig() != proj_kept; }

/* nothing going on that an erase (the audio stopped ~40 ms) would be heard in, or would race */
static int autosave_quiet(void)
{
    uint32_t k, i;
    if (transport_busy() || fm1_in.notes || fm1_in.buttons)
        return 0;
    for (k = 0; k < NTRK; k++)
        for (i = 0; i < NVOICE; i++)
            if (trk[k].v[i].active)
                return 0;
    return 1;
}

static void autosave_hold(void) { as.t = fm1_ms; }   /* the editor's transfers: their staging RAM is proj_wire */

/* main loop, every pass (after settings_poll) */
static void autosave_poll(void)
{
#if FELUCCA_WEB
    /* A B6S1 autosave cannot own portable source patterns. Browser checkpoints
     * retain the whole logical song instead; never persist dangling slot rows. */
    if (portable_session) return;
#endif
#if FELUCCA_FLASH
    uint32_t sig;
    if (!flash_ok || fm1_ms - as.poll < AS_POLL_MS)
        return;
    as.poll = fm1_ms;
    sig = autosave_sig();
    if (sig != as.seen || !autosave_quiet()) {
        as.seen = sig;
        as.t = fm1_ms;
        return;
    }
    if ((as.full_blocked && sig == as.full_sig) || sig == as.saved || (ui_prefs & PREF_RESTORE_OFF) || fm1_ms - as.t < AS_IDLE_MS ||
        (as.wrote && fm1_ms - as.last < (as.err ? AS_RETRY_MS : AS_GAP_MS)))
        return;
    as.last = fm1_ms;
    as.wrote = 1;
    project_capture(&proj_scratch);
    proj_wire_gen++;
    as.full_blocked = 0;
    if (!proj_pack(&proj_wire, &proj_scratch)) {
        as.err = 1;
        if (proj_full) { as.full_blocked = 1; as.full_sig = sig; ui_message("PROJECT FULL"); }
        return;
    }
    as.err = st_save(OBJ_AUTOSAVE, &proj_wire, sizeof proj_wire) != 0;
    if (!as.err) {
        as.saved = sig;
        as.writes++;
    }
#endif
}

/* power-on, after the defaults (main.c): the autosave becomes the music when `allowed` (no crash, hang or failed boot
 * before) and RESTORE LAST is ON; an autosave that does not load (none, torn, damaged, another format) is ignored.
 * The music now (restored or not) is what is saved: nothing is written until it changes. 1 = restored */
static int autosave_boot(int allowed)
{
    int rc = 0;
#if FELUCCA_FLASH
    if (flash_ok && allowed && !(ui_prefs & PREF_RESTORE_OFF)) {
        int n;
        proj_wire_gen++;
        n = st_load(OBJ_AUTOSAVE, &proj_wire, sizeof proj_wire);
        if (n > 0 && proj_rd(proj_wire.raw, 4) != PROJ_MAGIC) ui_message("OLD FORMAT: NEW SONG");
        if (n > 0 && proj_import(&proj_scratch, &proj_wire, n)) {
            proj_bound(&proj_scratch);
            if (!project_restore_runtime(&proj_scratch)) {
                ui_message("RESTORED");
                rc = 1;
            }
        }
    }
#else
    (void)allowed;
#endif
    as.saved = as.seen = autosave_sig();
    as.t = fm1_ms;
    proj_kept_now();
    return rc;
}

#if FELUCCA_FLASH
_Static_assert(sizeof(project_store_t) <= ST_PAYLOAD_MAX, "project does not fit one flash sector");
_Static_assert(sizeof(persist_t) <= ST_PAYLOAD_MAX, "settings do not fit one flash sector");
#endif
#endif /* PROJ_HOST */

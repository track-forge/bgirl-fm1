/* SPDX-License-Identifier: GPL-3.0-only
 * Core0-owned bounded activity; see docs/CREW_ACTIVITY_CONTRACT.md. */
#ifndef BGIRL_CREW_TELEMETRY
#define BGIRL_CREW_TELEMETRY 1
#endif
#define CREW_ACTIVE 256u
#define CREW_SETTLE 512u
#define CREW_MUTED 1024u
#define CREW_SEEN 8192u
static uint32_t crew_state[NTRK];
typedef struct { uint32_t pulse, settle, accent; uint8_t pending, strength, blocked; } crew_runtime_t;
static crew_runtime_t crew_rt[NTRK];
#define CREW_ODD_CYCLE (1u << 14)
static uint64_t crew_phase;
static uint32_t crew_read(uint32_t i) { return BG_SNAPSHOT_LOAD(&crew_state[i]); }
static void crew_reset(uint32_t i) {
    crew_rt[i]=(crew_runtime_t){0};
    BG_SNAPSHOT_STORE(&crew_state[i], 0u);
}
static void crew_attack(uint32_t i,uint32_t vel) {
#if BGIRL_CREW_TELEMETRY
    if(vel>crew_rt[i].pending) crew_rt[i].pending=(uint8_t)vel;
#else
    (void)i;(void)vel;
#endif
}
static uint32_t crew_sub(uint32_t v,uint32_t n) { return v>n ? v-n : 0; }

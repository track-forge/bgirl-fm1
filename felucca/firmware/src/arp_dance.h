/* SPDX-License-Identifier: GPL-3.0-only
 * Display-only arp telemetry. Included by core.h for all host/firmware builds.
 * Writes belong to core 0's sequencer / existing all-off paths (UI mutations
 * exclude its audio ISR); core 1 never advances it. A single aligned, lock-free
 * atomic word publishes the WHOLE snapshot. No volatile-only handoff, queue,
 * retry loop, IRQ masking, RNG, graphics or read of UI state in the producer.
 * Relaxed is sufficient: no other memory is published through this word.
 * Bits 0..3 current, 4..7 previous, 8 previous valid, 9 started.
 */
#ifndef BGIRL_DANCE_TELEMETRY
#define BGIRL_DANCE_TELEMETRY 1
#endif
_Static_assert(__atomic_always_lock_free(sizeof(uint32_t), 0), "dance needs lock-free 32-bit load/store");
static uint32_t dance_state[NTRK];
static uint32_t dance_read(uint32_t i) { return __atomic_load_n(&dance_state[i], __ATOMIC_RELAXED); }
static void dance_reset(uint32_t i) { __atomic_store_n(&dance_state[i], 0u, __ATOMIC_RELAXED); }
static void dance_trigger(uint32_t i)
{
#if BGIRL_DANCE_TELEMETRY
    uint32_t s = dance_read(i), p = s & 15u;
    s = s & 512u ? 512u | 256u | (p << 4) | (p == 11u ? 0u : p + 1u) : 512u;
    __atomic_store_n(&dance_state[i], s, __ATOMIC_RELAXED);
#else
    (void)i;
#endif
}

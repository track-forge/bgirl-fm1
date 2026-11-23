/* SPDX-License-Identifier: GPL-3.0-only */
#ifndef BGIRL_FM1_SNAPSHOT_H
#define BGIRL_FM1_SNAPSHOT_H
/* Naturally aligned SRAM words, accessed in one instruction. Core 0 owns
 * both sides; foreground reset paths exclude the audio IRQ. The render
 * worker never accesses these words. Use the chip's csync ordering, not
 * the compiler's unsupported atomic load/store lowering (CAS/exchange). */
static inline uint32_t fm1_snapshot_load(const uint32_t *p)
{
    uint32_t word;
    __asm__ volatile("%0 = [%1]\n\tcsync" : "=r"(word) : "r"(p) : "memory");
    return word;
}

static inline void fm1_snapshot_store(uint32_t *p, uint32_t word)
{
    __asm__ volatile("csync\n\t[%0] = %1" :: "r"(p), "r"(word) : "memory");
}
#define BG_SNAPSHOT_LOAD fm1_snapshot_load
#define BG_SNAPSHOT_STORE fm1_snapshot_store
#endif

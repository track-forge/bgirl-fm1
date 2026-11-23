/* SPDX-License-Identifier: GPL-3.0-only */
/* Starter bank transfer: fixed content and bounds; the validated header commits last. */
#define ED_MIAMI 82u
#if FELUCCA_FLASH && FELUCCA_SLICE && defined(BGIRL_MIAMI_SLOT)
static const uint8_t MIAMI_PENDING[] = {'B','G','V','P'};
static uint8_t miami_upload_slot = SMP_USER_SLOTS;
static uint32_t miami_upload_off;

static int ed_miami_owned(uint32_t k)
{
    const uint8_t *slot = smp_user_xip(k);
    return !memcmp(slot, MIAMI_HEADER, 4) ||
           !memcmp(slot + 256u, MIAMI_PENDING, sizeof MIAMI_PENDING);
}

enum { MIAMI_EMPTY, MIAMI_READY, MIAMI_RESTORE, MIAMI_OCCUPIED };

static uint32_t ed_miami_state(uint32_t k)
{
    const uint8_t *slot = smp_user_xip(k);
    if (ed_miami_owned(k))
        return miami_valid(slot) ? MIAMI_READY : MIAMI_RESTORE;
    /* A blank header alone does not prove empty (e.g. interrupted ordinary upload). */
    for (uint32_t i = 0; i < SMP_USER_SIZE; i++) {
        if ((i & 4095u) == 0) fm1_wdt_feed();
        if (slot[i] != 0xffu) return MIAMI_OCCUPIED;
    }
    return MIAMI_EMPTY;
}

static uint32_t ed_miami_begin(const uint8_t *a, uint32_t na)
{
    uint32_t k = a[1];
    if (na < 4u || ed_unpack7(a + 3, na - 3u, ed_smp_buf, sizeof MIAMI_HEADER) != sizeof MIAMI_HEADER ||
        memcmp(ed_smp_buf, MIAMI_HEADER, sizeof MIAMI_HEADER))
        return 2;
    if (miami_slot) return 3; /* matching content anywhere: no rewrite or duplicate */
    uint32_t state = ed_miami_state(k);
    if (state == MIAMI_READY || a[2] != state) return 3;
    if (ed_flash_stop()) return 4;
    miami_upload_slot = SMP_USER_SLOTS;
    if (ed_smp_erase(k, 0)) return 4;
    memcpy(ed_smp_buf, MIAMI_PENDING, sizeof MIAMI_PENDING);
    int rc = fl_write(ed_smp_slot(k) + 256u, ed_smp_buf, sizeof MIAMI_PENDING);
    ed_smp_inval(k);
    if (rc || !ed_miami_owned(k)) return 4;
    miami_upload_slot = k;
    miami_upload_off = SMP_USER_DATA;
    return 0;
}

static uint32_t ed_miami_chunk(uint32_t k, const uint8_t *a, uint32_t na)
{
    const uint32_t end = SMP_USER_DATA + MIAMI_PAYLOAD_BYTES;
    if (na < 4u || k != miami_upload_slot || miami_upload_off >= end) return 1;
    uint32_t off = a[0] | (uint32_t)a[1] << 7 | (uint32_t)a[2] << 14;
    uint32_t len = ed_unpack7(a + 3, na - 3u, ed_smp_buf, 256u);
    uint32_t remaining = end - miami_upload_off;
    if (off != miami_upload_off || len != (remaining < 256u ? remaining : 256u)) return 1;
    if (ed_flash_stop()) return 4;
    uint32_t addr = ed_smp_slot(k) + off, took;
    if (!(off & 4095u)) {
        audio_silence();
        if (fl_erase4k(addr, &took)) return 4;
    }
    if (fl_write(addr, ed_smp_buf, len)) return 4;
    miami_upload_off += len;
    return 0;
}

static uint32_t ed_miami_end(uint32_t k)
{
    if (k != miami_upload_slot || miami_upload_off != SMP_USER_DATA + MIAMI_PAYLOAD_BYTES)
        return 1;
    miami_upload_slot = SMP_USER_SLOTS;
    ed_smp_inval(k);
    if (st_crc32(smp_user_xip(k) + SMP_USER_DATA, MIAMI_PAYLOAD_BYTES) != MIAMI_PAYLOAD_CRC)
        return 2;
    memcpy(ed_smp_buf, MIAMI_HEADER, sizeof MIAMI_HEADER);
    if (ed_flash_stop() || fl_write(ed_smp_slot(k), ed_smp_buf, sizeof MIAMI_HEADER))
        return 4;
    ed_smp_inval(k);
    miami_scan();
    ui.force = 1;
    return miami_slot == smp_user_xip(k) ? 0 : 4;
}

static void ed_miami(const uint8_t *a, uint32_t na)
{
    if (!flash_ok || !na) return;
    autosave_hold();
    if (a[0] == 0u && na == 1u) {
        ed_b(1); /* protocol version */
        for (uint32_t k = 0; k < SMP_USER_SLOTS; k++) ed_b(ed_miami_state(k));
        return;
    }
    if (na < 2u || a[1] >= SMP_USER_SLOTS) return;
    uint32_t rc = 1;
    if (a[0] == 1u) rc = ed_miami_begin(a, na);
    if (a[0] == 2u) rc = ed_miami_chunk(a[1], a + 2, na - 2u);
    if (a[0] == 3u && na == 2u) rc = ed_miami_end(a[1]);
    ed_b(a[0]);
    ed_b(a[1]);
    ed_b(rc);
}
#else
#define ed_miami_owned(k) 0
#endif

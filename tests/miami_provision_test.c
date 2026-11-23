/* SPDX-License-Identifier: GPL-3.0-only */
#include "../felucca/web/emu/felucca_web.c"
#include <assert.h>

static uint8_t reply[664];
static uint32_t reply_len, so_r, so_w;
#define SXQ 64u
static void audio_silence(void) {}
static void fl_inval(uint32_t off, uint32_t n) { (void)off; (void)n; }
static int fl_erase4k(uint32_t off, uint32_t *took) { *took = 0; return st_erase(off); }
static int fl_write(uint32_t off, const void *p, uint32_t n) { return st_prog(off, p, n); }
static void ota_wire_send(const uint8_t *p, uint32_t n) { memcpy(reply, p, n); reply_len = n; }
static int ota_frame_get(const uint8_t **p, uint32_t *n) { (void)p; (void)n; return 0; }
static void ota_frame_done(void) {}
#include "../felucca/firmware/src/editor.c"

static const uint32_t image_size = SMP_USER_DATA + MIAMI_PAYLOAD_BYTES;
static uint8_t image[SMP_USER_DATA + MIAMI_PAYLOAD_BYTES];
static uint8_t frame[310] = {ED_HDR0, ED_HDR1, ED_HDR2, ED_MIAMI};

static void packet(uint8_t *dst, const uint8_t *src, uint32_t n)
{
    while (n) {
        uint32_t len = n > 7 ? 7 : n;
        uint8_t *mask = dst++;
        *mask = 0;
        for (uint32_t j = 0; j < len; j++) {
            *mask |= (*src >> 7) << j;
            *dst++ = *src++ & 127u;
        }
        n -= len;
    }
    ed_handle(frame, (uint32_t)(dst - frame));
    assert(reply[3] == 0x36 && reply[4] == frame[3]);
}
static uint8_t begin(uint8_t state)
{
    frame[4] = 1; frame[5] = 0; frame[6] = state;
    packet(frame + 7, image, 24);
    return reply[7];
}
static void chunk(uint32_t off)
{
    frame[4] = 2; frame[5] = 0;
    frame[6] = off & 127u; frame[7] = (off >> 7) & 127u; frame[8] = off >> 14;
    uint32_t len = image_size - off;
    packet(frame + 9, image + off, len > 256 ? 256 : len);
    assert(reply_len == 9 && !reply[7]);
}
static uint8_t end(void)
{
    frame[4] = 3; frame[5] = 0;
    packet(frame + 6, image, 0);
    return reply[7];
}
int main(int argc, char **argv)
{
    assert(argc == 2);
    FILE *file = fopen(argv[1], "rb");
    assert(file && fread(image, 1, image_size, file) == image_size && fclose(file) == 0);
    web_nor_erase(); web_boot();
    assert(ed_miami_state(0) == MIAMI_EMPTY);
    nor[SMP_USER_BASE + SMP_USER_DATA] = 42;
    nor[SMP_USER_BASE + SMP_USER_SIZE] = 43;
    assert(begin(MIAMI_EMPTY) == 3 && nor[SMP_USER_BASE + SMP_USER_DATA] == 42);
    assert(begin(MIAMI_OCCUPIED) == 0);
    chunk(SMP_USER_DATA);
    assert(end() == 1);
    miami_upload_slot = SMP_USER_SLOTS; /* power cut discards upload cursor */
    miami_scan();
    assert(!miami_slot && ed_miami_state(0) == MIAMI_RESTORE);
    assert(begin(MIAMI_RESTORE) == 0);
    for (uint32_t off = SMP_USER_DATA; off < image_size; off += 256) chunk(off);
    nor[SMP_USER_BASE + MIAMI_AUDIO_OFF] ^= 1;
    assert(end() == 2 && ed_miami_state(0) == MIAMI_RESTORE);
    assert(begin(MIAMI_RESTORE) == 0);
    for (uint32_t off = SMP_USER_DATA; off < image_size; off += 256) chunk(off);
    assert(end() == 0 && ed_miami_state(0) == MIAMI_READY);
    uint32_t writes = web_flash_writes;
    assert(begin(MIAMI_READY) == 3);
    const uint8_t blocked[] = {ED_SMP_BEGIN, ED_SMP_ERASE, ED_SMP_END, ED_SMP_WRITE};
    for (uint32_t i = 0; i < NELEM(blocked); i++) {
        frame[3] = blocked[i];
        memset(frame + 4, 0, 5);
        frame[6] = 4; /* WRITE offset 512 */
        if (blocked[i] == ED_SMP_WRITE) packet(frame + 8, image, 1);
        else if (blocked[i] == ED_SMP_END) packet(frame + 5, image, 1);
        else packet(frame + 5, image, 0);
        assert(reply[blocked[i] == ED_SMP_WRITE ? 9 : 6] != 0);
    }
    assert(web_flash_writes == writes && miami_valid(smp_user_xip(0)));
    assert(nor[SMP_USER_BASE + SMP_USER_SIZE] == 43 && ed_miami_state(2) == MIAMI_EMPTY);
    puts("Miami provisioning: occupied guard, interruption, corruption, restore, no rewrite, protected editor and adjacent slots PASS");
}

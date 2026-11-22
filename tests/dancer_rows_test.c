/* SPDX-License-Identifier: GPL-3.0-only */
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "../assets/dance/bgirl_dance.h"
#include "../assets/crew/windmill/bgirl_windmill.h"
#include "../assets/crew/reference-02/bgirl_reference_02.h"
#include "../assets/crew/wave-ish/bgirl_wave_ish.h"
#include "../assets/crew/worm/bgirl_worm.h"

static uint8_t actual[96 * 12];
static void cv_pset(int32_t x, int32_t y, uint16_t color)
{
    assert(x >= 132 && x < 228 && y >= 22 && y < 118 && color == 1);
    x -= 132;
    y -= 22;
    actual[y * 12 + x / 8] |= 128u >> (x % 8);
}

#include "../felucca/firmware/src/dancer_rows.h"

int main(void)
{
    const uint8_t (*const masks[])[1152] = {
        bg_windmill_masks, bg_reference_02_masks, bg_wave_ish_masks, bg_worm_masks, bg_dance_masks
    };
    for (uint32_t move = 0; move < 5; move++) {
        for (uint32_t pose = 0; pose < 12; pose++) {
            for (uint32_t shift = 0; shift < 96; shift++) {
                uint8_t expected[sizeof actual] = {0};
                memset(actual, 0, sizeof actual);
                draw_dancer_rows(move, pose, 132, shift, 1);
                for (uint32_t y = 0; y < 96; y++) {
                    for (uint32_t x = 0; x + shift < 96; x++) {
                        if (masks[move][pose][y * 12 + x / 8] & (128u >> (x % 8)))
                            expected[y * 12 + (x + shift) / 8] |= 128u >> ((x + shift) % 8);
                    }
                }
                assert(!memcmp(actual, expected, sizeof actual));
            }
        }
    }
    printf("All 60 dancer poses and 96 shadow offsets match raw masks; %zu packed bytes\n",
           sizeof dancer_rows + sizeof dancer_pose_offsets);
}

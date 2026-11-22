/* SPDX-License-Identifier: GPL-3.0-only */
#include "bgirl_dancer_rows.h"

enum { DANCER_FLOORWORK = 4, DANCER_WIDTH = 96 };

/* Each row is a start byte/length nibble pair followed by the occupied span.
 * Draw directly into the existing canvas, omitting shadow pixels off the right. */
static void draw_dancer_rows(uint32_t move, uint32_t pose, int32_t left,
                             uint32_t shift, uint16_t color)
{
    const uint8_t *row = dancer_rows + dancer_pose_offsets[move * 12u + pose];
    for (uint32_t y = 0; y < DANCER_WIDTH; y++) {
        uint32_t span = *row++;
        uint32_t x = (span >> 4) * 8u + shift;
        for (uint32_t i = 0; i < (span & 15u); i++) {
            uint32_t bits = *row++;
            for (uint32_t bit = 128u; bits && bit && x < DANCER_WIDTH; bit >>= 1, x++) {
                if (bits & bit) {
                    cv_pset(left + (int32_t)x, 22 + (int32_t)y, color);
                    bits ^= bit;
                }
            }
            x = (span >> 4) * 8u + shift + (i + 1u) * 8u;
        }
    }
}

#include "mfrc522_picc.h"
#include "common.h"
#include <string.h>

/*
 * PICC memory organization:
 *
 *                  +---------------------------------------------------------------------+
 *                  |                      Byte number within a block                     |
 * +--------+-------+---+---+---+---+---+---+---+---+---+---+----+----+----+----+----+----+------------------+
 * | Sector | Block | 0 | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 | 9 | 10 | 11 | 12 | 13 | 14 | 15 |    Description   |
 * +--------+-------+---+---+---+---+---+---+---+---+---+---+----+----+----+----+----+----+------------------+
 * |        |   3   |         Key A         |  Access Bits  |            Key B            |  Sector Trailer  |
 * |        +-------+-----------------------+---------------+-----------------------------+------------------+
 * |        |   2   |                                                                     |   Data block 2   |
 * |   15   +-------+---------------------------------------------------------------------+------------------+
 * |        |   1   |                                                                     |   Data block 1   |
 * |        +-------+---------------------------------------------------------------------+------------------+
 * |        |   0   |                                                                     |   Data block 0   |
 * +--------+-------+---------------------------------------------------------------------+------------------+
 * |                                                   ...                                                   |
 * +--------+-------+-----------------------+---------------+-----------------------------+------------------+
 * |        |   3   |         Key A         |  Access Bits  |            Key B            |  Sector Trailer  |
 * |        +-------+-----------------------+---------------+-----------------------------+------------------+
 * |        |   2   |                                                                     |   Data block 2   |
 * |    1   +-------+---------------------------------------------------------------------+------------------+
 * |        |   1   |                                                                     |   Data block 1   |
 * |        +-------+---------------------------------------------------------------------+------------------+
 * |        |   0   |                                                                     |   Data block 0   |
 * +--------+-------+-----------------------+---------------+-----------------------------+------------------+
 * |        |   3   |         Key A         |  Access Bits  |            Key B            |  Sector Trailer  |
 * |        +-------+-----------------------+---------------+-----------------------------+------------------+
 * |        |   2   |                                                                     |   Data block 2   |
 * |    0   +-------+---------------------------------------------------------------------+------------------+
 * |        |   1   |                                                                     |   Data block 1   |
 * |        +-------+---------------------------------------------------------------------+------------------+
 * |        |   0   |                          Manufacturer block                         |   Data block 0   |
 * +--------+-------+---------------------------------------------------------------------+------------------+
 */

/* ------------------------------------------------------------ */
/* ----------------------- Private variables ------------------ */
/* ------------------------------------------------------------ */

/* Allowed access conditions for the sector trailer */
static const mfrc522_picc_trailer_acc trailer_acc_lut[] =
{
    { /* mfrc522_picc_accb_000 (Key B may be read) */
        .key_a_write = mfrc522_picc_acc_type_key_a,
        .access_bits_read = mfrc522_picc_acc_type_key_a,
        .access_bits_write = mfrc522_picc_acc_type_never,
        .key_b_read = mfrc522_picc_acc_type_key_a,
        .key_b_write = mfrc522_picc_acc_type_key_a,
    },
    { /* mfrc522_picc_accb_001 (Transport configuration, Key B may be read) */
        .key_a_write = mfrc522_picc_acc_type_key_a,
        .access_bits_read = mfrc522_picc_acc_type_key_a,
        .access_bits_write = mfrc522_picc_acc_type_key_a,
        .key_b_read = mfrc522_picc_acc_type_key_a,
        .key_b_write = mfrc522_picc_acc_type_key_a,
    },
    { /* mfrc522_picc_accb_010 (Key B may be read)  */
        .key_a_write = mfrc522_picc_acc_type_never,
        .access_bits_read = mfrc522_picc_acc_type_key_a,
        .access_bits_write = mfrc522_picc_acc_type_never,
        .key_b_read = mfrc522_picc_acc_type_key_a,
        .key_b_write = mfrc522_picc_acc_type_never,
    },
    { /* mfrc522_picc_accb_011 */
        .key_a_write = mfrc522_picc_acc_type_key_b,
        .access_bits_read = mfrc522_picc_acc_type_key_both,
        .access_bits_write = mfrc522_picc_acc_type_key_b,
        .key_b_read = mfrc522_picc_acc_type_never,
        .key_b_write = mfrc522_picc_acc_type_key_b,
    },
    { /* mfrc522_picc_accb_100 */
        .key_a_write = mfrc522_picc_acc_type_key_b,
        .access_bits_read = mfrc522_picc_acc_type_key_both,
        .access_bits_write = mfrc522_picc_acc_type_never,
        .key_b_read = mfrc522_picc_acc_type_never,
        .key_b_write = mfrc522_picc_acc_type_key_b,
    },
    { /* mfrc522_picc_accb_101 */
        .key_a_write = mfrc522_picc_acc_type_never,
        .access_bits_read = mfrc522_picc_acc_type_key_both,
        .access_bits_write = mfrc522_picc_acc_type_key_b,
        .key_b_read = mfrc522_picc_acc_type_never,
        .key_b_write = mfrc522_picc_acc_type_never,
    },
    { /* mfrc522_picc_accb_110 */
        .key_a_write = mfrc522_picc_acc_type_never,
        .access_bits_read = mfrc522_picc_acc_type_key_both,
        .access_bits_write = mfrc522_picc_acc_type_never,
        .key_b_read = mfrc522_picc_acc_type_never,
        .key_b_write = mfrc522_picc_acc_type_never,
    },
    { /* mfrc522_picc_accb_111 */
        .key_a_write = mfrc522_picc_acc_type_never,
        .access_bits_read = mfrc522_picc_acc_type_key_both,
        .access_bits_write = mfrc522_picc_acc_type_never,
        .key_b_read = mfrc522_picc_acc_type_never,
        .key_b_write = mfrc522_picc_acc_type_never,
    }
};

/* Allowed access conditions for the block */
static const mfrc522_picc_block_acc block_acc_lut[] =
{
    { /* mfrc522_picc_accb_000 (Transport configuration) */
        .read = mfrc522_picc_acc_type_key_both,
        .write = mfrc522_picc_acc_type_key_both,
        .increment = mfrc522_picc_acc_type_key_both,
        .dtr = mfrc522_picc_acc_type_key_both
    },
    { /* mfrc522_picc_accb_001 (Value block) */
        .read = mfrc522_picc_acc_type_key_both,
        .write = mfrc522_picc_acc_type_never,
        .increment = mfrc522_picc_acc_type_never,
        .dtr = mfrc522_picc_acc_type_key_both
    },
    { /* mfrc522_picc_accb_010 (Read/write block) */
        .read = mfrc522_picc_acc_type_key_both,
        .write = mfrc522_picc_acc_type_never,
        .increment = mfrc522_picc_acc_type_never,
        .dtr = mfrc522_picc_acc_type_never
    },
    { /* mfrc522_picc_accb_011 (Read/write block) */
        .read = mfrc522_picc_acc_type_key_b,
        .write = mfrc522_picc_acc_type_key_b,
        .increment = mfrc522_picc_acc_type_never,
        .dtr = mfrc522_picc_acc_type_never
    },
    { /* mfrc522_picc_accb_100 (Read/write block) */
        .read = mfrc522_picc_acc_type_key_both,
        .write = mfrc522_picc_acc_type_key_b,
        .increment = mfrc522_picc_acc_type_never,
        .dtr = mfrc522_picc_acc_type_never
    },
    { /* mfrc522_picc_accb_101 (Read/write block) */
        .read = mfrc522_picc_acc_type_key_b,
        .write = mfrc522_picc_acc_type_never,
        .increment = mfrc522_picc_acc_type_never,
        .dtr = mfrc522_picc_acc_type_never
    },
    { /* mfrc522_picc_accb_110 (Value block) */
        .read = mfrc522_picc_acc_type_key_both,
        .write = mfrc522_picc_acc_type_key_b,
        .increment = mfrc522_picc_acc_type_key_b,
        .dtr = mfrc522_picc_acc_type_key_both
    },
    { /* mfrc522_picc_accb_111 (Read/write block) */
        .read = mfrc522_picc_acc_type_never,
        .write = mfrc522_picc_acc_type_never,
        .increment = mfrc522_picc_acc_type_never,
        .dtr = mfrc522_picc_acc_type_never
    },
};

/* ------------------------------------------------------------ */
/* ----------------------- Public functions ------------------- */
/* ------------------------------------------------------------ */

/*
 * Memory access conditions:
 *
 * +------+---------------------------------------------------------------+
 * | Byte |                   Bit number within a field                   |
 * +------+-------+-------+-------+-------+-------+-------+-------+-------+
 * |      |   7   |   6   |   5   |   4   |   3   |   2   |   1   |   0   |
 * +------+-------+-------+-------+-------+-------+-------+-------+-------+
 * |   6  | ~C2_3 | ~C2_2 | ~C2_1 | ~C2_0 | ~C1_3 | ~C1_2 | ~C1_1 | ~C1_0 |
 * +------+-------+-------+-------+-------+-------+-------+-------+-------+
 * |   7  |  C1_3 |  C1_2 |  C1_1 |  C1_0 | ~C3_3 | ~C3_2 | ~C3_1 | ~C3_0 |
 * +------+-------+-------+-------+-------+-------+-------+-------+-------+
 * |   8  |  C3_3 |  C3_2 |  C3_1 |  C3_0 |  C2_3 |  C2_2 |  C2_1 |  C2_0 |
 * +------+-------+-------+-------+-------+-------+-------+-------+-------+
 * |   9  |                           User data                           |
 * +------+---------------------------------------------------------------+
 */
void
mfrc522_picc_encode_accb(const mfrc522_picc_accb* accb, u8* encoded, u8 user_data)
{
    if (UNLIKELY((NULL == accb) | (NULL == encoded))) {
        return;
    }

    u8 c1 = (accb[0] & 0x01) | ((accb[1] & 0x01) << 1) | ((accb[2] & 0x01) << 2) | ((accb[3] & 0x01) << 3);
    u8 c2 = ((accb[0] & 0x02) >> 1) | (accb[1] & 0x02) | ((accb[2] & 0x02) << 1) | ((accb[3] & 0x02) << 2);
    u8 c3 = ((accb[0] & 0x04) >> 2) | ((accb[1] & 0x04) >> 1) | (accb[2] & 0x04) | ((accb[3] & 0x04) << 1);

    encoded[0] = (~c1 & 0x0F) | ((~c2 & 0x0F) << 4);
    encoded[1] = (~c3 & 0x0F) | (c1 << 4);
    encoded[2] = c2 | (c3 << 4);
    encoded[3] = user_data;
}

bool
mfrc522_picc_get_trailer_accb(const mfrc522_picc_trailer_acc* acc_cond, mfrc522_picc_accb* out)
{
    if (UNLIKELY((NULL == acc_cond) || (NULL == out))) {
        return false;
    }

    for (size i = 0; i < SIZE_ARRAY(trailer_acc_lut); ++i) {
        if (!memcmp(acc_cond, &trailer_acc_lut[i], sizeof(mfrc522_picc_trailer_acc))) {
            *out = (mfrc522_picc_accb)i;
            return true;
        }
    }
    return false;
}

bool
mfrc522_picc_get_block_accb(const mfrc522_picc_block_acc* acc_cond, mfrc522_picc_accb* out)
{
    if (UNLIKELY((NULL == acc_cond) || (NULL == out))) {
        return false;
    }

    for (size i = 0; i < SIZE_ARRAY(block_acc_lut); ++i) {
        if (!memcmp(acc_cond, &block_acc_lut[i], sizeof(mfrc522_picc_block_acc))) {
            *out = (mfrc522_picc_accb)i;
            return true;
        }
    }
    return false;
}

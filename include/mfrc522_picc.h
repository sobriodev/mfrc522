#ifndef MFRC522_MFRC522_PICC_H
#define MFRC522_MFRC522_PICC_H

#include "type.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ------------------------------------------------------------ */
/* ---------------------------- Macros ------------------------ */
/* ------------------------------------------------------------ */

/* Invalid ATQA response */
#define MFRC522_PICC_ATQA_INV 0xFFFF

/* ------------------------------------------------------------ */
/* -------------------------- Data types ---------------------- */
/* ------------------------------------------------------------ */

/**
 * List of PICC sectors.
 * The 1024 × 8 bit EEPROM memory is organized in 16 sectors of 4 blocks. One block contains 16 bytes.
 */
typedef enum mfrc522_picc_sector_
{
    mfrc522_picc_sector0 = 0, /**< Sector 0 */
    mfrc522_picc_sector1, /**< Sector 1 */
    mfrc522_picc_sector2, /**< Sector 2 */
    mfrc522_picc_sector3, /**< Sector 3 */
    mfrc522_picc_sector4, /**< Sector 4 */
    mfrc522_picc_sector5, /**< Sector 5 */
    mfrc522_picc_sector6, /**< Sector 6 */
    mfrc522_picc_sector7, /**< Sector 7 */
    mfrc522_picc_sector8, /**< Sector 8 */
    mfrc522_picc_sector9, /**< Sector 9 */
    mfrc522_picc_sector10, /**< Sector 10 */
    mfrc522_picc_sector11, /**< Sector 11 */
    mfrc522_picc_sector12, /**< Sector 12 */
    mfrc522_picc_sector13, /**< Sector 13 */
    mfrc522_picc_sector14, /**< Sector 14 */
    mfrc522_picc_sector15 /**< Sector 15 */
} mfrc522_picc_sector;

/**
 * List of PICC blocks.
 * The 1024 × 8 bit EEPROM memory is organized in 16 sectors of 4 blocks. One block contains 16 bytes.
 */
typedef enum mfrc522_picc_block_
{
    mfrc522_picc_block0 = 0, /**< Block 0 */
    mfrc522_picc_block1, /**< Block 1 */
    mfrc522_picc_block2, /**< Block 2 */
    mfrc522_picc_block3 /**< Block 3 */
} mfrc522_picc_block;

/**
 * Memory access conditions
 */
typedef enum mfrc522_picc_accb_
{
    mfrc522_picc_accb_000 = 0x00,
    mfrc522_picc_accb_001 = 0x01,
    mfrc522_picc_accb_010 = 0x02,
    mfrc522_picc_accb_011 = 0x03,
    mfrc522_picc_accb_100 = 0x04,
    mfrc522_picc_accb_101 = 0x05,
    mfrc522_picc_accb_110 = 0x06,
    mfrc522_picc_accb_111 = 0x07
} mfrc522_picc_accb;

/**
 * Memory access types
 */
typedef enum mfrc522_picc_acc_type_
{
    mfrc522_picc_acc_type_key_a, /**< Access with key A only */
    mfrc522_picc_acc_type_key_b, /**< Access with key B only */
    mfrc522_picc_acc_type_key_both, /**< Access with both keys */
    mfrc522_picc_acc_type_never, /**< Access never */
} mfrc522_picc_acc_type;

/**
 * Access conditions for the sector trailer
 */
typedef struct mfrc522_picc_trailer_acc_
{
    mfrc522_picc_acc_type key_a_write; /**< Key used to write to 'Key A' field */
    mfrc522_picc_acc_type access_bits_read; /**< Key used to read from 'Access Bits' field */
    mfrc522_picc_acc_type access_bits_write; /**< Key used to write to 'Access Bits' field */
    mfrc522_picc_acc_type key_b_read; /**< Key used to read from 'Key B' field */
    mfrc522_picc_acc_type key_b_write; /**< Key used to write to 'Key B' field */
} mfrc522_picc_trailer_acc;

/**
 * Access conditions for the block
 */
typedef struct mfrc522_picc_block_acc_
{
    mfrc522_picc_acc_type read; /**< Key used to read from a block */
    mfrc522_picc_acc_type write; /**< Key used to write to a block */
    mfrc522_picc_acc_type increment; /**< Key used to increment value in a block */
    mfrc522_picc_acc_type dtr; /**< Key used to decrement, transfer or restore value in a block */
} mfrc522_picc_block_acc;

/**
 * Available PICC commands
 */
typedef enum mfrc522_picc_cmd_
{
    mfrc522_picc_cmd_reqa = 0x26, /**< Request (7-bit) */
    mfrc522_picc_cmd_wupa = 0x52, /**< Wake-up (7-bit) */
    mfrc522_picc_cmd_anticoll_cl1 = 0x9320, /**< Anticollision CL1 */
    mfrc522_picc_cmd_select_cl1 = 0x9370, /**< Select CL1 */
    mfrc522_picc_cmd_anticoll_cl2 = 0x9520, /**< Anticollision CL2 */
    mfrc522_picc_cmd_select_cl2 = 0x9570, /**< Select CL2 */
    mfrc522_picc_cmd_halt = 0x5000, /**< Halt */
    mfrc522_picc_cmd_auth_key_a = 0x60, /**< Authenticate with Key A */
    mfrc522_picc_cmd_auth_key_b = 0x61, /** Authenticate with Key B */
    mfrc522_picc_cmd_pers_uid = 0x40, /**< Personalize UID usage */
    mfrc522_picc_cmd_set_mod_type = 0x43, /**< SET_MOD_TYPE */
    mfrc522_picc_cmd_read = 0x30, /**< MIFARE read */
    mfrc522_picc_cmd_write = 0xA0, /**< MIFARE write */
    mfrc522_picc_cmd_decrement = 0xC0, /**< MIFARE decrement */
    mfrc522_picc_cmd_increment = 0xC1, /**< MIFARE increment */
    mfrc522_picc_cmd_restore = 0xC2, /**< MIFARE restore */
    mfrc522_picc_cmd_transfer = 0xB0 /**< MIFARE transfer */
} mfrc522_picc_cmd;

/**
 * Function type to verify if ATQA response meets requirements.
 *
 * @param atqa ATQA response received from a PICC.
 * @return True if response is valid, false otherwise.
 */
typedef bool (*mfrc522_picc_atqa_verify_fn)(u16 atqa);

/* ------------------------------------------------------------ */
/* ----------------------- Public functions ------------------- */
/* ------------------------------------------------------------ */

/**
 * Encode access bits.
 *
 * The function converts access bits for each sector into vector of bytes that might be sent directly to a PCD.
 * It expects that input vector contains four elements (each of them keeping access bits for the respective section):
 *  - accb[0] = access bits for section 0
 *  - accb[1] = access bits for section 1
 *  - accb[2] = access bits for section 2
 *  - accb[3] = access bits for section 3 (trailer)
 *  Furthermore, output buffer has to be large enough to store four output bytes.
 *  MIFARE Classic standard allows to keep one byte of user data inside the 'access bits' field.
 *
 *  The function does nothing, when either input or output buffer is NULL.
 *
 * @param accb : Pointer to an input buffer
 * @param encoded : Pointer to a buffer, where calculated output is stored
 * @param user_data : User data byte
 */
void mfrc522_picc_encode_accb(const mfrc522_picc_accb* accb, u8* encoded, u8 user_data);

/**
 * Calculate access bits for sector trailer block.
 *
 * Note, that this function shall be used for sector trailer block only (block 3).
 * In case when access bits for blocks 0, 1 or 2 should be calculated, use function 'mfrc522_picc_get_block_accb()'.
 *
 * The function returns false, when either 'acc_cond' or 'out' argument is NULL.
 *
 * @param acc_cond Pointer to a structure describing access conditions
 * @param out Pointer to a buffer where the result is stored
 * @return True on success, false on failure
 */
bool mfrc522_picc_get_trailer_accb(const mfrc522_picc_trailer_acc* acc_cond, mfrc522_picc_accb* out);

/**
 * Calculate access bits for sector block.
 *
 * Note, that this function shall be used for blocks 0, 1 or 2 only.
 * In case when access bits for sector trailer should be calculated, use function 'mfrc522_picc_get_trailer_accb()'.
 *
 * The function returns false, when either 'acc_cond' or 'out' argument is NULL.
 *
 * @param acc_cond Pointer to a structure describing access conditions
 * @param out Pointer to a buffer where the result is stored
 * @return True on success, false on failure
 */
bool mfrc522_picc_get_block_accb(const mfrc522_picc_block_acc* acc_cond, mfrc522_picc_accb* out);

/**
 * Get transport (default) access bits configuration for section trailer block.
 *
 * @return Transport configuration
 */
static inline mfrc522_picc_accb mfrc522_picc_get_trailer_transport_accb(void)
{
    return mfrc522_picc_accb_001;
}

/**
 * Get transport (default) access bits configuration for blocks 0, 1 and 2
 *
 * @return Transport configuration
 */
static inline mfrc522_picc_accb mfrc522_picc_get_block_transport_accb(void)
{
    return mfrc522_picc_accb_000;
}

#ifdef __cplusplus
}
#endif

#endif //MFRC522_MFRC522_PICC_H

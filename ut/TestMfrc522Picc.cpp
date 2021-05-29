#include <gtest/gtest.h>
#include "mfrc522_picc.h"

/* ------------------------------------------------------------ */
/* ------------------------ Test cases ------------------------ */
/* ------------------------------------------------------------ */

TEST(TestMfrc522Picc, mfrc522_picc_encode_accb__NullCases)
{
    mfrc522_picc_accb accb[4] = {mfrc522_picc_accb_000};
    u8 buffer[4] = {0};

    /* Case 1 */
    mfrc522_picc_encode_accb(nullptr, &buffer[0], 0xAB);
    for (const auto& b : buffer) {
        ASSERT_EQ(0x00, b);
    }

    /* Case 2 */
    mfrc522_picc_encode_accb(&accb[0], nullptr, 0xAB);
    for (const auto& b : buffer) {
        ASSERT_EQ(0x00, b);
    }
}

TEST(TestMfrc522Picc, mfrc522_picc_encode_accb__CheckIfUserDataIsSet)
{
    mfrc522_picc_accb accb[4] = {mfrc522_picc_accb_000};
    u8 buffer[4] = {0};
    u8 user_data = 0xFC;

    mfrc522_picc_encode_accb(&accb[0], &buffer[0], user_data);
    ASSERT_EQ(user_data, buffer[3]);
}

TEST(TestMfrc522Picc, mfrc522_picc_encode_accb__Encode__Case1__Success)
{
    mfrc522_picc_accb accb[4];
    accb[0] = mfrc522_picc_accb_000;
    accb[1] = mfrc522_picc_accb_001;
    accb[2] = mfrc522_picc_accb_010;
    accb[3] = mfrc522_picc_accb_100;

    u8 buffer[4];
    mfrc522_picc_encode_accb(&accb[0], &buffer[0], 0xAA);

    ASSERT_EQ(0xBD, buffer[0]);
    ASSERT_EQ(0x27, buffer[1]);
    ASSERT_EQ(0x84, buffer[2]);
    ASSERT_EQ(0xAA, buffer[3]); /* User data */
}

TEST(TestMfrc522Picc, mfrc522_picc_encode_accb__Encode__Case2__Success)
{
    mfrc522_picc_accb accb[4];
    accb[0] = mfrc522_picc_accb_111;
    accb[1] = mfrc522_picc_accb_111;
    accb[2] = mfrc522_picc_accb_111;
    accb[3] = mfrc522_picc_accb_111;

    u8 buffer[4];
    mfrc522_picc_encode_accb(&accb[0], &buffer[0], 0xCA);

    ASSERT_EQ(0x00, buffer[0]);
    ASSERT_EQ(0xF0, buffer[1]);
    ASSERT_EQ(0xFF, buffer[2]);
    ASSERT_EQ(0xCA, buffer[3]); /* User data */
}

TEST(TestMfrc522Picc, mfrc522_picc_encode_accb__Encode__Case3__Success)
{
    mfrc522_picc_accb accb[4];
    accb[0] = mfrc522_picc_accb_011;
    accb[1] = mfrc522_picc_accb_110;
    accb[2] = mfrc522_picc_accb_111;
    accb[3] = mfrc522_picc_accb_001;

    u8 buffer[4];
    mfrc522_picc_encode_accb(&accb[0], &buffer[0], 0xBD);

    ASSERT_EQ(0x82, buffer[0]);
    ASSERT_EQ(0xD9, buffer[1]);
    ASSERT_EQ(0x67, buffer[2]);
    ASSERT_EQ(0xBD, buffer[3]); /* User data */
}

TEST(TestMfrc522Picc, mfrc522_picc_get_trailer_accb__NullCases)
{
    mfrc522_picc_trailer_acc trailerAccessCond;
    mfrc522_picc_accb output;

    /* Case 1 */
    auto status = mfrc522_picc_get_trailer_accb(nullptr, &output);
    ASSERT_EQ(false, status);

    /* Case 2 */
    status = mfrc522_picc_get_trailer_accb(&trailerAccessCond, nullptr);
    ASSERT_EQ(false, status);
}

TEST(TestMfrc522Picc, mfrc522_picc_get_trailer_accb__UnknownConfiguration__FalseReturned)
{
    /* Use only 'never' access rights - this is not valid configuration */
    mfrc522_picc_trailer_acc trailerAccCond;
    trailerAccCond.key_a_write = mfrc522_picc_acc_type_never;
    trailerAccCond.access_bits_read = mfrc522_picc_acc_type_never;
    trailerAccCond.access_bits_write = mfrc522_picc_acc_type_never;
    trailerAccCond.key_b_read = mfrc522_picc_acc_type_never;
    trailerAccCond.key_b_write = mfrc522_picc_acc_type_never;

    mfrc522_picc_accb output;
    auto status = mfrc522_picc_get_trailer_accb(&trailerAccCond, &output);
    ASSERT_EQ(false, status);
}

TEST(TestMfrc522Picc, mfrc522_picc_get_trailer_accb__ConfigurationFound__TrueReturnedAndOutputSet)
{
    mfrc522_picc_trailer_acc trailerAccCond;
    trailerAccCond.key_a_write = mfrc522_picc_acc_type_key_b;
    trailerAccCond.access_bits_read = mfrc522_picc_acc_type_key_both;
    trailerAccCond.access_bits_write = mfrc522_picc_acc_type_key_b;
    trailerAccCond.key_b_read = mfrc522_picc_acc_type_never;
    trailerAccCond.key_b_write = mfrc522_picc_acc_type_key_b;

    mfrc522_picc_accb output;
    auto status = mfrc522_picc_get_trailer_accb(&trailerAccCond, &output);
    ASSERT_EQ(true, status);
    ASSERT_EQ(mfrc522_picc_accb_011, output);
}

TEST(TestMfrc522Picc, mfrc522_picc_get_block_accb__NullCases)
{
    mfrc522_picc_block_acc blockAccessCond;
    mfrc522_picc_accb output;

    /* Case 1 */
    auto status = mfrc522_picc_get_block_accb(nullptr, &output);
    ASSERT_EQ(false, status);

    /* Case 2 */
    status = mfrc522_picc_get_block_accb(&blockAccessCond, nullptr);
    ASSERT_EQ(false, status);
}

TEST(TestMfrc522Picc, mfrc522_picc_get_block_accb__UnknownConfiguration__FalseReturned)
{
    /* Use only Key A - this is invalid configuration */
    mfrc522_picc_block_acc blockAccCond;
    blockAccCond.increment = mfrc522_picc_acc_type_key_a;
    blockAccCond.read = mfrc522_picc_acc_type_key_a;
    blockAccCond.write = mfrc522_picc_acc_type_key_a;
    blockAccCond.dtr = mfrc522_picc_acc_type_key_a;

    mfrc522_picc_accb output;
    auto status = mfrc522_picc_get_block_accb(&blockAccCond, &output);
    ASSERT_EQ(false, status);
}

TEST(TestMfrc522Picc, mfrc522_picc_get_block_accb__ValidConfiguration__TrueReturnedAndOutputSet)
{
    /* Use only Key A - this is invalid configuration */
    mfrc522_picc_block_acc blockAccCond;
    blockAccCond.read = mfrc522_picc_acc_type_key_both;
    blockAccCond.write = mfrc522_picc_acc_type_key_b;
    blockAccCond.increment = mfrc522_picc_acc_type_never;
    blockAccCond.dtr = mfrc522_picc_acc_type_never;

    mfrc522_picc_accb output;
    auto status = mfrc522_picc_get_block_accb(&blockAccCond, &output);
    ASSERT_EQ(true, status);
    ASSERT_EQ(mfrc522_picc_accb_100, output);
}

TEST(TestMfrc522Picc, mfrc522_picc_get_trailer_transport_accb__001ConfReturned)
{
    auto accb = mfrc522_picc_get_trailer_transport_accb();
    ASSERT_EQ(mfrc522_picc_accb_001, accb);
}

TEST(TestMfrc522Picc, mfrc522_picc_get_block_transport_accb__000ConfReturned)
{
    auto accb = mfrc522_picc_get_block_transport_accb();
    ASSERT_EQ(mfrc522_picc_accb_000, accb);
}
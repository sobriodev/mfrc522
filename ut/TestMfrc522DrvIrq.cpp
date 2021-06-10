#include "mfrc522_drv.h"
#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "Mockable.h" /* Provide mocks */

/* Required by C Mock library */
DEFINE_MOCKABLE(mfrc522_ll_status, mfrc522_ll_send, (u8, size, u8*));
DEFINE_MOCKABLE(mfrc522_ll_status, mfrc522_ll_recv, (u8, u8*));

using namespace testing;

/* ------------------------------------------------------------ */
/* ------------------------ Test cases ------------------------ */
/* ------------------------------------------------------------ */

TEST(TestMfrc522DrvIrq, mfrc522_drv_irq_init__NullCases)
{
    mfrc522_drv_conf conf;
    mfrc522_drv_irq_conf irqConf;

    auto status = mfrc522_drv_irq_init(nullptr, &irqConf);
    ASSERT_EQ(mfrc522_drv_status_nullptr, status);

    status = mfrc522_drv_irq_init(&conf, nullptr);
    ASSERT_EQ(mfrc522_drv_status_nullptr, status);
}

TEST(TestMfrc522DrvIrq, mfrc522_drv_irq_init__Success)
{
    auto conf = initDevice();

    /* Populate IRQ config fields */
    mfrc522_drv_irq_conf irqConf;
    irqConf.irq_push_pull = true;
    irqConf.irq_signal_inv = false;

    /* Set expectations */
    MOCK(mfrc522_ll_send);
    InSequence seq;
    /* Expect that all IRQs are cleared */
    MOCK_CALL(mfrc522_ll_send, mfrc522_reg_com_irq, 1, _).WillOnce(Return(mfrc522_ll_status_ok));
    MOCK_CALL(mfrc522_ll_send, mfrc522_reg_div_irq, 1, _).WillOnce(Return(mfrc522_ll_status_ok));
    /* Expect calls that configure IRQ registers */
    MOCK_CALL(mfrc522_ll_send, mfrc522_reg_com_irq_en, 1, _).WillOnce(Return(mfrc522_ll_status_ok));
    MOCK_CALL(mfrc522_ll_send, mfrc522_reg_div_irq_en, 1, _).WillOnce(Return(mfrc522_ll_status_ok));

    auto status = mfrc522_drv_irq_init(&conf, &irqConf);
    ASSERT_EQ(mfrc522_drv_status_ok, status);
}

TEST(TestMfrc522DrvIrq, mfrc522_drv_irq_clr__NullCases)
{
    auto status = mfrc522_drv_irq_clr(nullptr, mfrc522_reg_irq_all);
    ASSERT_EQ(mfrc522_drv_status_nullptr, status);
}

TEST(TestMfrc522DrvIrq, mfrc522_drv_irq_clr__ClearAllIrqs__BothRegistersAreUpdated)
{
    auto conf = initDevice();

    /* Expect both registers to be updated */
    u8 word1_irq_bits = 0x7F;
    u8 word2_irq_bits = 0x14;

    /* Set expectations */
    MOCK(mfrc522_ll_send);
    InSequence s;
    MOCK_CALL(mfrc522_ll_send, mfrc522_reg_com_irq, 1, Pointee(word1_irq_bits))
        .WillOnce(Return(mfrc522_ll_status_ok));
    MOCK_CALL(mfrc522_ll_send, mfrc522_reg_div_irq, 1, Pointee(word2_irq_bits))
        .WillOnce(Return(mfrc522_ll_status_ok));

    auto status = mfrc522_drv_irq_clr(&conf, mfrc522_reg_irq_all);
    ASSERT_EQ(mfrc522_drv_status_ok, status);
}

TEST(TestMfrc522DrvIrq, mfrc522_drv_irq_clr__ClearIrqFromComReg__Success)
{
    auto conf = initDevice();

    /* Set expectations */
    MOCK(mfrc522_ll_send);
    /* Expect ComIrqReg to be updated */
    MOCK_CALL(mfrc522_ll_send, mfrc522_reg_com_irq, 1, Pointee(1 << mfrc522_reg_irq_hi_alert))
        .WillOnce(Return(mfrc522_ll_status_ok));

    auto status = mfrc522_drv_irq_clr(&conf, mfrc522_reg_irq_hi_alert);
    ASSERT_EQ(mfrc522_drv_status_ok, status);
}

TEST(TestMfrc522DrvIrq, mfrc522_drv_irq_clr__ClearIrqFromDivReg__Success)
{
    auto conf = initDevice();

    u8 crc_irq_bit = 2;

    /* Expect DivIrqReg to be updated */
    MOCK(mfrc522_ll_send);
    MOCK_CALL(mfrc522_ll_send, mfrc522_reg_div_irq, 1, Pointee(1 << crc_irq_bit))
        .WillOnce(Return(mfrc522_ll_status_ok));

    auto status = mfrc522_drv_irq_clr(&conf, mfrc522_reg_irq_crc);
    ASSERT_EQ(mfrc522_drv_status_ok, status);
}

TEST(TestMfrc522DrvIrq, mfrc522_drv_irq_en__NullCases)
{
    auto status = mfrc522_drv_irq_en(nullptr, mfrc522_reg_irq_idle, true);
    ASSERT_EQ(mfrc522_drv_status_nullptr, status);
}

TEST(TestMfrc522DrvIrq, mfrc522_drv_irq_en__IrqAllPassedAsAnArgument__ErrorIsReturned)
{
    auto conf = initDevice();

    auto status = mfrc522_drv_irq_en(&conf, mfrc522_reg_irq_all, true);
    ASSERT_EQ(mfrc522_drv_status_nok, status);
}

TEST(TestMfrc522DrvIrq, mfrc522_drv_irq_en__EnableIrqInComReg__Success)
{
    auto conf = initDevice();

    /* Pretend that some IRQs are enabled */
    u8 actualIrqs = 0x41;

    MOCK(mfrc522_ll_recv);
    MOCK(mfrc522_ll_send);
    InSequence s;
    MOCK_CALL(mfrc522_ll_recv, mfrc522_reg_com_irq_en, _)
        .WillOnce(DoAll(SetArgPointee<1>(actualIrqs), Return(mfrc522_ll_status_ok)));
    MOCK_CALL(mfrc522_ll_send, mfrc522_reg_com_irq_en, 1,
                        Pointee((u8)(actualIrqs | (1 << mfrc522_reg_irq_idle))))
        .WillOnce(Return(mfrc522_ll_status_ok));

    auto status = mfrc522_drv_irq_en(&conf, mfrc522_reg_irq_idle, true);
    ASSERT_EQ(mfrc522_drv_status_ok, status);
}

TEST(TestMfrc522DrvIrq, mfrc522_drv_irq_en__DisableIrqInComReg__Success)
{
    auto conf = initDevice();

    /* Pretend that IRQ was enabled, thus 0x00 should be written back */
    u8 actualIrqs = 1 << mfrc522_reg_irq_idle;

    MOCK(mfrc522_ll_recv);
    MOCK(mfrc522_ll_send);
    InSequence s;
    MOCK_CALL(mfrc522_ll_recv, mfrc522_reg_com_irq_en, _)
        .WillOnce(DoAll(SetArgPointee<1>(actualIrqs), Return(mfrc522_ll_status_ok)));
    MOCK_CALL(mfrc522_ll_send, mfrc522_reg_com_irq_en, 1, Pointee(0x00))
        .WillOnce(Return(mfrc522_ll_status_ok));

    auto status = mfrc522_drv_irq_en(&conf, mfrc522_reg_irq_idle, false);
    ASSERT_EQ(mfrc522_drv_status_ok, status);
}

TEST(TestMfrc522DrvIrq, mfrc522_drv_irq_en__EnableIrqInDivReg__Success)
{
    auto conf = initDevice();

    MOCK(mfrc522_ll_recv);
    MOCK(mfrc522_ll_send);
    InSequence s;
    MOCK_CALL(mfrc522_ll_recv, mfrc522_reg_div_irq_en, _)
        .WillOnce(DoAll(SetArgPointee<1>(0x00), Return(mfrc522_ll_status_ok)));
    MOCK_CALL(mfrc522_ll_send, mfrc522_reg_div_irq_en, 1,
                        Pointee(1 << (mfrc522_reg_irq_crc & ~MFRC522_REG_IRQ_DIV)))
        .WillOnce(Return(mfrc522_ll_status_ok));

    auto status = mfrc522_drv_irq_en(&conf, mfrc522_reg_irq_crc, true);
    ASSERT_EQ(mfrc522_drv_status_ok, status);
}

TEST(TestMfrc522DrvIrq, mfrc522_irq_states__NullCases)
{
    auto device = initDevice();
    u16 out;

    auto status = mfrc522_irq_states(&device, nullptr);
    ASSERT_EQ(mfrc522_drv_status_nullptr, status);

    status = mfrc522_irq_states(nullptr, &out);
    ASSERT_EQ(mfrc522_drv_status_nullptr, status);
}

TEST(TestMfrc522DrvIrq, mfrc522_irq_states__TypicalCase__Success)
{
    auto device = initDevice();
    u16 states = 0x00;

    /* Set expectations */
    MOCK(mfrc522_ll_recv);
    MOCK_CALL(mfrc522_ll_recv, mfrc522_reg_com_irq, _)
        .WillOnce(DoAll(SetArgPointee<1>(0xCD), Return(mfrc522_ll_status_ok)));
    MOCK_CALL(mfrc522_ll_recv, mfrc522_reg_div_irq, _)
            .WillOnce(DoAll(SetArgPointee<1>(0xAB), Return(mfrc522_ll_status_ok)));

    auto status = mfrc522_irq_states(&device, &states);
    ASSERT_EQ(mfrc522_drv_status_ok, status);
    ASSERT_EQ(0xABCD, states);
}

TEST(TestMfrc522DrvIrq, mfrc522_drv_irq_pending__IrqAllFlagWasPassed__False)
{
    u16 states = 0xFFFF;
    auto status = mfrc522_drv_irq_pending(states, mfrc522_reg_irq_all);
    ASSERT_FALSE(status);
}

TEST(TestMfrc522DrvIrq, mfrc522_drv_irq_pending__IrqPendingInComReg__True)
{
    u16 states = 1 << mfrc522_reg_irq_timer;
    auto status = mfrc522_drv_irq_pending(states, mfrc522_reg_irq_timer);
    ASSERT_TRUE(status);
}

TEST(TestMfrc522DrvIrq, mfrc522_drv_irq_pending__IrqPendingInDivReg__True)
{
    u16 states = 1 << 10; /* CRC IRQ is pending */
    auto status = mfrc522_drv_irq_pending(states, mfrc522_reg_irq_crc);
    ASSERT_TRUE(status);
}

TEST(TestMfrc522DrvIrq, mfrc522_drv_irq_pending__IrqNotPending__False)
{
    u16 states = 0x0000;
    auto status = mfrc522_drv_irq_pending(states, mfrc522_reg_irq_crc);
    ASSERT_FALSE(status);
    status = mfrc522_drv_irq_pending(states, mfrc522_reg_irq_idle);
    ASSERT_FALSE(status);
}

TEST(TestMfrc522DrvIrq, mfrc522_drv_irq_pending__IrqsPendingInBothRegisters__DoubleTrueReturned)
{
    u16 states = 0xFFFF;
    auto status = mfrc522_drv_irq_pending(states, mfrc522_reg_irq_mfin_act);
    ASSERT_TRUE(status);
    status = mfrc522_drv_irq_pending(states, mfrc522_reg_irq_hi_alert);
    ASSERT_TRUE(status);
}
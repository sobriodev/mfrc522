#include "TestCommon.h"
#include "mfrc522_drv.h"
#include "Mockable.h" /* Provide mocks */

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
    INSTALL_HOOK(mfrc522_drv_init, mfrc522_drv_init__STUB);
    mfrc522_drv_conf conf;
    mfrc522_drv_init(&conf);

    /* Populate IRQ config fields */
    mfrc522_drv_irq_conf irqConf;
    irqConf.irq_push_pull = true;
    irqConf.irq_signal_inv = false;

    /* Set expectations */
    INSTALL_MOCK(mfrc522_ll_send);
    InSequence seq;
    /* Expect that all IRQs are cleared */
    CUTIE_EXPECT_CALL(mfrc522_ll_send, mfrc522_reg_com_irq, 1, _).WillOnce(Return(mfrc522_ll_status_ok));
    CUTIE_EXPECT_CALL(mfrc522_ll_send, mfrc522_reg_div_irq, 1, _).WillOnce(Return(mfrc522_ll_status_ok));
    /* Expect calls that configure IRQ registers */
    CUTIE_EXPECT_CALL(mfrc522_ll_send, mfrc522_reg_com_irq_en, 1, _).WillOnce(Return(mfrc522_ll_status_ok));
    CUTIE_EXPECT_CALL(mfrc522_ll_send, mfrc522_reg_div_irq_en, 1, _).WillOnce(Return(mfrc522_ll_status_ok));

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
    INSTALL_HOOK(mfrc522_drv_init, mfrc522_drv_init__STUB);
    mfrc522_drv_conf conf;
    mfrc522_drv_init(&conf);

    /* Expect both registers to be updated */
    u8 word1_irq_bits = 0x7F;
    u8 word2_irq_bits = 0x14;

    /* Set expectations */
    INSTALL_MOCK(mfrc522_ll_send);
    InSequence s;
    CUTIE_EXPECT_CALL(mfrc522_ll_send, mfrc522_reg_com_irq, 1, Pointee(word1_irq_bits))
        .WillOnce(Return(mfrc522_ll_status_ok));
    CUTIE_EXPECT_CALL(mfrc522_ll_send, mfrc522_reg_div_irq, 1, Pointee(word2_irq_bits))
        .WillOnce(Return(mfrc522_ll_status_ok));

    auto status = mfrc522_drv_irq_clr(&conf, mfrc522_reg_irq_all);
    ASSERT_EQ(mfrc522_drv_status_ok, status);
}

TEST(TestMfrc522DrvIrq, mfrc522_drv_irq_clr__ClearIrqFromComReg__Success)
{
    INSTALL_HOOK(mfrc522_drv_init, mfrc522_drv_init__STUB);
    mfrc522_drv_conf conf;
    mfrc522_drv_init(&conf);

    /* Set expectations */
    InSequence s;
    /* Expect ComIrqReg to be updated */
    INSTALL_EXPECT_CALL(mfrc522_ll_send, mfrc522_reg_com_irq, 1, Pointee(1 << mfrc522_reg_irq_hi_alert))
        .WillOnce(Return(mfrc522_ll_status_ok));

    auto status = mfrc522_drv_irq_clr(&conf, mfrc522_reg_irq_hi_alert);
    ASSERT_EQ(mfrc522_drv_status_ok, status);
}

TEST(TestMfrc522DrvIrq, mfrc522_drv_irq_clr__ClearIrqFromDivReg__Success)
{
    INSTALL_HOOK(mfrc522_drv_init, mfrc522_drv_init__STUB);
    mfrc522_drv_conf conf;
    mfrc522_drv_init(&conf);

    u8 crc_irq_bit = 2;

    /* Expect DivIrqReg to be updated */
    InSequence s;
    INSTALL_EXPECT_CALL(mfrc522_ll_send, mfrc522_reg_div_irq, 1, Pointee(1 << crc_irq_bit))
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
    INSTALL_HOOK(mfrc522_drv_init, mfrc522_drv_init__STUB);
    mfrc522_drv_conf conf;
    mfrc522_drv_init(&conf);

    auto status = mfrc522_drv_irq_en(&conf, mfrc522_reg_irq_all, true);
    ASSERT_EQ(mfrc522_drv_status_nok, status);
}

TEST(TestMfrc522DrvIrq, mfrc522_drv_irq_en__EnableIrqInComReg__Success)
{
    INSTALL_HOOK(mfrc522_drv_init, mfrc522_drv_init__STUB);
    mfrc522_drv_conf conf;
    mfrc522_drv_init(&conf);

    /* Pretend that some IRQs are enabled */
    u8 actualIrqs = 0x41;

    InSequence s;
    INSTALL_EXPECT_CALL(mfrc522_ll_recv, mfrc522_reg_com_irq_en, _)
        .WillOnce(DoAll(SetArgPointee<1>(actualIrqs), Return(mfrc522_ll_status_ok)));
    INSTALL_EXPECT_CALL(mfrc522_ll_send, mfrc522_reg_com_irq_en, 1,
                        Pointee((u8)(actualIrqs | (1 << mfrc522_reg_irq_idle))))
        .WillOnce(Return(mfrc522_ll_status_ok));

    auto status = mfrc522_drv_irq_en(&conf, mfrc522_reg_irq_idle, true);
    ASSERT_EQ(mfrc522_drv_status_ok, status);
}

TEST(TestMfrc522DrvIrq, mfrc522_drv_irq_en__DisableIrqInComReg__Success)
{
    INSTALL_HOOK(mfrc522_drv_init, mfrc522_drv_init__STUB);
    mfrc522_drv_conf conf;
    mfrc522_drv_init(&conf);

    /* Pretend that IRQ was enabled, thus 0x00 should be written back */
    u8 actualIrqs = 1 << mfrc522_reg_irq_idle;

    InSequence s;
    INSTALL_EXPECT_CALL(mfrc522_ll_recv, mfrc522_reg_com_irq_en, _)
        .WillOnce(DoAll(SetArgPointee<1>(actualIrqs), Return(mfrc522_ll_status_ok)));
    INSTALL_EXPECT_CALL(mfrc522_ll_send, mfrc522_reg_com_irq_en, 1, Pointee(0x00))
        .WillOnce(Return(mfrc522_ll_status_ok));

    auto status = mfrc522_drv_irq_en(&conf, mfrc522_reg_irq_idle, false);
    ASSERT_EQ(mfrc522_drv_status_ok, status);
}

TEST(TestMfrc522DrvIrq, mfrc522_drv_irq_en__EnableIrqInDivReg__Success)
{
    INSTALL_HOOK(mfrc522_drv_init, mfrc522_drv_init__STUB);
    mfrc522_drv_conf conf;
    mfrc522_drv_init(&conf);

    InSequence s;
    INSTALL_EXPECT_CALL(mfrc522_ll_recv, mfrc522_reg_div_irq_en, _)
        .WillOnce(DoAll(SetArgPointee<1>(0x00), Return(mfrc522_ll_status_ok)));
    INSTALL_EXPECT_CALL(mfrc522_ll_send, mfrc522_reg_div_irq_en, 1,
                        Pointee(1 << (mfrc522_reg_irq_crc & ~MFRC522_REG_IRQ_DIV)))
        .WillOnce(Return(mfrc522_ll_status_ok));

    auto status = mfrc522_drv_irq_en(&conf, mfrc522_reg_irq_crc, true);
    ASSERT_EQ(mfrc522_drv_status_ok, status);
}

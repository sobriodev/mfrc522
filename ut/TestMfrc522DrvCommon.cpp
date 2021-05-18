#include "TestCommon.h"
#include "mfrc522_drv.h"
#include "Mockable.h" /* Provide mocks */

using namespace testing;

/* ------------------------------------------------------------ */
/* ------------------------ Test cases ------------------------ */
/* ------------------------------------------------------------ */
//
TEST(TestMfrc522DrvCommon, mfrc522_drv_init__NullCases)
{
    auto status = mfrc522_drv_init(nullptr);
    EXPECT_EQ(mfrc522_drv_status_nullptr, status);
}

TEST(TestMfrc522DrvCommon, mfrc522_drv_init__LlReceiveError__LlErrorIsGenerated)
{
    u8 payload = 0xAB; /* Assume that low-level call failed, thus trash value was returned */
    auto llStatus = mfrc522_ll_status_recv_err;

    /* Set expectations */
    InSequence seq;
    INSTALL_EXPECT_CALL(mfrc522_ll_recv, mfrc522_reg_version, _)
        .WillOnce(DoAll(SetArgPointee<1>(payload), Return(llStatus)));

    mfrc522_drv_conf conf;
    auto status = mfrc522_drv_init(&conf);
    ASSERT_EQ(mfrc522_drv_status_ll_err, status);
    /* In a case low-level API returns an error 'chip_version' field should be equal to MFRC522_REG_VERSION_INVALID */
    ASSERT_EQ(MFRC522_REG_VERSION_INVALID, conf.chip_version);
}

TEST(TestMfrc522DrvCommon, mfrc522_drv_init__DeviceNotSupported__Failure)
{
    /* Set expectations */
    InSequence seq;
    INSTALL_EXPECT_CALL(mfrc522_ll_recv, mfrc522_reg_version, _)
        .WillOnce(DoAll(SetArgPointee<1>(0xAA), Return(mfrc522_ll_status_ok)));

    mfrc522_drv_conf conf;
    auto status = mfrc522_drv_init(&conf);
    ASSERT_EQ(mfrc522_drv_status_dev_err, status);
}

TEST(TestMfrc522DrvCommon, mfrc522_drv_init__DeviceFound__Success)
{
    INSTALL_HOOK(mfrc522_drv_init, mfrc522_drv_init__STUB);
    mfrc522_drv_conf conf;
    mfrc522_drv_init(&conf);

    /* Check if 'chip_version' field was set */
    ASSERT_EQ(0x9B, conf.chip_version);
}

TEST(TestMfrc522DrvCommon, mfrc522_drv_write__NullCases)
{
    auto status = mfrc522_drv_write_byte(nullptr, mfrc522_reg_fifo_data_reg, 0xAB);
    ASSERT_EQ(mfrc522_ll_status_send_err, status);
}

TEST(TestMfrc522DrvCommon, mfrc522_drv_read__NullCases)
{
    mfrc522_drv_conf conf;
    u8 pl;

    auto status = mfrc522_drv_read(&conf, mfrc522_reg_fifo_data_reg, nullptr);
    ASSERT_EQ(mfrc522_ll_status_recv_err, status);

    status = mfrc522_drv_read(nullptr, mfrc522_reg_fifo_data_reg, &pl);
    ASSERT_EQ(mfrc522_ll_status_recv_err, status);
}

TEST(TestMfrc522DrvCommon, mfrc522_drv_read_until__NullCases)
{
    mfrc522_drv_conf conf;
    mfrc522_drv_read_until_conf ruConf;

    auto status = mfrc522_drv_read_until(&conf, nullptr);
    ASSERT_EQ(mfrc522_drv_status_nullptr, status);

    status = mfrc522_drv_read_until(nullptr, &ruConf);
    ASSERT_EQ(mfrc522_drv_status_nullptr, status);
}

TEST(TestMfrc522DrvCommon, mfrc522_drv_read_until__InfinityFlagEnabled__Success)
{
    INSTALL_HOOK(mfrc522_drv_init, mfrc522_drv_init__STUB);
    mfrc522_drv_conf conf;
    mfrc522_drv_init(&conf);

    mfrc522_drv_read_until_conf ruConf;
    ruConf.addr = mfrc522_reg_fifo_data_reg;
    ruConf.exp_payload = 0x0E;
    ruConf.mask = 0x0F;
    ruConf.retry_cnt = MFRC522_DRV_RETRY_CNT_INF;
    ruConf.delay = 1;

    /* Populate fake responses */
    INSTALL_MOCK(mfrc522_ll_recv);
    InSequence s;
    CUTIE_EXPECT_CALL(mfrc522_ll_recv, ruConf.addr, _).Times(100)
        .WillRepeatedly(DoAll(SetArgPointee<1>(0x00), Return(mfrc522_ll_status_ok)));
    CUTIE_EXPECT_CALL(mfrc522_ll_recv, ruConf.addr, _)
        .WillOnce(DoAll(SetArgPointee<1>(ruConf.exp_payload), Return(mfrc522_ll_status_ok)));

    auto status = mfrc522_drv_read_until(&conf, &ruConf);
    ASSERT_EQ(mfrc522_drv_status_ok, status);
    ASSERT_EQ(ruConf.exp_payload, ruConf.payload & ruConf.mask);
}

TEST(TestMfrc522DrvCommon, mfrc522_drv_read_until__AnswerOnFirstTry__Success)
{
    INSTALL_HOOK(mfrc522_drv_init, mfrc522_drv_init__STUB);
    mfrc522_drv_conf conf;
    mfrc522_drv_init(&conf);

    mfrc522_drv_read_until_conf ruConf;
    ruConf.addr = mfrc522_reg_fifo_data_reg;
    ruConf.exp_payload = 0xF0;
    ruConf.mask = 0xF0;
    ruConf.retry_cnt = 0;
    ruConf.delay = 1;

    /* Set expectations */
    InSequence seq;
    INSTALL_EXPECT_CALL(mfrc522_ll_recv, ruConf.addr, _)
        .WillOnce(DoAll(SetArgPointee<1>(ruConf.exp_payload), Return(mfrc522_ll_status_ok)));

    auto status = mfrc522_drv_read_until(&conf, &ruConf);
    ASSERT_EQ(mfrc522_drv_status_ok, status);
    ASSERT_EQ(ruConf.exp_payload, ruConf.payload & ruConf.mask);
}

TEST(TestMfrc522DrvCommon, mfrc522_drv_read_until__AnswerOnThirdTime__Success)
{
    INSTALL_HOOK(mfrc522_drv_init, mfrc522_drv_init__STUB);
    mfrc522_drv_conf conf;
    mfrc522_drv_init(&conf);

    mfrc522_drv_read_until_conf ruConf;
    ruConf.addr = mfrc522_reg_fifo_data_reg;
    ruConf.exp_payload = 0x80;
    ruConf.mask = 0xC0;
    ruConf.retry_cnt = 2; /* One try + two retries */
    ruConf.delay = 1;

    /* Set expectations */
    InSequence seq;
    INSTALL_EXPECT_CALL(mfrc522_ll_recv, ruConf.addr, _)
        .WillOnce(DoAll(SetArgPointee<1>(0x00), Return(mfrc522_ll_status_ok)))
        .WillOnce(DoAll(SetArgPointee<1>(0x40), Return(mfrc522_ll_status_ok)))
        .WillOnce(DoAll(SetArgPointee<1>(0x8F), Return(mfrc522_ll_status_ok)));

    auto status = mfrc522_drv_read_until(&conf, &ruConf);
    ASSERT_EQ(mfrc522_drv_status_ok, status);
    ASSERT_EQ(ruConf.exp_payload, ruConf.payload & ruConf.mask);
}

TEST(TestMfrc522DrvCommon, mfrc522_drv_read_until__NoAnswerAfterAllTries__Failure)
{
    INSTALL_HOOK(mfrc522_drv_init, mfrc522_drv_init__STUB);
    mfrc522_drv_conf conf;
    mfrc522_drv_init(&conf);

    mfrc522_drv_read_until_conf ruConf;
    ruConf.addr = mfrc522_reg_fifo_data_reg;
    ruConf.exp_payload = 0xAA;
    ruConf.mask = 0xFF;
    ruConf.retry_cnt = 10;
    ruConf.delay = 1;

    /* Set expectations */
    InSequence seq;
    INSTALL_EXPECT_CALL(mfrc522_ll_recv, ruConf.addr, _).Times(static_cast<i32>(ruConf.retry_cnt + 1))
        .WillRepeatedly(DoAll(SetArgPointee<1>(0x00), Return(mfrc522_ll_status_ok)));

    auto status = mfrc522_drv_read_until(&conf, &ruConf);
    ASSERT_EQ(mfrc522_drv_status_dev_rtr_err, status);

    /* Check recent payload */
    ASSERT_EQ(0x00, ruConf.payload);
}

TEST(TestMfrc522DrvCommon, mfrc522_drv_soft_reset__NullCases)
{
    auto status = mfrc522_drv_soft_reset(nullptr);
    ASSERT_EQ(mfrc522_drv_status_nullptr, status);
}

TEST(TestMfrc522DrvCommon, mfrc522_drv_soft_reset__TypicalCase__Success)
{
    INSTALL_HOOK(mfrc522_drv_init, mfrc522_drv_init__STUB);
    mfrc522_drv_conf conf;
    mfrc522_drv_init(&conf);

    /* Create expectations */
    InSequence seq;
    INSTALL_EXPECT_CALL(mfrc522_ll_recv, mfrc522_reg_command, _)
        /* Phase 0: Some command is running */
        .WillOnce(DoAll(SetArgPointee<1>(mfrc522_reg_cmd_mem), Return(mfrc522_ll_status_ok)))
        /* Phase 1: Idle command is active */
        .WillOnce(DoAll(SetArgPointee<1>(mfrc522_reg_cmd_idle), Return(mfrc522_ll_status_ok)))
        /* Phase 2: Perform soft reset */
        .WillOnce(DoAll(SetArgPointee<1>(mfrc522_reg_cmd_soft_reset), Return(mfrc522_ll_status_ok)))
        /* Phase 3: Soft reset has not been done yet */
        .WillOnce(DoAll(SetArgPointee<1>(mfrc522_reg_cmd_soft_reset), Return(mfrc522_ll_status_ok)))
        /* Phase 4: Idle command is active back */
        .WillOnce(DoAll(SetArgPointee<1>(mfrc522_reg_cmd_idle), Return(mfrc522_ll_status_ok)));

    auto status = mfrc522_drv_soft_reset(&conf);
    ASSERT_EQ(mfrc522_drv_status_ok, status);
}

TEST(TestMfrc522DrvCommon, mfrc522_drv_soft_reset__IdleCommandNotReached__Failure)
{
    INSTALL_HOOK(mfrc522_drv_init, mfrc522_drv_init__STUB);
    mfrc522_drv_conf conf;
    mfrc522_drv_init(&conf);

    /* Create expectations */
    InSequence seq;
    INSTALL_EXPECT_CALL(mfrc522_ll_recv, mfrc522_reg_command, _).Times(static_cast<i32>(MFRC522_DRV_DEF_RETRY_CNT + 1))
        .WillRepeatedly(DoAll(SetArgPointee<1>(mfrc522_reg_cmd_mem), Return(mfrc522_ll_status_ok)));

    auto status = mfrc522_drv_soft_reset(&conf);
    ASSERT_EQ(mfrc522_drv_status_dev_rtr_err, status);
}

TEST(TestMfrc522DrvCommon, mfrc522_drv_write_masked__NullCases)
{
    auto status = mfrc522_drv_write_masked(nullptr, mfrc522_reg_demod, 0x0F, 0x0F, 0);
    ASSERT_EQ(mfrc522_ll_status_send_err, status);
}

TEST(TestMfrc522DrvCommon, mfrc522_drv_write_masked__MaskedWritePerformed)
{
    INSTALL_HOOK(mfrc522_drv_init, mfrc522_drv_init__STUB);
    mfrc522_drv_conf conf;
    mfrc522_drv_init(&conf);

    const u8 msk = 0x0F;
    const u8 pos = 0;

    /* Set expectations */
    InSequence seq;
    INSTALL_EXPECT_CALL(mfrc522_ll_recv, mfrc522_reg_gs_n, _)
        .WillOnce(DoAll(SetArgPointee<1>(0x55), Return(mfrc522_ll_status_ok)));
    INSTALL_EXPECT_CALL(mfrc522_ll_send, mfrc522_reg_gs_n, 1, Pointee(0x5D))
        .WillOnce(Return(mfrc522_ll_status_ok));

    auto status = mfrc522_drv_write_masked(&conf, mfrc522_reg_gs_n, 13, msk, pos);
    ASSERT_EQ(mfrc522_ll_status_ok, status);
}

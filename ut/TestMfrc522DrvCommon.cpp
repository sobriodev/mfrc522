#include "mfrc522_drv.h"
#include "mfrc522_conf.h"
#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "TestCommon.h"
#include "Mockable.h" /* Provide mocks */

/* Required by C Mock library */
DEFINE_MOCKABLE(mfrc522_ll_status, mfrc522_ll_send, (u8, size, u8*));
DEFINE_MOCKABLE(mfrc522_ll_status, mfrc522_ll_recv, (u8, u8*));
DEFINE_MOCKABLE(void, mfrc522_ll_delay, (u32));
DEFINE_MOCKABLE(mfrc522_drv_status, mfrc522_drv_init, (mfrc522_drv_conf*));
DEFINE_MOCKABLE(mfrc522_drv_status, mfrc522_drv_soft_reset, (const mfrc522_drv_conf*));
DEFINE_MOCKABLE(mfrc522_drv_status, mfrc522_drv_invoke_cmd, (const mfrc522_drv_conf*, mfrc522_reg_cmd));
DEFINE_MOCKABLE(mfrc522_drv_status, mfrc522_irq_states, (const mfrc522_drv_conf*, u16*));
DEFINE_MOCKABLE(mfrc522_drv_status, mfrc522_drv_transceive, (const mfrc522_drv_conf*, mfrc522_drv_transceive_conf*));

using namespace testing;

/* ------------------------------------------------------------ */
/* ---------------------- Custom matchers --------------------- */
/* ------------------------------------------------------------ */

/* Matcher to check if configuration struct for transceive function contains valid fields */
MATCHER_P(TransceiveStructInputMatcher, expected, "Transceive struct input matcher")
{
    return (arg->tx_data_sz == expected->tx_data_sz) && /* Compare TX sizes */
           (arg->rx_data_sz == expected->rx_data_sz) &&  /* Compare RX sizes */
           !memcmp(arg->tx_data, expected->tx_data, expected->tx_data_sz); /* Compare TX data */
}

/* ------------------------------------------------------------ */
/* ----------------------- Custom Actions --------------------- */
/* ------------------------------------------------------------ */

/* Action to set RX bytes inside mocked transceive function */
ACTION_P(TransceiveAction, expected)
{
    /* Copy RX data from expected structure */
    memcpy(arg1->rx_data, expected->rx_data, expected->rx_data_sz);
}

/* ------------------------------------------------------------ */
/* ------------------------ Test cases ------------------------ */
/* ------------------------------------------------------------ */

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
    MOCK(mfrc522_ll_recv);
    MOCK_CALL(mfrc522_ll_recv, mfrc522_reg_version, _)
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
    MOCK(mfrc522_ll_recv);
    MOCK_CALL( mfrc522_ll_recv, mfrc522_reg_version, _)
        .WillOnce(DoAll(SetArgPointee<1>(0xAA), Return(mfrc522_ll_status_ok)));

    mfrc522_drv_conf conf;
    auto status = mfrc522_drv_init(&conf);
    ASSERT_EQ(mfrc522_drv_status_dev_err, status);
}

TEST(TestMfrc522DrvCommon, mfrc522_drv_init__DeviceFound__Success)
{
    /* Set expectations */
    MOCK(mfrc522_ll_recv);
    MOCK_CALL(mfrc522_ll_recv, mfrc522_reg_version, _)
        .WillOnce(DoAll(SetArgPointee<1>(MFRC522_CONF_CHIP_TYPE), Return(mfrc522_ll_status_ok)));

    mfrc522_drv_conf conf;
    auto status = mfrc522_drv_init(&conf);
    ASSERT_EQ(mfrc522_drv_status_ok, status);

    /* Check if 'chip_version' field was set */
    ASSERT_EQ(MFRC522_CONF_CHIP_TYPE, conf.chip_version);
}

TEST(TestMfrc522DrvCommon, mfrc522_drv_write__NullCases)
{
    u8 payload = 0xAB;
    auto status = mfrc522_drv_write(nullptr, mfrc522_reg_fifo_data, 1, &payload);
    ASSERT_EQ(mfrc522_ll_status_send_err, status);
}

TEST(TestMfrc522DrvCommon, mfrc522_drv_read__NullCases)
{
    mfrc522_drv_conf conf;
    u8 pl;

    auto status = mfrc522_drv_read(&conf, mfrc522_reg_fifo_data, nullptr);
    ASSERT_EQ(mfrc522_ll_status_recv_err, status);

    status = mfrc522_drv_read(nullptr, mfrc522_reg_fifo_data, &pl);
    ASSERT_EQ(mfrc522_ll_status_recv_err, status);
}

TEST(TestMfrc522DrvCommon, mfrc522_drv_fifo_store__ValidLowLevelCallIsMade)
{
    auto device = initDevice();

    /* Expect that low-level call is made */
    MOCK(mfrc522_ll_send);
    MOCK_CALL(mfrc522_ll_send, mfrc522_reg_fifo_data, 1, Pointee(0xBC)).WillOnce(Return(mfrc522_ll_status_ok));

    auto status = mfrc522_drv_fifo_store(&device, 0xBC);
    ASSERT_EQ(mfrc522_drv_status_ok, status);
}

TEST(TestMfrc522DrvCommon, mfrc522_drv_fifo_store_mul__ValidLowLevelCallIsMade)
{
    auto dev = initDevice();

    /* Bytes to be stored */
    u8 bytes[] = {0xAA, 0xBB, 0xCC};

    /* Expect that low-level call is made */
    MOCK(mfrc522_ll_send);
    MOCK_CALL(mfrc522_ll_send, mfrc522_reg_fifo_data, SIZE_ARRAY(bytes), &bytes[0])
        .WillOnce(Return(mfrc522_ll_status_ok));

    auto status = mfrc522_drv_fifo_store_mul(&dev, &bytes[0], SIZE_ARRAY(bytes));
    ASSERT_EQ(mfrc522_drv_status_ok, status);
}

TEST(TestMfrc522DrvCommon, mfrc522_drv_fifo_read__ValidLowLevelCallIsMade)
{
    auto dev = initDevice();
    u8 buffer;

    /* Expect that low-level call is made */
    MOCK(mfrc522_ll_recv);
    MOCK_CALL(mfrc522_ll_recv, mfrc522_reg_fifo_data, &buffer)
        .WillOnce(DoAll(SetArgPointee<1>(0xFA), Return(mfrc522_ll_status_ok)));

    auto status = mfrc522_drv_fifo_read(&dev, &buffer);
    ASSERT_EQ(mfrc522_drv_status_ok, status);
    ASSERT_EQ(0xFA, buffer);
}

TEST(TestMfrc522DrvCommon, mfrc522_drv_fifo_flush__ValidLowLevelCallIsMade)
{
    auto dev = initDevice();

    /* Expect that low-level call is made */
    MOCK(mfrc522_ll_send);
    MOCK_CALL(mfrc522_ll_send, mfrc522_reg_fifo_level, 1, Pointee(0x80)).WillOnce(Return(mfrc522_ll_status_ok));

    auto status = mfrc522_drv_fifo_flush(&dev);
    ASSERT_EQ(mfrc522_drv_status_ok, status);
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
    auto conf = initDevice();

    mfrc522_drv_read_until_conf ruConf;
    ruConf.addr = mfrc522_reg_fifo_data;
    ruConf.exp_payload = 0x0E;
    ruConf.mask = 0x0F;
    ruConf.retry_cnt = MFRC522_DRV_RETRY_CNT_INF;
    ruConf.delay = 1;

    /* Populate fake responses */
    MOCK(mfrc522_ll_recv);
    InSequence s;
    MOCK_CALL(mfrc522_ll_recv, ruConf.addr, _).Times(100)
        .WillRepeatedly(DoAll(SetArgPointee<1>(0x00), Return(mfrc522_ll_status_ok)));
    MOCK_CALL(mfrc522_ll_recv, ruConf.addr, _)
        .WillOnce(DoAll(SetArgPointee<1>(ruConf.exp_payload), Return(mfrc522_ll_status_ok)));

    auto status = mfrc522_drv_read_until(&conf, &ruConf);
    ASSERT_EQ(mfrc522_drv_status_ok, status);
    ASSERT_EQ(ruConf.exp_payload, ruConf.payload & ruConf.mask);
}

TEST(TestMfrc522DrvCommon, mfrc522_drv_read_until__AnswerOnFirstTry__Success)
{
    auto conf = initDevice();

    mfrc522_drv_read_until_conf ruConf;
    ruConf.addr = mfrc522_reg_fifo_data;
    ruConf.exp_payload = 0xF0;
    ruConf.mask = 0xF0;
    ruConf.retry_cnt = 0;
    ruConf.delay = 1;

    /* Set expectations */
    MOCK(mfrc522_ll_recv);
    MOCK_CALL(mfrc522_ll_recv, ruConf.addr, _)
        .WillOnce(DoAll(SetArgPointee<1>(ruConf.exp_payload), Return(mfrc522_ll_status_ok)));

    auto status = mfrc522_drv_read_until(&conf, &ruConf);
    ASSERT_EQ(mfrc522_drv_status_ok, status);
    ASSERT_EQ(ruConf.exp_payload, ruConf.payload & ruConf.mask);
}

TEST(TestMfrc522DrvCommon, mfrc522_drv_read_until__AnswerOnThirdTime__Success)
{
    auto conf = initDevice();

    mfrc522_drv_read_until_conf ruConf;
    ruConf.addr = mfrc522_reg_fifo_data;
    ruConf.exp_payload = 0x80;
    ruConf.mask = 0xC0;
    ruConf.retry_cnt = 2; /* One try + two retries */
    ruConf.delay = 1;

    /* Set expectations */
    MOCK(mfrc522_ll_recv);
    InSequence seq;
    MOCK_CALL(mfrc522_ll_recv, ruConf.addr, _)
        .WillOnce(DoAll(SetArgPointee<1>(0x00), Return(mfrc522_ll_status_ok)))
        .WillOnce(DoAll(SetArgPointee<1>(0x40), Return(mfrc522_ll_status_ok)))
        .WillOnce(DoAll(SetArgPointee<1>(0x8F), Return(mfrc522_ll_status_ok)));

    auto status = mfrc522_drv_read_until(&conf, &ruConf);
    ASSERT_EQ(mfrc522_drv_status_ok, status);
    ASSERT_EQ(ruConf.exp_payload, ruConf.payload & ruConf.mask);
}

TEST(TestMfrc522DrvCommon, mfrc522_drv_read_until__NoAnswerAfterAllTries__Failure)
{
    auto conf = initDevice();

    mfrc522_drv_read_until_conf ruConf;
    ruConf.addr = mfrc522_reg_fifo_data;
    ruConf.exp_payload = 0xAA;
    ruConf.mask = 0xFF;
    ruConf.retry_cnt = 10;
    ruConf.delay = 1;

    /* Set expectations */
    MOCK(mfrc522_ll_recv);
    MOCK_CALL(mfrc522_ll_recv, ruConf.addr, _).Times(static_cast<i32>(ruConf.retry_cnt + 1))
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
    auto conf = initDevice();

    /* Create expectations */
    MOCK(mfrc522_ll_recv);
    InSequence seq;
    MOCK_CALL(mfrc522_ll_recv, mfrc522_reg_command, NotNull())
        /* Phase 1: Perform soft reset */
        .WillOnce(DoAll(SetArgPointee<1>(mfrc522_reg_cmd_soft_reset), Return(mfrc522_ll_status_ok)))
        /* Phase 2: Soft reset has not been done yet */
        .WillOnce(DoAll(SetArgPointee<1>(mfrc522_reg_cmd_soft_reset), Return(mfrc522_ll_status_ok)))
        /* Phase 3: Idle command is active back */
        .WillOnce(DoAll(SetArgPointee<1>(mfrc522_reg_cmd_idle), Return(mfrc522_ll_status_ok)));

    auto status = mfrc522_drv_soft_reset(&conf);
    ASSERT_EQ(mfrc522_drv_status_ok, status);
}

TEST(TestMfrc522DrvCommon, mfrc522_drv_soft_reset__IdleCommandNotReached__Failure)
{
    auto conf = initDevice();

    /* Create expectations */
    MOCK(mfrc522_ll_recv);
    MOCK_CALL(mfrc522_ll_recv, mfrc522_reg_command, _).Times(MFRC522_DRV_DEF_RETRY_CNT + 1)
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
    auto conf = initDevice();

    const u8 msk = 0x0F;
    const u8 pos = 0;

    /* Set expectations */
    MOCK(mfrc522_ll_recv);
    MOCK(mfrc522_ll_send);
    InSequence seq;//
    MOCK_CALL(mfrc522_ll_recv, mfrc522_reg_gs_n, _)
        .WillOnce(DoAll(SetArgPointee<1>(0x55), Return(mfrc522_ll_status_ok)));
    MOCK_CALL(mfrc522_ll_send, mfrc522_reg_gs_n, 1, Pointee(0x5D))
        .WillOnce(Return(mfrc522_ll_status_ok));

    auto status = mfrc522_drv_write_masked(&conf, mfrc522_reg_gs_n, 13, msk, pos);
    ASSERT_EQ(mfrc522_ll_status_ok, status);
}

TEST(TestMfrc522DrvCommon, mfrc522_drv_read_masked__NullCases)
{
    auto device = initDevice();
    u8 out;

    auto status = mfrc522_drv_read_masked(&device, mfrc522_reg_demod, nullptr, 0xFF, 0);
    ASSERT_EQ(mfrc522_drv_status_nullptr, status);
    status = mfrc522_drv_read_masked(nullptr, mfrc522_reg_fifo_data, &out, 0xFF, 0);
    ASSERT_EQ(mfrc522_drv_status_nullptr, status);
}

TEST(TestMfrc522DrvCommon, mfrc522_drv_read_masked__MaskedReadPerformed)
{
    auto device = initDevice();
    u8 out;

    /* Set expectations */
    MOCK(mfrc522_ll_recv);
    MOCK_CALL(mfrc522_ll_recv, mfrc522_reg_fifo_data, NotNull())
        .WillOnce(DoAll(SetArgPointee<1>(0xCF), Return(mfrc522_ll_status_ok)));

    /* Call FUT */
    auto status = mfrc522_drv_read_masked(&device, mfrc522_reg_fifo_data, &out, 0x0F, 4);
    ASSERT_EQ(mfrc522_ll_status_ok, status);
    ASSERT_EQ(0x0C, out);
}

TEST(TestMfrc522DrvCommon, mfrc522_drv_self_test__NullCases)
{
    auto status = mfrc522_drv_self_test(nullptr);
    ASSERT_EQ(mfrc522_drv_status_nullptr, status);
}

TEST(TestMfrc522DrvCommon, mfrc522_drv_self_test__TypicalCase__Success)
{
    auto conf = initDevice();

    /* Self test procedure */
    MOCK(mfrc522_ll_recv);
    MOCK(mfrc522_ll_send);
    MOCK(mfrc522_drv_soft_reset);
    MOCK(mfrc522_drv_invoke_cmd);
    IGNORE_REDUNDANT_LL_RECV_CALLS();
    InSequence s;
    /* 1. Soft reset */
    MOCK_CALL(mfrc522_drv_soft_reset, &conf)
        .WillOnce(Return(mfrc522_drv_status_ok));
    /* 2. Clear the internal buffer */
    MOCK_CALL(mfrc522_ll_send, mfrc522_reg_fifo_data, 1, Pointee(0x00))
        .WillOnce(Return(mfrc522_ll_status_ok));
    MOCK_CALL(mfrc522_drv_invoke_cmd, &conf, mfrc522_reg_cmd_mem)
        .WillOnce(Return(mfrc522_drv_status_ok));
    /* 3. Enable the self test by writing 09h to the AutoTestReg register */
    MOCK_CALL(mfrc522_ll_send, mfrc522_reg_auto_test, 1, _)
        .WillOnce(Return(mfrc522_ll_status_ok));
    /* 4. Write 00h to the FIFO buffer */
    MOCK_CALL(mfrc522_ll_send, mfrc522_reg_fifo_data, 1, Pointee(0x00))
        .WillOnce(Return(mfrc522_ll_status_ok));
    /* 5. Start the self test with the CalcCRC command */
    MOCK_CALL(mfrc522_drv_invoke_cmd, &conf, mfrc522_reg_cmd_crc)
        .WillOnce(Return(mfrc522_drv_status_ok));
    /* 6 Wait until FIFO buffer contains 64 bytes */
    MOCK_CALL(mfrc522_ll_recv, mfrc522_reg_fifo_level, _)
        .WillOnce(DoAll(SetArgPointee<1>(64), Return(mfrc522_ll_status_ok)));
    /* Read FIFO contents */
    u8 ret[MFRC522_DRV_SELF_TEST_FIFO_SZ] = {MFRC522_CONF_SELF_TEST_FIFO_OUT};
    for (const auto& r : ret) {
        MOCK_CALL(mfrc522_ll_recv, mfrc522_reg_fifo_data, _)
            .WillOnce(DoAll(SetArgPointee<1>(r), Return(mfrc522_ll_status_ok)));
    }

    auto status = mfrc522_drv_self_test(&conf);
    ASSERT_EQ(mfrc522_drv_status_ok, status);

    /* Check if internal buffer was filled */
    ASSERT_EQ(0, memcmp(ret, conf.self_test_out, MFRC522_DRV_SELF_TEST_FIFO_SZ));
}

TEST(TestMfrc522DrvCommon, mfrc522_drv_self_test__FifoContentsDoNotMatchExpectedOnes__Failure)
{
    auto conf = initDevice();

    /* Self test procedure */
    MOCK(mfrc522_ll_recv);
    MOCK(mfrc522_ll_send);
    MOCK(mfrc522_drv_soft_reset);
    MOCK(mfrc522_drv_invoke_cmd);
    IGNORE_REDUNDANT_LL_RECV_CALLS();
    InSequence s;
    /* 1. Soft reset */
    MOCK_CALL(mfrc522_drv_soft_reset, &conf)
            .WillOnce(Return(mfrc522_drv_status_ok));
    /* 2. Clear the internal buffer */
    MOCK_CALL(mfrc522_ll_send, mfrc522_reg_fifo_data, 1, Pointee(0x00))
            .WillOnce(Return(mfrc522_ll_status_ok));
    MOCK_CALL(mfrc522_drv_invoke_cmd, &conf, mfrc522_reg_cmd_mem)
            .WillOnce(Return(mfrc522_drv_status_ok));
    /* 3. Enable the self test by writing 09h to the AutoTestReg register */
    MOCK_CALL(mfrc522_ll_send, mfrc522_reg_auto_test, 1, _)
            .WillOnce(Return(mfrc522_ll_status_ok));
    /* 4. Write 00h to the FIFO buffer */
    MOCK_CALL(mfrc522_ll_send, mfrc522_reg_fifo_data, 1, Pointee(0x00))
            .WillOnce(Return(mfrc522_ll_status_ok));
    /* 5. Start the self test with the CalcCRC command */
    MOCK_CALL(mfrc522_drv_invoke_cmd, &conf, mfrc522_reg_cmd_crc)
            .WillOnce(Return(mfrc522_drv_status_ok));
    /* 6 Wait until FIFO buffer contains 64 bytes */
    MOCK_CALL(mfrc522_ll_recv, mfrc522_reg_fifo_level, _)
            .WillOnce(DoAll(SetArgPointee<1>(64), Return(mfrc522_ll_status_ok)));
    /* Read FIFO contents */
    u8 ret[MFRC522_DRV_SELF_TEST_FIFO_SZ] = {0x00}; /* Only zeros returned */
    for (const auto& r : ret) {
        MOCK_CALL(mfrc522_ll_recv, mfrc522_reg_fifo_data, _)
                .WillOnce(DoAll(SetArgPointee<1>(r), Return(mfrc522_ll_status_ok)));
    }

    auto status = mfrc522_drv_self_test(&conf);
    ASSERT_EQ(mfrc522_drv_status_self_test_err, status);
}

TEST(TestMfrc522DrvCommon, mfrc522_drv_invoke_cmd__NullCases)
{
    auto status = mfrc522_drv_invoke_cmd(nullptr, mfrc522_reg_cmd_idle);
    ASSERT_EQ(mfrc522_drv_status_nullptr, status);
}

TEST(TestMfrc522DrvCommon, mfrc522_drv_invoke_cmd__InvokeCmdThatTerminates__LookIfIdleCmdIsActiveBack)
{
    auto conf = initDevice();

    /* Set expectations */
    MOCK(mfrc522_ll_recv);
    InSequence s;
    MOCK_CALL(mfrc522_ll_recv, mfrc522_reg_command, _)
        /* Called when new command needs to be invoked */
        .WillOnce(DoAll(SetArgPointee<1>(0x00), Return(mfrc522_ll_status_ok)))
        /* Idle command is active back */
        .WillOnce(DoAll(SetArgPointee<1>(mfrc522_reg_cmd_idle), Return(mfrc522_ll_status_ok)));

    auto status = mfrc522_drv_invoke_cmd(&conf, mfrc522_reg_cmd_mem);
    ASSERT_EQ(mfrc522_drv_status_ok, status);
}

TEST(TestMfrc522DrvCommon, mfrc522_drv_invoke_cmd__InvokeCmdThatDoesNotTerminateItself__WaitLoopDisabled)
{
    auto conf = initDevice();

    /* Set expectations */
    MOCK(mfrc522_ll_recv);
    MOCK_CALL(mfrc522_ll_recv, mfrc522_reg_command, _)
        /* Called when new command needs to be invoked */
        .WillOnce(DoAll(SetArgPointee<1>(0x00), Return(mfrc522_ll_status_ok)));

    auto status = mfrc522_drv_invoke_cmd(&conf, mfrc522_reg_cmd_transceive);
    ASSERT_EQ(mfrc522_drv_status_ok, status);
}

TEST(TestMfrc522DrvCommon, mfrc522_drv_crc_init__NullCases)
{
    auto device = initDevice();
    mfrc522_drv_crc_conf crc;

    auto status = mfrc522_drv_crc_init(&device, nullptr);
    ASSERT_EQ(mfrc522_drv_status_nullptr, status);
    status = mfrc522_drv_crc_init(nullptr, &crc);
    ASSERT_EQ(mfrc522_drv_status_nullptr, status);
}

TEST(TestMfrc522DrvCommon, mfrc522_drv_crc_init__TypicalCase__Success)
{
    auto device = initDevice();
    mfrc522_drv_crc_conf crcConfig;
    crcConfig.msb_first = true;
    crcConfig.preset = mfrc522_drv_crc_preset_6363;

    /* Set expectations */
    MOCK(mfrc522_ll_send);
    InSequence s;
    MOCK_CALL(mfrc522_ll_send, mfrc522_reg_mode, 1, Pointee(0x01)).WillOnce(Return(mfrc522_ll_status_ok));
    MOCK_CALL(mfrc522_ll_send, mfrc522_reg_mode, 1, Pointee(1 << 7)).WillOnce(Return(mfrc522_ll_status_ok));

    auto status = mfrc522_drv_crc_init(&device, &crcConfig);
    ASSERT_EQ(mfrc522_drv_status_ok, status);
}

TEST(TestMfrc522DrvCommon, mfrc522_drv_crc_compute__NullCases)
{
    auto device = initDevice();
    u16 buffer;

    auto status = mfrc522_drv_crc_compute(nullptr, &buffer);
    ASSERT_EQ(mfrc522_drv_status_nullptr, status);
    status = mfrc522_drv_crc_compute(&device, nullptr);
    ASSERT_EQ(mfrc522_drv_status_nullptr, status);
}

TEST(TestMfrc522DrvCommon, mfrc522_drv_crc_compute__TypicalCase__CrcComputed)
{
    auto device = initDevice();
    u8 crcLo = 0xAA;
    u8 crcHi = 0xBB;

    /* Set expectations */
    MOCK(mfrc522_ll_recv);
    MOCK(mfrc522_drv_invoke_cmd);
    InSequence s;
    /* Phase 1: CRC command should be invoked */
    MOCK_CALL(mfrc522_drv_invoke_cmd, &device, mfrc522_reg_cmd_crc).WillOnce(Return(mfrc522_drv_status_ok));
    /* Phase 2: CRC coprocessor has computed the value */
    MOCK_CALL(mfrc522_ll_recv, mfrc522_reg_status1, _)
        .WillOnce(DoAll(SetArgPointee<1>(1 << 5), Return(mfrc522_ll_status_ok)));
    /* Phase 3: Activate Idle command back */
    MOCK_CALL(mfrc522_drv_invoke_cmd, &device, mfrc522_reg_cmd_idle).WillOnce(Return(mfrc522_drv_status_ok));
    /* Phase 4: Computed value is read */
    MOCK_CALL(mfrc522_ll_recv, mfrc522_reg_crc_result_lsb, _)
            .WillOnce(DoAll(SetArgPointee<1>(crcLo), Return(mfrc522_ll_status_ok)));
    MOCK_CALL(mfrc522_ll_recv, mfrc522_reg_crc_result_msb, _)
            .WillOnce(DoAll(SetArgPointee<1>(crcHi), Return(mfrc522_ll_status_ok)));

    u16 out;
    auto status = mfrc522_drv_crc_compute(&device, &out);
    ASSERT_EQ(mfrc522_drv_status_ok, status);
    ASSERT_EQ(crcLo, (out & 0x00FF));
    ASSERT_EQ(crcHi, ((out & 0xFF00) >> 8));
}

TEST(TestMfrc522DrvCommon, mfrc522_drv_generate_rand__NullCases)
{
    auto device = initDevice();
    u8 buffer;
    size sz = 1;

    auto status = mfrc522_drv_generate_rand(&device, nullptr, sz);
    ASSERT_EQ(mfrc522_drv_status_nullptr, status);
    status = mfrc522_drv_generate_rand(nullptr, &buffer, sz);
    ASSERT_EQ(mfrc522_drv_status_nullptr, status);
}

TEST(TestMfrc522DrvCommon, mfrc522_drv_generate_rand__TypicalCase__Success) {
    auto device = initDevice();
    u8 randomByteIdx[MFRC522_DRV_RAND_BYTES] = {MFRC522_CONF_RAND_BYTE_IDX};
    u8 returnedBytes[MFRC522_DRV_RAND_TOTAL] = {0x00};
    for (const auto &i : randomByteIdx) {
        returnedBytes[i] = 0xCF;
    }

    /* Set expectations */
    MOCK(mfrc522_ll_recv);
    MOCK(mfrc522_drv_invoke_cmd);
    IGNORE_REDUNDANT_LL_RECV_CALLS();
    InSequence s;
    /* 'Rand' command is invoked */
    MOCK_CALL(mfrc522_drv_invoke_cmd, &device, mfrc522_reg_cmd_rand).WillOnce(Return(mfrc522_drv_status_ok));
    /* 'Mem' command is invoked */
    MOCK_CALL(mfrc522_drv_invoke_cmd, &device, mfrc522_reg_cmd_mem).WillOnce(Return(mfrc522_drv_status_ok));
    /* 25 bytes are read from the FIFO buffer */
    for (const auto& i: returnedBytes) {
        MOCK_CALL(mfrc522_ll_recv, mfrc522_reg_fifo_data, _)
            .WillOnce(DoAll(SetArgPointee<1>(i), Return(mfrc522_ll_status_ok)));
    }

    u8 out[10];
    auto status = mfrc522_drv_generate_rand(&device, &out[0], 10);
    ASSERT_EQ(mfrc522_drv_status_ok, status);
    for (const auto& i : out) {
        ASSERT_EQ(0xCF, i);
    }
}

TEST(TestMfrc522DrvCommon, mfrc522_drv_generate_rand__ZeroBytesRequested__OutputBufferRemainsTheSame)
{
    auto device = initDevice();

    u8 out = 0xAA;
    auto status = mfrc522_drv_generate_rand(&device, &out, 0);
    ASSERT_EQ(mfrc522_drv_status_ok, status);
    ASSERT_EQ(0xAA, out);
}

TEST(TestMfrc522DrvCommon, mfrc522_drv_generate_rand__MoreThanTenBytesRequested__TenBytesReturned)
{
    auto device = initDevice();
    u8 randomByteIdx[MFRC522_DRV_RAND_BYTES] = {MFRC522_CONF_RAND_BYTE_IDX};
    u8 returnedBytes[MFRC522_DRV_RAND_TOTAL] = {0x00};
    for (const auto &i : randomByteIdx) {
        returnedBytes[i] = 0xCF;
    }

    /* Set expectations */
    MOCK(mfrc522_ll_recv);
    MOCK(mfrc522_drv_invoke_cmd);
    IGNORE_REDUNDANT_LL_RECV_CALLS();
    InSequence s;
    /* 'Rand' command is invoked */
    MOCK_CALL(mfrc522_drv_invoke_cmd, &device, mfrc522_reg_cmd_rand).WillOnce(Return(mfrc522_drv_status_ok));
    /* 'Mem' command is invoked */
    MOCK_CALL(mfrc522_drv_invoke_cmd, &device, mfrc522_reg_cmd_mem).WillOnce(Return(mfrc522_drv_status_ok));
    /* 25 bytes are read from the FIFO buffer */
    for (const auto& i: returnedBytes) {
        MOCK_CALL(mfrc522_ll_recv, mfrc522_reg_fifo_data, _)
                .WillOnce(DoAll(SetArgPointee<1>(i), Return(mfrc522_ll_status_ok)));
    }

    u8 out[11] = {0x00};
    auto status = mfrc522_drv_generate_rand(&device, &out[0], 11); /* Request 11 bytes */
    ASSERT_EQ(mfrc522_drv_status_ok, status);
    for (size i = 0; i < 10; ++i) {
        ASSERT_EQ(0xCF, out[i]);
    }
    ASSERT_EQ(0x00, out[10]); /* Last byte shall be zeroed */
}

TEST(TestMfrc522DrvCommon, mfrc522_drv_check_error__NoErrors__CheckForAnyError__False)
{
    u8 errors = 0x00;
    auto status = mfrc522_drv_check_error(errors, mfrc522_reg_err_any);
    ASSERT_FALSE(status);
}

TEST(TestMfrc522DrvCommon, mfrc522_drv_check_error__TwoErrors__CheckForAnyError__False)
{
    u8 errors = 0x11;
    auto status = mfrc522_drv_check_error(errors, mfrc522_reg_err_any);
    ASSERT_TRUE(status);
}

TEST(TestMfrc522DrvCommon, mfrc522_drv_check_error__MiscCases)
{
    /* Case 1 - search for error that is not present */
    u8 errors = 1 << mfrc522_reg_err_coll;
    auto status = mfrc522_drv_check_error(errors, mfrc522_reg_err_crc);
    ASSERT_FALSE(status);

    /* Case 2 - search for error that is present */
    errors = 1 << mfrc522_reg_err_temp;
    status = mfrc522_drv_check_error(errors, mfrc522_reg_err_temp);
    ASSERT_TRUE(status);

    /* Case 3 - no errors active */
    errors = 0x00;
    status = mfrc522_drv_check_error(errors, mfrc522_reg_err_protocol);
    ASSERT_FALSE(status);
}

TEST(TestMfrc522DrvCommon, mfrc522_drv_ext_itf_init__NullCases)
{
    auto device = initDevice();
    mfrc522_drv_ext_itf_conf itfConf;

    auto status = mfrc522_drv_ext_itf_init(&device, nullptr);
    ASSERT_EQ(mfrc522_drv_status_nullptr, status);

    status = mfrc522_drv_ext_itf_init(nullptr, &itfConf);
    ASSERT_EQ(mfrc522_drv_status_nullptr, status);
}

TEST(TestMfrc522DrvCommon, mfrc522_drv_ext_itf_init__TypicalCase__Success)
{
    auto device = initDevice();
    mfrc522_drv_ext_itf_conf itfConf; /* Nothing to initialize here */

    /* Set expectations */
    MOCK(mfrc522_ll_send);
    MOCK_CALL(mfrc522_ll_send, mfrc522_reg_tx_ask, 1, _).WillOnce(Return(mfrc522_ll_status_ok));
    MOCK_CALL(mfrc522_ll_send, mfrc522_reg_tx_control, 1, _).Times(2).WillRepeatedly(Return(mfrc522_ll_status_ok));
    MOCK_CALL(mfrc522_ll_send, mfrc522_reg_command, 1, _).WillOnce(Return(mfrc522_ll_status_ok));

    auto status = mfrc522_drv_ext_itf_init(&device, &itfConf);
    ASSERT_EQ(mfrc522_drv_status_ok, status);
}

TEST(TestMfrc522DrvCommon, mfrc522_drv_transceive__NullCases)
{
    auto device = initDevice();
    mfrc522_drv_transceive_conf transceiveConf;

    auto status = mfrc522_drv_transceive(&device, nullptr);
    ASSERT_EQ(mfrc522_drv_status_nullptr, status);
    status = mfrc522_drv_transceive(nullptr, &transceiveConf);
    ASSERT_EQ(mfrc522_drv_status_nullptr, status);
}

TEST(TestMfrc522DrvCommon, mfrc522_drv_transceive__NoIrqAfterAllRetries__TimeoutError)
{
    auto device = initDevice();

    /* Populate configuration struct */
    u8 tx = 0xCF;
    u8 rx = 0xAA;
    mfrc522_drv_transceive_conf transceiveConf;
    transceiveConf.tx_data = &tx;
    transceiveConf.tx_data_sz = 1;
    transceiveConf.rx_data = &rx;
    transceiveConf.rx_data_sz = 1;

    /* Set expectations */
    MOCK(mfrc522_ll_send);
    MOCK(mfrc522_ll_recv);
    MOCK(mfrc522_irq_states);
    MOCK(mfrc522_drv_invoke_cmd);
    IGNORE_REDUNDANT_LL_RECV_CALLS();
    InSequence s;
    /* FIFO buffer should be flushed and populated with new data */
    MOCK_CALL(mfrc522_ll_send, mfrc522_reg_fifo_level, 1, NotNull()).WillOnce(Return(mfrc522_ll_status_ok));
    MOCK_CALL(mfrc522_ll_send, mfrc522_reg_fifo_data, 1, Pointee(tx)).WillOnce(Return(mfrc522_ll_status_ok));
    /* Start transceive command and transmission of data */
    MOCK_CALL(mfrc522_drv_invoke_cmd, &device, mfrc522_reg_cmd_transceive).WillOnce(Return(mfrc522_drv_status_ok));
    MOCK_CALL(mfrc522_ll_send, mfrc522_reg_bit_framing, 1, _).WillOnce(Return(mfrc522_ll_status_ok));
    /* Simulate that no IRQ is set (states = 0x0000) */
    MOCK_CALL(mfrc522_irq_states, &device, NotNull()).Times(AtLeast(1))
        .WillRepeatedly(DoAll(SetArgPointee<1>(0x0000), Return(mfrc522_drv_status_ok)));
    /* Stop transmission of data and enter Idle state back */
    MOCK_CALL(mfrc522_ll_send, mfrc522_reg_bit_framing, 1, NotNull()).WillOnce(Return(mfrc522_ll_status_ok));
    MOCK_CALL(mfrc522_drv_invoke_cmd, &device, mfrc522_reg_cmd_idle).WillOnce(Return(mfrc522_drv_status_ok));

    /* Check results */
    auto status = mfrc522_drv_transceive(&device, &transceiveConf);
    ASSERT_EQ(mfrc522_drv_status_transceive_timeout, status);
    ASSERT_EQ(0xAA, rx); /* RX data shall not be touched */
}

TEST(TestMfrc522DrvCommon, mfrc522_drv_transceive__ErrorBitIsSet__ErrorReturned)
{
    auto device = initDevice();

    /* Populate configuration struct */
    u8 tx = 0xCF;
    u8 rx = 0xAA;
    mfrc522_drv_transceive_conf transceiveConf;
    transceiveConf.tx_data = &tx;
    transceiveConf.tx_data_sz = 1;
    transceiveConf.rx_data = &rx;
    transceiveConf.rx_data_sz = 1;

    /* Set expectations */
    MOCK(mfrc522_ll_send);
    MOCK(mfrc522_ll_recv);
    MOCK(mfrc522_irq_states);
    MOCK(mfrc522_drv_invoke_cmd);
    IGNORE_REDUNDANT_LL_RECV_CALLS();
    InSequence s;
    /* FIFO buffer should be flushed and populated with new data */
    MOCK_CALL(mfrc522_ll_send, mfrc522_reg_fifo_level, 1, NotNull()).WillOnce(Return(mfrc522_ll_status_ok));
    MOCK_CALL(mfrc522_ll_send, mfrc522_reg_fifo_data, 1, Pointee(tx)).WillOnce(Return(mfrc522_ll_status_ok));
    /* Start transceive command and transmission of data */
    MOCK_CALL(mfrc522_drv_invoke_cmd, &device, mfrc522_reg_cmd_transceive).WillOnce(Return(mfrc522_drv_status_ok));
    MOCK_CALL(mfrc522_ll_send, mfrc522_reg_bit_framing, 1, _).WillOnce(Return(mfrc522_ll_status_ok));
    /* Simulate that error IRQ is set (low byte = 0x02) */
    MOCK_CALL(mfrc522_irq_states, &device, NotNull()).Times(AtLeast(1))
            .WillRepeatedly(DoAll(SetArgPointee<1>(0x0002), Return(mfrc522_drv_status_ok)));
    /* Stop transmission of data and enter Idle state back */
    MOCK_CALL(mfrc522_ll_send, mfrc522_reg_bit_framing, 1, NotNull()).WillOnce(Return(mfrc522_ll_status_ok));
    MOCK_CALL(mfrc522_drv_invoke_cmd, &device, mfrc522_reg_cmd_idle).WillOnce(Return(mfrc522_drv_status_ok));

    /* Check results */
    auto status = mfrc522_drv_transceive(&device, &transceiveConf);
    ASSERT_EQ(mfrc522_drv_status_transceive_err, status);
    ASSERT_EQ(0xAA, rx); /* RX data shall not be touched */
}

TEST(TestMfrc522DrvCommon, mfrc522_drv_transceive__InvalidNumberOfRxValidBits__Error)
{
    auto device = initDevice();

    /* Populate configuration struct */
    u8 tx = 0xCF;
    u8 rx = 0xAA;
    mfrc522_drv_transceive_conf transceiveConf;
    transceiveConf.tx_data = &tx;
    transceiveConf.tx_data_sz = 1;
    transceiveConf.rx_data = &rx;
    transceiveConf.rx_data_sz = 1;

    /* Set expectations */
    MOCK(mfrc522_ll_send);
    MOCK(mfrc522_ll_recv);
    MOCK(mfrc522_irq_states);
    MOCK(mfrc522_drv_invoke_cmd);
    IGNORE_REDUNDANT_LL_RECV_CALLS();
    InSequence s;
    /* FIFO buffer should be flushed and populated with new data */
    MOCK_CALL(mfrc522_ll_send, mfrc522_reg_fifo_level, 1, NotNull()).WillOnce(Return(mfrc522_ll_status_ok));
    MOCK_CALL(mfrc522_ll_send, mfrc522_reg_fifo_data, 1, Pointee(tx)).WillOnce(Return(mfrc522_ll_status_ok));
    /* Start transceive command and transmission of data */
    MOCK_CALL(mfrc522_drv_invoke_cmd, &device, mfrc522_reg_cmd_transceive).WillOnce(Return(mfrc522_drv_status_ok));
    MOCK_CALL(mfrc522_ll_send, mfrc522_reg_bit_framing, 1, _).WillOnce(Return(mfrc522_ll_status_ok));
    MOCK_CALL(mfrc522_irq_states, &device, NotNull()).Times(AtLeast(1))
            .WillRepeatedly(DoAll(SetArgPointee<1>(1 << 5), Return(mfrc522_drv_status_ok)));
    /* Stop transmission of data and enter Idle state back */
    MOCK_CALL(mfrc522_ll_send, mfrc522_reg_bit_framing, 1, NotNull()).WillOnce(Return(mfrc522_ll_status_ok));
    MOCK_CALL(mfrc522_drv_invoke_cmd, &device, mfrc522_reg_cmd_idle).WillOnce(Return(mfrc522_drv_status_ok));
    /* Simulate that RX last bits = 0x01 */
    MOCK_CALL(mfrc522_ll_recv, mfrc522_reg_control, NotNull())
            .WillOnce(DoAll(SetArgPointee<1>(0x01), Return(mfrc522_ll_status_ok)));

    /* Check results */
    auto status = mfrc522_drv_transceive(&device, &transceiveConf);
    ASSERT_EQ(mfrc522_drv_status_transceive_rx_mism, status);
    ASSERT_EQ(0xAA, rx); /* RX data shall not be touched */
}

TEST(TestMfrc522DrvCommon, mfrc522_drv_transceive__NoDataOnRxSide__Error)
{
    auto device = initDevice();

    /* Populate configuration struct */
    u8 tx = 0xCF;
    u8 rx = 0xAA;
    mfrc522_drv_transceive_conf transceiveConf;
    transceiveConf.tx_data = &tx;
    transceiveConf.tx_data_sz = 1;
    transceiveConf.rx_data = &rx;
    transceiveConf.rx_data_sz = 1;

    /* Set expectations */
    MOCK(mfrc522_ll_send);
    MOCK(mfrc522_ll_recv);
    MOCK(mfrc522_irq_states);
    MOCK(mfrc522_drv_invoke_cmd);
    IGNORE_REDUNDANT_LL_RECV_CALLS();
    InSequence s;
    /* FIFO buffer should be flushed and populated with new data */
    MOCK_CALL(mfrc522_ll_send, mfrc522_reg_fifo_level, 1, NotNull()).WillOnce(Return(mfrc522_ll_status_ok));
    MOCK_CALL(mfrc522_ll_send, mfrc522_reg_fifo_data, 1, Pointee(tx)).WillOnce(Return(mfrc522_ll_status_ok));
    /* Start transceive command and transmission of data */
    MOCK_CALL(mfrc522_drv_invoke_cmd, &device, mfrc522_reg_cmd_transceive).WillOnce(Return(mfrc522_drv_status_ok));
    MOCK_CALL(mfrc522_ll_send, mfrc522_reg_bit_framing, 1, _).WillOnce(Return(mfrc522_ll_status_ok));
    MOCK_CALL(mfrc522_irq_states, &device, NotNull()).Times(AtLeast(1))
            .WillRepeatedly(DoAll(SetArgPointee<1>(1 << 5), Return(mfrc522_drv_status_ok)));
    /* Stop transmission of data and enter Idle state back */
    MOCK_CALL(mfrc522_ll_send, mfrc522_reg_bit_framing, 1, NotNull()).WillOnce(Return(mfrc522_ll_status_ok));
    MOCK_CALL(mfrc522_drv_invoke_cmd, &device, mfrc522_reg_cmd_idle).WillOnce(Return(mfrc522_drv_status_ok));
    /* Get number of RX last bits and FIFO level */
    MOCK_CALL(mfrc522_ll_recv, mfrc522_reg_control, NotNull())
            .WillOnce(DoAll(SetArgPointee<1>(0x00), Return(mfrc522_ll_status_ok)));
    MOCK_CALL(mfrc522_ll_recv, mfrc522_reg_fifo_level, NotNull())
            .WillOnce(DoAll(SetArgPointee<1>(0x00), Return(mfrc522_ll_status_ok))); /* Lack of RX bytes */

    /* Check results */
    auto status = mfrc522_drv_transceive(&device, &transceiveConf);
    ASSERT_EQ(mfrc522_drv_status_transceive_rx_mism, status);
    ASSERT_EQ(0xAA, rx); /* RX data shall not be touched */
}

TEST(TestMfrc522DrvCommon, mfrc522_drv_transceive__TypicalCase__Success)
{
    auto device = initDevice();

    /* Populate configuration struct */
    u8 tx = 0xCF;
    u8 rx[2] = {0x00};
    mfrc522_drv_transceive_conf transceiveConf;
    transceiveConf.tx_data = &tx;
    transceiveConf.tx_data_sz = 1;
    transceiveConf.rx_data = &rx[0];
    transceiveConf.rx_data_sz = 2;

    /* Set expectations */
    MOCK(mfrc522_ll_send);
    MOCK(mfrc522_ll_recv);
    MOCK(mfrc522_irq_states);
    MOCK(mfrc522_drv_invoke_cmd);
    IGNORE_REDUNDANT_LL_RECV_CALLS();
    InSequence s;
    /* FIFO buffer should be flushed and populated with new data */
    MOCK_CALL(mfrc522_ll_send, mfrc522_reg_fifo_level, 1, NotNull()).WillOnce(Return(mfrc522_ll_status_ok));
    MOCK_CALL(mfrc522_ll_send, mfrc522_reg_fifo_data, 1, Pointee(tx)).WillOnce(Return(mfrc522_ll_status_ok));
    /* Start transceive command and transmission of data */
    MOCK_CALL(mfrc522_drv_invoke_cmd, &device, mfrc522_reg_cmd_transceive).WillOnce(Return(mfrc522_drv_status_ok));
    MOCK_CALL(mfrc522_ll_send, mfrc522_reg_bit_framing, 1, _).WillOnce(Return(mfrc522_ll_status_ok));
    MOCK_CALL(mfrc522_irq_states, &device, NotNull()).Times(AtLeast(1))
            .WillRepeatedly(DoAll(SetArgPointee<1>(1 << 5), Return(mfrc522_drv_status_ok)));
    /* Stop transmission of data and enter Idle state back */
    MOCK_CALL(mfrc522_ll_send, mfrc522_reg_bit_framing, 1, NotNull()).WillOnce(Return(mfrc522_ll_status_ok));
    MOCK_CALL(mfrc522_drv_invoke_cmd, &device, mfrc522_reg_cmd_idle).WillOnce(Return(mfrc522_drv_status_ok));
    /* Get number of RX last bits and FIFO level */
    MOCK_CALL(mfrc522_ll_recv, mfrc522_reg_control, NotNull())
            .WillOnce(DoAll(SetArgPointee<1>(0x00), Return(mfrc522_ll_status_ok)));
    MOCK_CALL(mfrc522_ll_recv, mfrc522_reg_fifo_level, NotNull())
            .WillOnce(DoAll(SetArgPointee<1>(0x02), Return(mfrc522_ll_status_ok)));
    /* Read RX data */
    MOCK_CALL(mfrc522_ll_recv, mfrc522_reg_fifo_data, NotNull())
            .WillOnce(DoAll(SetArgPointee<1>(0xBB), Return(mfrc522_ll_status_ok)))
            .WillOnce(DoAll(SetArgPointee<1>(0xCC), Return(mfrc522_ll_status_ok)));

    /* Check results */
    auto status = mfrc522_drv_transceive(&device, &transceiveConf);
    ASSERT_EQ(mfrc522_drv_status_ok, status);
    ASSERT_EQ(0xBB, rx[0]);
    ASSERT_EQ(0xCC, rx[1]);
}

TEST(TestMfrc522DrvCommon, mfrc522_drv_reqa__NullCases)
{
    auto device = initDevice();
    u16 atqa;

    auto status = mfrc522_drv_reqa(&device, nullptr);
    ASSERT_EQ(mfrc522_drv_status_nullptr, status);

    status = mfrc522_drv_reqa(nullptr, &atqa);
    ASSERT_EQ(mfrc522_drv_status_nullptr, status);
}

TEST(TestMfrc522DrvCommon, mfrc522_drv_reqa__TransceiveError__InvalidATQAReturned)
{
    auto device = initDevice();

    /* Set the same expectations for each test case */
    MOCK(mfrc522_ll_send);
    MOCK(mfrc522_drv_transceive);
    /* TX last bits should be set to 0x07 in each test cases */
    MOCK_CALL(mfrc522_ll_send, mfrc522_reg_bit_framing, 1, Pointee(0x07)).Times(3)
        .WillRepeatedly(Return(mfrc522_ll_status_ok));
    /* Simulate failure in transceive process */
    MOCK_CALL(mfrc522_drv_transceive, &device, NotNull())
        .WillOnce(Return(mfrc522_drv_status_transceive_timeout))
        .WillOnce(Return(mfrc522_drv_status_transceive_err))
        .WillOnce(Return(mfrc522_drv_status_transceive_rx_mism));

    /* Case 1 */
    u16 atqa1;
    auto status1 = mfrc522_drv_reqa(&device, &atqa1);
    ASSERT_EQ(mfrc522_drv_status_transceive_timeout, status1);
    ASSERT_EQ(MFRC522_PICC_ATQA_INV, atqa1);

    /* Case 2 */
    u16 atqa2;
    auto status2 = mfrc522_drv_reqa(&device, &atqa2);
    ASSERT_EQ(mfrc522_drv_status_transceive_err, status2);
    ASSERT_EQ(MFRC522_PICC_ATQA_INV, atqa2);

    /* Case 3 */
    u16 atqa3;
    auto status3 = mfrc522_drv_reqa(&device, &atqa3);
    ASSERT_EQ(mfrc522_drv_status_transceive_rx_mism, status3);
    ASSERT_EQ(MFRC522_PICC_ATQA_INV, atqa3);
}

TEST(TestMfrc522DrvCommon, mfrc522_drv_reqa__TypicalCase__Success)
{
    auto device = initDevice();
    u16 atqa;

    /* Set expectations */
    MOCK(mfrc522_ll_send);
    MOCK(mfrc522_drv_transceive);
    InSequence s;
    u8 txData[1] = {mfrc522_picc_cmd_reqa}; /* TX data (expected input to mocked function) */
    u8 rxData[2] = {0x04, 0x00}; /* RX data (expected output from mocked function) */
    mfrc522_drv_transceive_conf transceiveConf;
    transceiveConf.tx_data = &txData[0];
    transceiveConf.tx_data_sz = 1;
    transceiveConf.rx_data = &rxData[0];
    transceiveConf.rx_data_sz = 2;
    /* TX last bits should be set to 0x07 */
    MOCK_CALL(mfrc522_ll_send, mfrc522_reg_bit_framing, 1, Pointee(0x07)).WillOnce(Return(mfrc522_ll_status_ok));
    MOCK_CALL(mfrc522_drv_transceive, &device, TransceiveStructInputMatcher(&transceiveConf))
        .WillOnce(DoAll(TransceiveAction(&transceiveConf), Return(mfrc522_drv_status_ok)));

    auto status = mfrc522_drv_reqa(&device, &atqa);
    ASSERT_EQ(mfrc522_drv_status_ok, status);
    ASSERT_EQ(0x0004, atqa);
}

TEST(TestMfrc522DrvCommon, mfrc522_drv_reqa__BlacklistedAtqaReturned__Error)
{
    auto device = initDevice();
    device.atqa_verify_fn = piccAccept0x0004; /* Accept PICC with 0x0004 ATQA response */
    u16 atqa;

    /* Set expectations */
    MOCK(mfrc522_ll_send);
    MOCK(mfrc522_drv_transceive);
    InSequence s;
    u8 txData[1] = {mfrc522_picc_cmd_reqa}; /* TX data (expected input to mocked function) */
    u8 rxData[2] = {0xFF, 0xAA}; /* RX data (expected output from mocked function) */
    mfrc522_drv_transceive_conf transceiveConf;
    transceiveConf.tx_data = &txData[0];
    transceiveConf.tx_data_sz = 1;
    transceiveConf.rx_data = &rxData[0];
    transceiveConf.rx_data_sz = 2;
    /* TX last bits should be set to 0x07 */
    MOCK_CALL(mfrc522_ll_send, mfrc522_reg_bit_framing, 1, Pointee(0x07)).WillOnce(Return(mfrc522_ll_status_ok));
    MOCK_CALL(mfrc522_drv_transceive, &device, TransceiveStructInputMatcher(&transceiveConf))
            .WillOnce(DoAll(TransceiveAction(&transceiveConf), Return(mfrc522_drv_status_ok)));

    auto status = mfrc522_drv_reqa(&device, &atqa);
    ASSERT_EQ(mfrc522_drv_status_picc_vrf_err, status);
    ASSERT_EQ(0xAAFF, atqa);
}

TEST(TestMfrc522DrvCommon, mfrc522_drv_anticollision__NullCases)
{
    auto device = initDevice();
    u8 serial[5];

    auto status = mfrc522_drv_anticollision(nullptr, &serial[0]);
    ASSERT_EQ(mfrc522_drv_status_nullptr, status);
    status = mfrc522_drv_anticollision(&device, nullptr);
    ASSERT_EQ(mfrc522_drv_status_nullptr, status);
}

TEST(TestMfrc522DrvCommon, mfrc522_drv_anticollision__ErrorDuringTransceiveCommand__Error)
{
    auto device = initDevice();
    u8 serial[5] = {0x00};

    /* Set expectations */
    MOCK(mfrc522_ll_send);
    MOCK(mfrc522_drv_transceive);
    /* TX last bits = 0x00 (whole byte valid) */
    MOCK_CALL(mfrc522_ll_send, mfrc522_reg_bit_framing, 1, Pointee(0x00)).WillOnce(Return(mfrc522_ll_status_ok));
    /* Simulate an error during transmission/reception of the data */
    MOCK_CALL(mfrc522_drv_transceive, &device, NotNull()).WillOnce(Return(mfrc522_drv_status_transceive_rx_mism));

    auto status = mfrc522_drv_anticollision(&device, &serial[0]);
    ASSERT_EQ(mfrc522_drv_status_transceive_rx_mism, status); /* The error should be forwarded as an output */
    for (const auto& byte : serial) {
        ASSERT_EQ(0x00, byte); /* Serial data shall not be touched */
    }
}

TEST(TestMfrc522DrvCommon, mfrc522_drv_anticollision__InvalidChecksum__Error)
{
    auto device = initDevice();
    u8 serial[5] = {0x00};

    /* Set expectations */
    MOCK(mfrc522_ll_send);
    MOCK(mfrc522_drv_transceive);
    u8 txData[2] = {0x93, 0x20};
    u8 rxData[5] = {0x73, 0xEF, 0xD7, 0x18, 0xAB}; /* 0xAB is invalid in this case */
    mfrc522_drv_transceive_conf transceiveConf;
    transceiveConf.tx_data = &txData[0];
    transceiveConf.tx_data_sz = 2;
    transceiveConf.rx_data = &rxData[0];
    transceiveConf.rx_data_sz = 5;
    /* TX last bits = 0x00 (whole byte valid) */
    MOCK_CALL(mfrc522_ll_send, mfrc522_reg_bit_framing, 1, Pointee(0x00)).WillOnce(Return(mfrc522_ll_status_ok));
    MOCK_CALL(mfrc522_drv_transceive, &device, TransceiveStructInputMatcher(&transceiveConf))
            .WillOnce(DoAll(TransceiveAction(&transceiveConf), Return(mfrc522_drv_status_ok)));

    auto status = mfrc522_drv_anticollision(&device, &serial[0]);
    ASSERT_EQ(mfrc522_drv_status_anticoll_chksum_err, status);
}

TEST(TestMfrc522DrvCommon, mfrc522_drv_anticollision__ValidNUIDAndChecksumReturned__Success)
{
    /*
     * Example valid serial outputs after anticollision command:
     * 1) 0x73 0xef 0xd7 0x18 0x53
     * 2) 0x83 0x35 0x19 0x16 0xb9
     */

    auto device = initDevice();
    u8 serial[5] = {0x00};

    /* Set expectations */
    MOCK(mfrc522_ll_send);
    MOCK(mfrc522_drv_transceive);
    u8 txData[2] = {0x93, 0x20};
    u8 rxData[5] = {0x73, 0xEF, 0xD7, 0x18, 0x53};
    mfrc522_drv_transceive_conf transceiveConf;
    transceiveConf.tx_data = &txData[0];
    transceiveConf.tx_data_sz = 2;
    transceiveConf.rx_data = &rxData[0];
    transceiveConf.rx_data_sz = 5;
    /* TX last bits = 0x00 (whole byte valid) */
    MOCK_CALL(mfrc522_ll_send, mfrc522_reg_bit_framing, 1, Pointee(0x00)).WillOnce(Return(mfrc522_ll_status_ok));
    MOCK_CALL(mfrc522_drv_transceive, &device, TransceiveStructInputMatcher(&transceiveConf))
            .WillOnce(DoAll(TransceiveAction(&transceiveConf), Return(mfrc522_drv_status_ok)));

    auto status = mfrc522_drv_anticollision(&device, &serial[0]);
    ASSERT_EQ(mfrc522_drv_status_ok, status);
    ASSERT_EQ(0, memcmp(&serial[0], &rxData[0], 5)); /* Compare output serial with expected one */
}
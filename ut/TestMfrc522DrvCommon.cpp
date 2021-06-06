#include "mfrc522_drv.h"
#include "mfrc522_conf.h"
#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "Mockable.h" /* Provide mocks */

/* Required by C Mock library */
DEFINE_MOCKABLE(mfrc522_ll_status, mfrc522_ll_send, (u8, size, u8*));
DEFINE_MOCKABLE(mfrc522_ll_status, mfrc522_ll_recv, (u8, u8*));
DEFINE_MOCKABLE(void, mfrc522_ll_delay, (u32));
DEFINE_MOCKABLE(mfrc522_drv_status, mfrc522_drv_init, (mfrc522_drv_conf*));
DEFINE_MOCKABLE(mfrc522_drv_status, mfrc522_drv_soft_reset, (const mfrc522_drv_conf*));
DEFINE_MOCKABLE(mfrc522_drv_status, mfrc522_drv_invoke_cmd, (const mfrc522_drv_conf*, mfrc522_reg_cmd));

using namespace testing;

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
    auto status = mfrc522_drv_write(nullptr, mfrc522_reg_fifo_data_reg, 1, &payload);
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

TEST(TestMfrc522DrvCommon, mfrc522_drv_fifo_store__ValidLowLevelCallIsMade)
{
    auto device = initDevice();

    /* Expect that low-level call is made */
    MOCK(mfrc522_ll_send);
    MOCK_CALL(mfrc522_ll_send, mfrc522_reg_fifo_data_reg, 1, Pointee(0xBC)).WillOnce(Return(mfrc522_ll_status_ok));

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
    MOCK_CALL(mfrc522_ll_send, mfrc522_reg_fifo_data_reg, SIZE_ARRAY(bytes), &bytes[0])
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
    MOCK_CALL(mfrc522_ll_recv, mfrc522_reg_fifo_data_reg, &buffer)
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
    MOCK_CALL(mfrc522_ll_send, mfrc522_reg_fifo_level_reg, 1, Pointee(0x80)).WillOnce(Return(mfrc522_ll_status_ok));

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
    ruConf.addr = mfrc522_reg_fifo_data_reg;
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
    ruConf.addr = mfrc522_reg_fifo_data_reg;
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
    ruConf.addr = mfrc522_reg_fifo_data_reg;
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
    ruConf.addr = mfrc522_reg_fifo_data_reg;
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
    MOCK_CALL(mfrc522_ll_recv, mfrc522_reg_command, _)
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
    MOCK_CALL(mfrc522_ll_send, mfrc522_reg_fifo_data_reg, 1, Pointee(0x00))
        .WillOnce(Return(mfrc522_ll_status_ok));
    MOCK_CALL(mfrc522_drv_invoke_cmd, &conf, mfrc522_reg_cmd_mem)
        .WillOnce(Return(mfrc522_drv_status_ok));
    /* 3. Enable the self test by writing 09h to the AutoTestReg register */
    MOCK_CALL(mfrc522_ll_send, mfrc522_reg_auto_test, 1, _)
        .WillOnce(Return(mfrc522_ll_status_ok));
    /* 4. Write 00h to the FIFO buffer */
    MOCK_CALL(mfrc522_ll_send, mfrc522_reg_fifo_data_reg, 1, Pointee(0x00))
        .WillOnce(Return(mfrc522_ll_status_ok));
    /* 5. Start the self test with the CalcCRC command */
    MOCK_CALL(mfrc522_drv_invoke_cmd, &conf, mfrc522_reg_cmd_crc)
        .WillOnce(Return(mfrc522_drv_status_ok));
    /* 6 Wait until FIFO buffer contains 64 bytes */
    MOCK_CALL(mfrc522_ll_recv, mfrc522_reg_fifo_level_reg, _)
        .WillOnce(DoAll(SetArgPointee<1>(64), Return(mfrc522_ll_status_ok)));
    /* Read FIFO contents */
    u8 ret[MFRC522_DRV_SELF_TEST_FIFO_SZ] = {MFRC522_CONF_SELF_TEST_FIFO_OUT};
    for (const auto& r : ret) {
        MOCK_CALL(mfrc522_ll_recv, mfrc522_reg_fifo_data_reg, _)
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
    MOCK_CALL(mfrc522_ll_send, mfrc522_reg_fifo_data_reg, 1, Pointee(0x00))
            .WillOnce(Return(mfrc522_ll_status_ok));
    MOCK_CALL(mfrc522_drv_invoke_cmd, &conf, mfrc522_reg_cmd_mem)
            .WillOnce(Return(mfrc522_drv_status_ok));
    /* 3. Enable the self test by writing 09h to the AutoTestReg register */
    MOCK_CALL(mfrc522_ll_send, mfrc522_reg_auto_test, 1, _)
            .WillOnce(Return(mfrc522_ll_status_ok));
    /* 4. Write 00h to the FIFO buffer */
    MOCK_CALL(mfrc522_ll_send, mfrc522_reg_fifo_data_reg, 1, Pointee(0x00))
            .WillOnce(Return(mfrc522_ll_status_ok));
    /* 5. Start the self test with the CalcCRC command */
    MOCK_CALL(mfrc522_drv_invoke_cmd, &conf, mfrc522_reg_cmd_crc)
            .WillOnce(Return(mfrc522_drv_status_ok));
    /* 6 Wait until FIFO buffer contains 64 bytes */
    MOCK_CALL(mfrc522_ll_recv, mfrc522_reg_fifo_level_reg, _)
            .WillOnce(DoAll(SetArgPointee<1>(64), Return(mfrc522_ll_status_ok)));
    /* Read FIFO contents */
    u8 ret[MFRC522_DRV_SELF_TEST_FIFO_SZ] = {0x00}; /* Only zeros returned */
    for (const auto& r : ret) {
        MOCK_CALL(mfrc522_ll_recv, mfrc522_reg_fifo_data_reg, _)
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
    MOCK_CALL(mfrc522_ll_recv, mfrc522_reg_status1_reg, _)
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
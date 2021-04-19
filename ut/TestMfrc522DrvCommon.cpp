#include "TestRunner.h"
#include "mfrc522_drv.h"
#include "mfrc522_reg.h"
#include "Mfrc522MockWrapper.h"

/* ------------------------------------------------------------ */
/* ------------------------ Test groups ----------------------- */
/* ------------------------------------------------------------ */

TEST_GROUP(TestMfrc522DrvCommon)
{
    LowLevelCallParams lowLevelCallParams = {};

    void setup() override
    {
        /* Nothing to do here */
    }

    void teardown() override
    {
        mock().checkExpectations();
        mfrc522DestroyLowLevelParams(lowLevelCallParams);
        mock().clear();
    }

    /* Return initialized device structure by value. The function must be called prior to initializing fake device */
    auto initDevice()
    {
        /* Add fake response to return version number */
        lowLevelCallParams.push_back(READ(1, mfrc522_reg_version, 0x9B));
        mfrc522UpdateLowLevelExpectations(lowLevelCallParams);

        mfrc522_drv_conf conf;
        auto status = mfrc522_drv_init(&conf);
        CHECK_EQUAL(mfrc522_drv_status_ok, status);

        /* Flush params vector */
        mfrc522DestroyLowLevelParams(lowLevelCallParams);
        return conf;
    }
};

/* ------------------------------------------------------------ */
/* ------------------------ Test cases ------------------------ */
/* ------------------------------------------------------------ */
//
TEST(TestMfrc522DrvCommon, mfrc522_drv_init__NullCases)
{
    auto status = mfrc522_drv_init(nullptr);
    CHECK_EQUAL(mfrc522_drv_status_nullptr, status);
}

TEST(TestMfrc522DrvCommon, mfrc522_drv_init__LlReceiveError__LlErrorIsGenerated)
{
    u8 payload = 0xAB; /* Assume that low-level call failed, thus trash value was returned */
    auto llStatus = mfrc522_ll_status_recv_err;
    mock().expectOneCall("mfrc522_ll_recv")
    .withParameter("addr", mfrc522_reg_version)
    .withOutputParameterReturning("payload", &payload, sizeof(payload))
    .andReturnValue(&llStatus);

    mfrc522_drv_conf conf;
    auto status = mfrc522_drv_init(&conf);
    CHECK_EQUAL(mfrc522_drv_status_ll_err, status);
    /* In a case low-level API returns an error 'chip_version' field should be equal to MFRC522_REG_VERSION_INVALID */
    CHECK_EQUAL(MFRC522_REG_VERSION_INVALID, conf.chip_version);
}

TEST(TestMfrc522DrvCommon, mfrc522_drv_init__DeviceNotSupported__Failure) {
    /* Populate fake responses */
    lowLevelCallParams.push_back(READ(1, mfrc522_reg_version, 0xAA));
    mfrc522UpdateLowLevelExpectations(lowLevelCallParams);

    mfrc522_drv_conf conf;
    auto status = mfrc522_drv_init(&conf);
    CHECK_EQUAL(mfrc522_drv_status_dev_err, status);
}

TEST(TestMfrc522DrvCommon, mfrc522_drv_init__DeviceFound__Success)
{
    auto conf = initDevice();
    /* Check if 'chip_version' field was set */
    CHECK_EQUAL(0x9B, conf.chip_version);
}

TEST(TestMfrc522DrvCommon, mfrc522_drv_write__NullCases)
{
    auto status = mfrc522_drv_write_byte(nullptr, mfrc522_reg_fifo_data_reg, 0xAB);
    CHECK_EQUAL(mfrc522_ll_status_send_err, status);
}

TEST(TestMfrc522DrvCommon, mfrc522_drv_read__NullCases)
{
    mfrc522_drv_conf conf;
    u8 pl;

    auto status = mfrc522_drv_read(&conf, mfrc522_reg_fifo_data_reg, nullptr);
    CHECK_EQUAL(mfrc522_ll_status_recv_err, status);

    status = mfrc522_drv_read(nullptr, mfrc522_reg_fifo_data_reg, &pl);
    CHECK_EQUAL(mfrc522_ll_status_recv_err, status);
}

TEST(TestMfrc522DrvCommon, mfrc522_drv_read_until__NullCases)
{
    mfrc522_drv_conf conf;
    mfrc522_drv_read_until_conf ruConf;

    auto status = mfrc522_drv_read_until(&conf, nullptr);
    CHECK_EQUAL(mfrc522_drv_status_nullptr, status);

    status = mfrc522_drv_read_until(nullptr, &ruConf);
    CHECK_EQUAL(mfrc522_drv_status_nullptr, status);
}

TEST(TestMfrc522DrvCommon, mfrc522_drv_read_until__InfinityFlagEnabled__Success)
{
    auto conf = initDevice();

    mfrc522_drv_read_until_conf ruConf;
    ruConf.addr = mfrc522_reg_fifo_data_reg;
    ruConf.exp_payload = 0x0E;
    ruConf.field_mask = 0x0F;
    ruConf.retry_cnt = MFRC522_DRV_RETRY_CNT_INF;
    ruConf.delay = 1;

    /* Populate fake responses */
    lowLevelCallParams.push_back(READ(100, ruConf.addr, 0x00));
    lowLevelCallParams.push_back(READ(1, ruConf.addr, ruConf.exp_payload));
    mfrc522UpdateLowLevelExpectations(lowLevelCallParams);

    auto status = mfrc522_drv_read_until(&conf, &ruConf);
    CHECK_EQUAL(mfrc522_drv_status_ok, status);
    CHECK_EQUAL(ruConf.exp_payload, ruConf.payload & ruConf.field_mask);
}

TEST(TestMfrc522DrvCommon, mfrc522_drv_read_until__AnswerOnFirstTry__Success)
{
    auto conf = initDevice();

    mfrc522_drv_read_until_conf ruConf;
    ruConf.addr = mfrc522_reg_fifo_data_reg;
    ruConf.exp_payload = 0xF0;
    ruConf.field_mask = 0xF0;
    ruConf.retry_cnt = 0;
    ruConf.delay = 1;

    lowLevelCallParams.push_back(READ(1, ruConf.addr, ruConf.exp_payload));
    mfrc522UpdateLowLevelExpectations(lowLevelCallParams);

    auto status = mfrc522_drv_read_until(&conf, &ruConf);
    CHECK_EQUAL(mfrc522_drv_status_ok, status);
    CHECK_EQUAL(ruConf.exp_payload, ruConf.payload & ruConf.field_mask);
}

TEST(TestMfrc522DrvCommon, mfrc522_drv_read_until__AnswerOnThirdTime__Success)
{
    auto conf = initDevice();

    mfrc522_drv_read_until_conf ruConf;
    ruConf.addr = mfrc522_reg_fifo_data_reg;
    ruConf.exp_payload = 0x80;
    ruConf.field_mask = 0xC0;
    ruConf.retry_cnt = 2; /* One try + two retries */
    ruConf.delay = 1;

    lowLevelCallParams.push_back(READ(1, ruConf.addr, 0x00));
    lowLevelCallParams.push_back(READ(1, ruConf.addr, 0x40));
    lowLevelCallParams.push_back(READ(1, ruConf.addr, 0x8F));
    mfrc522UpdateLowLevelExpectations(lowLevelCallParams);

    auto status = mfrc522_drv_read_until(&conf, &ruConf);
    CHECK_EQUAL(mfrc522_drv_status_ok, status);
    CHECK_EQUAL(ruConf.exp_payload, ruConf.payload & ruConf.field_mask);
}

TEST(TestMfrc522DrvCommon, mfrc522_drv_read_until__NoAnswerAfterAllTries__Failure)
{
    auto conf = initDevice();

    mfrc522_drv_read_until_conf ruConf;
    ruConf.addr = mfrc522_reg_fifo_data_reg;
    ruConf.exp_payload = 0xAA;
    ruConf.field_mask = 0xFF;
    ruConf.retry_cnt = 10;
    ruConf.delay = 1;

    lowLevelCallParams.push_back(READ(ruConf.retry_cnt + 1, ruConf.addr, 0x00));
    mfrc522UpdateLowLevelExpectations(lowLevelCallParams);

    auto status = mfrc522_drv_read_until(&conf, &ruConf);
    CHECK_EQUAL(mfrc522_drv_status_dev_rtr_err, status);

    /* Check recent payload */
    CHECK_EQUAL(0x00, ruConf.payload);
}

TEST(TestMfrc522DrvCommon, mfrc522_soft_reset__NullCases)
{
    auto status = mfrc522_soft_reset(nullptr);
    CHECK_EQUAL(mfrc522_drv_status_nullptr, status);
}

TEST(TestMfrc522DrvCommon, mfrc522_soft_reset__TypicalCase__Success)
{
    auto conf = initDevice();

    /* Phase 0: Some command is running */
    lowLevelCallParams.push_back(READ(1, mfrc522_reg_command, mfrc522_reg_cmd_mem));
    /* Phase 1: Idle command is active */
    lowLevelCallParams.push_back(READ(1, mfrc522_reg_command, mfrc522_reg_cmd_idle));
    /* Phase 2: Perform soft reset */
    lowLevelCallParams.push_back(WRITE1(1, mfrc522_reg_command, mfrc522_reg_cmd_soft_reset));
    /* Phase 3: Soft reset has not done yet */
    lowLevelCallParams.push_back(READ(1, mfrc522_reg_command, mfrc522_reg_cmd_soft_reset));
    /* Phase 4: Idle command is active back */
    lowLevelCallParams.push_back(READ(1, mfrc522_reg_command, mfrc522_reg_cmd_idle));
    mfrc522UpdateLowLevelExpectations(lowLevelCallParams);

    auto status = mfrc522_soft_reset(&conf);
    CHECK_EQUAL(mfrc522_drv_status_ok, status);
}

TEST(TestMfrc522DrvCommon, mfrc522_soft_reset__IdleCommandNotReached__Failure)
{
    auto conf = initDevice();

    lowLevelCallParams.push_back(READ(MFRC522_DRV_DEF_RETRY_CNT + 1, mfrc522_reg_command, mfrc522_reg_cmd_mem));
    mfrc522UpdateLowLevelExpectations(lowLevelCallParams);

    auto status = mfrc522_soft_reset(&conf);
    CHECK_EQUAL(mfrc522_drv_status_dev_rtr_err, status);
}
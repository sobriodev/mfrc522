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

        /* Flush params vector and check expectations */
        mock().checkExpectations();
        mfrc522DestroyLowLevelParams(lowLevelCallParams);
        mock().clear();
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
    ruConf.mask = 0x0F;
    ruConf.retry_cnt = MFRC522_DRV_RETRY_CNT_INF;
    ruConf.delay = 1;

    /* Populate fake responses */
    lowLevelCallParams.push_back(READ(100, ruConf.addr, 0x00));
    lowLevelCallParams.push_back(READ(1, ruConf.addr, ruConf.exp_payload));
    mfrc522UpdateLowLevelExpectations(lowLevelCallParams);

    auto status = mfrc522_drv_read_until(&conf, &ruConf);
    CHECK_EQUAL(mfrc522_drv_status_ok, status);
    CHECK_EQUAL(ruConf.exp_payload, ruConf.payload & ruConf.mask);
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

    lowLevelCallParams.push_back(READ(1, ruConf.addr, ruConf.exp_payload));
    mfrc522UpdateLowLevelExpectations(lowLevelCallParams);

    auto status = mfrc522_drv_read_until(&conf, &ruConf);
    CHECK_EQUAL(mfrc522_drv_status_ok, status);
    CHECK_EQUAL(ruConf.exp_payload, ruConf.payload & ruConf.mask);
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

    lowLevelCallParams.push_back(READ(1, ruConf.addr, 0x00));
    lowLevelCallParams.push_back(READ(1, ruConf.addr, 0x40));
    lowLevelCallParams.push_back(READ(1, ruConf.addr, 0x8F));
    mfrc522UpdateLowLevelExpectations(lowLevelCallParams);

    auto status = mfrc522_drv_read_until(&conf, &ruConf);
    CHECK_EQUAL(mfrc522_drv_status_ok, status);
    CHECK_EQUAL(ruConf.exp_payload, ruConf.payload & ruConf.mask);
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
    /* Phase 3: Soft reset has not been done yet */
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

TEST(TestMfrc522DrvCommon, mfrc522_drv_write_masked__NullCases)
{
    auto status = mfrc522_drv_write_masked(nullptr, mfrc522_reg_demod, 0x0F, 0x0F, 0);
    CHECK_EQUAL(mfrc522_ll_status_send_err, status);
}

TEST(TestMfrc522DrvCommon, mfrc522_drv_write_masked__MaskedWritePerformed)
{
    auto conf = initDevice();

    const u8 msk = 0x0F;
    const u8 pos = 0;
    lowLevelCallParams.push_back(READ(1, mfrc522_reg_gs_n, 0x55));
    lowLevelCallParams.push_back(WRITE1(1, mfrc522_reg_gs_n, 0x5D));
    mfrc522UpdateLowLevelExpectations(lowLevelCallParams);

    auto status = mfrc522_drv_write_masked(&conf, mfrc522_reg_gs_n, 13, msk, pos);
    CHECK_EQUAL(mfrc522_ll_status_ok, status);
}

TEST(TestMfrc522DrvCommon, mfrc522_drv_tim_set__NullCases)
{
    auto status = mfrc522_drv_tim_set(nullptr, 10);
    CHECK_EQUAL(mfrc522_drv_status_nullptr, status);
}

TEST(TestMfrc522DrvCommon, mfrc522_drv_tim_set__PeriodEqualsToZero__Failure)
{
    mfrc522_drv_tim_conf conf;

    auto status = mfrc522_drv_tim_set(&conf, 0);
    CHECK_EQUAL(mfrc522_drv_status_tim_prd_err, status);
}

TEST(TestMfrc522DrvCommon, mfrc522_drv_tim_set__PeriodTooLong__Failure)
{
    mfrc522_drv_tim_conf conf;

    auto status = mfrc522_drv_tim_set(&conf, MFRC522_DRV_TIM_MAX_PERIOD + 1);
    CHECK_EQUAL(mfrc522_drv_status_tim_prd_err, status);
}

TEST(TestMfrc522DrvCommon, mfrc522_drv_tim_set__Periods__Success)
{
    /*
     * First pair member = timer period in ms
     * Second pair member = expected TReload
     */
    const std::pair<u16, u16> testData[] =
    {
        {1, 4},
        {2, 8},
        {100, 400},
        {1212, 4848},
        {1500, 6000},
        {MFRC522_DRV_TIM_MAX_PERIOD, 65532}
    };

    for (const auto& row : testData) {
        mfrc522_drv_tim_conf conf;
        auto status = mfrc522_drv_tim_set(&conf, row.first);
        CHECK_EQUAL(mfrc522_drv_status_ok, status);
        CHECK_EQUAL(1694, conf.prescaler);
        CHECK_EQUAL(mfrc522_drv_tim_psl_even, conf.prescaler_type);
        CHECK_EQUAL(row.second, conf.reload_val);
    }
}

TEST(TestMfrc522DrvCommon, mfrc522_drv_tim_start__NullCases)
{
    mfrc522_drv_conf conf;
    mfrc522_drv_tim_conf timConf;

    auto status = mfrc522_drv_tim_start(nullptr, &timConf);
    CHECK_EQUAL(mfrc522_drv_status_nullptr, status);

    status = mfrc522_drv_tim_start(&conf, nullptr);
    CHECK_EQUAL(mfrc522_drv_status_nullptr, status);
}

/* TODO write unit tests to cover mfrc522_drv_tim_start() */

TEST(TestMfrc522DrvCommon, mfrc522_drv_irq_init__NullCases)
{
    mfrc522_drv_conf conf;
    mfrc522_drv_irq_conf irqConf;

    auto status = mfrc522_drv_irq_init(nullptr, &irqConf);
    CHECK_EQUAL(mfrc522_drv_status_nullptr, status);

    status = mfrc522_drv_irq_init(&conf, nullptr);
    CHECK_EQUAL(mfrc522_drv_status_nullptr, status);
}

TEST(TestMfrc522DrvCommon, mfrc522_drv_irq_init__Success)
{
    auto conf = initDevice();

    /* Populate IRQ config fields */
    mfrc522_drv_irq_conf irqConf;
    irqConf.irq_push_pull = true;
    irqConf.irq_signal_inv = false;

    /* Expect that all IRQs are cleared */
    lowLevelCallParams.push_back(WRITE1_A(1, mfrc522_reg_com_irq));
    lowLevelCallParams.push_back(WRITE1_A(1, mfrc522_reg_div_irq));
    /* Expect calls that configure IRQ registers */
    lowLevelCallParams.push_back(WRITE1_A(1, mfrc522_reg_com_irq_en));
    lowLevelCallParams.push_back(WRITE1_A(1, mfrc522_reg_div_irq_en));
    mfrc522UpdateLowLevelExpectations(lowLevelCallParams);

    auto status = mfrc522_drv_irq_init(&conf, &irqConf);
    CHECK_EQUAL(mfrc522_drv_status_ok, status);
}

TEST(TestMfrc522DrvCommon, mfrc522_drv_irq_clr__NullCases)
{
    auto status = mfrc522_drv_irq_clr(nullptr, mfrc522_reg_irq_all);
    CHECK_EQUAL(mfrc522_drv_status_nullptr, status);
}

TEST(TestMfrc522DrvCommon, mfrc522_drv_irq_clr__ClearAllIrqs__BothRegistersAreUpdated)
{
    auto conf = initDevice();

    /* Expect both registers to be updated */
    u8 word1_irq_bits = 0x7F;
    u8 word2_irq_bits = 0x014;
    lowLevelCallParams.push_back(WRITE1(1, mfrc522_reg_com_irq, word1_irq_bits));
    lowLevelCallParams.push_back(WRITE1(1, mfrc522_reg_div_irq, word2_irq_bits));
    mfrc522UpdateLowLevelExpectations(lowLevelCallParams);

    auto status = mfrc522_drv_irq_clr(&conf, mfrc522_reg_irq_all);
    CHECK_EQUAL(mfrc522_drv_status_ok, status);
}

TEST(TestMfrc522DrvCommon, mfrc522_drv_irq_clr__ClearIrqFromComReg__Success)
{
    auto conf = initDevice();

    /* Expect ComIrqReg to be updated */
    lowLevelCallParams.push_back(WRITE1(1, mfrc522_reg_com_irq, 1 << mfrc522_reg_irq_hi_alert));
    mfrc522UpdateLowLevelExpectations(lowLevelCallParams);

    auto status = mfrc522_drv_irq_clr(&conf, mfrc522_reg_irq_hi_alert);
    CHECK_EQUAL(mfrc522_drv_status_ok, status);
}

TEST(TestMfrc522DrvCommon, mfrc522_drv_irq_clr__ClearIrqFromDivReg__Success)
{
    auto conf = initDevice();

    /* Expect DivIrqReg to be updated */
    u8 crc_irq_bit = 2;
    lowLevelCallParams.push_back(WRITE1(1, mfrc522_reg_div_irq, (u8)(1 << crc_irq_bit)));
    mfrc522UpdateLowLevelExpectations(lowLevelCallParams);

    auto status = mfrc522_drv_irq_clr(&conf, mfrc522_reg_irq_crc);
    CHECK_EQUAL(mfrc522_drv_status_ok, status);
}

TEST(TestMfrc522DrvCommon, mfrc522_drv_irq_en__NullCases)
{
    auto status = mfrc522_drv_irq_en(nullptr, mfrc522_reg_irq_idle, true);
    CHECK_EQUAL(mfrc522_drv_status_nullptr, status);
}

TEST(TestMfrc522DrvCommon, mfrc522_drv_irq_en__IrqAllPassedAsAnArgument__ErrorIsReturned)
{
    auto conf = initDevice();

    auto status = mfrc522_drv_irq_en(&conf, mfrc522_reg_irq_all, true);
    CHECK_EQUAL(mfrc522_drv_status_nok, status);
}

TEST(TestMfrc522DrvCommon, mfrc522_drv_irq_en__EnableIrqInComReg__Success)
{
    auto conf = initDevice();

    /* Pretend that some IRQs are enabled */
    u8 actualIrqs = 0x41;
    lowLevelCallParams.push_back(READ(1, mfrc522_reg_com_irq_en, actualIrqs));
    lowLevelCallParams.push_back(WRITE1(1, mfrc522_reg_com_irq_en, (u8)(actualIrqs | (1 << mfrc522_reg_irq_idle))));
    mfrc522UpdateLowLevelExpectations(lowLevelCallParams);

    auto status = mfrc522_drv_irq_en(&conf, mfrc522_reg_irq_idle, true);
    CHECK_EQUAL(mfrc522_drv_status_ok, status);
}

TEST(TestMfrc522DrvCommon, mfrc522_drv_irq_en__DisableIrqInComReg__Success)
{
    auto conf = initDevice();

    /* Pretend that IRQ was enabled, this 0x00 should be written back */
    u8 actualIrqs = 1 << mfrc522_reg_irq_idle;
    lowLevelCallParams.push_back(READ(1, mfrc522_reg_com_irq_en, actualIrqs));
    lowLevelCallParams.push_back(WRITE1(1, mfrc522_reg_com_irq_en, 0x00));
    mfrc522UpdateLowLevelExpectations(lowLevelCallParams);

    auto status = mfrc522_drv_irq_en(&conf, mfrc522_reg_irq_idle, false);
    CHECK_EQUAL(mfrc522_drv_status_ok, status);
}

TEST(TestMfrc522DrvCommon, mfrc522_drv_ieq_en__EnableIrqInDivReg__Success)
{
    auto conf = initDevice();

    lowLevelCallParams.push_back(READ(1, mfrc522_reg_div_irq_en, 0));
    lowLevelCallParams.push_back(WRITE1(1, mfrc522_reg_div_irq_en, 1 << (mfrc522_reg_irq_crc & ~MFRC522_REG_IRQ_DIV)));
    mfrc522UpdateLowLevelExpectations(lowLevelCallParams);

    auto status = mfrc522_drv_irq_en(&conf, mfrc522_reg_irq_crc, true);
    CHECK_EQUAL(mfrc522_drv_status_ok, status);
}
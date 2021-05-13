#include "TestRunner.h"
#include "mfrc522_drv.h"
#include "mfrc522_reg.h"
#include "Mfrc522MockWrapper.h"

/* ------------------------------------------------------------ */
/* ------------------------ Test groups ----------------------- */
/* ------------------------------------------------------------ */

TEST_GROUP(TestMfrc522DrvTimer)
{
    LowLevelCallParams llCallParams = {};

    void setup() override
    {
        /* Nothing to do here */
    }

    void teardown() override
    {
        mock().checkExpectations();
        mfrc522DestroyLowLevelParams(llCallParams);
        mock().clear();
    }

    /* Return initialized device structure by value. The function must be called prior to initializing fake device */
    auto initDevice()
    {
        /* Add fake response to return version number */
        llCallParams.push_back(READ(1, mfrc522_reg_version, 0x9B));
        mfrc522UpdateLowLevelExpectations(llCallParams);

        mfrc522_drv_conf conf;
        auto status = mfrc522_drv_init(&conf);
        CHECK_EQUAL(mfrc522_drv_status_ok, status);

        /* Flush params vector and check expectations */
        mock().checkExpectations();
        mfrc522DestroyLowLevelParams(llCallParams);
        mock().clear();
        return conf;
    }
};

/* ------------------------------------------------------------ */
/* ------------------------ Test cases ------------------------ */
/* ------------------------------------------------------------ */

TEST(TestMfrc522DrvTimer, mfrc522_drv_tim_set__NullCases)
{
    auto status = mfrc522_drv_tim_set(nullptr, 10);
    CHECK_EQUAL(mfrc522_drv_status_nullptr, status);
}

TEST(TestMfrc522DrvTimer, mfrc522_drv_tim_set__PeriodEqualsToZero__Failure)
{
    mfrc522_drv_tim_conf conf;

    auto status = mfrc522_drv_tim_set(&conf, 0);
    CHECK_EQUAL(mfrc522_drv_status_tim_prd_err, status);
}

TEST(TestMfrc522DrvTimer, mfrc522_drv_tim_set__PeriodTooLong__Failure)
{
    mfrc522_drv_tim_conf conf;

    auto status = mfrc522_drv_tim_set(&conf, MFRC522_DRV_TIM_MAX_PERIOD + 1);
    CHECK_EQUAL(mfrc522_drv_status_tim_prd_err, status);
}

TEST(TestMfrc522DrvTimer, mfrc522_drv_tim_set__Periods__Success)
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

TEST(TestMfrc522DrvTimer, mfrc522_drv_tim_start__NullCases)
{
    mfrc522_drv_conf conf;
    mfrc522_drv_tim_conf timConf;

    auto status = mfrc522_drv_tim_start(nullptr, &timConf);
    CHECK_EQUAL(mfrc522_drv_status_nullptr, status);

    status = mfrc522_drv_tim_start(&conf, nullptr);
    CHECK_EQUAL(mfrc522_drv_status_nullptr, status);
}

TEST(TestMfrc522DrvTimer, mfrc522_drv_tim_start__TypicalCase__TimerStarted)
{
    auto conf = initDevice();

    mfrc522_drv_tim_conf timerConf;
    timerConf.prescaler_type = mfrc522_drv_tim_psl_odd;
    timerConf.prescaler = 0xFABC;
    timerConf.reload_val = 0x0CA0;
    timerConf.periodic = true;

    /* Fill low-level calls vector */
    llCallParams.push_back(WRITE1(1, mfrc522_reg_tim_prescaler, 0xBC));
    llCallParams.push_back(WRITE1(1, mfrc522_reg_tim_mode, 0x0A));
    llCallParams.push_back(WRITE1_A(1, mfrc522_reg_demod));
    llCallParams.push_back(WRITE1(1, mfrc522_reg_tim_reload_lo, 0xA0));
    llCallParams.push_back(WRITE1(1, mfrc522_reg_tim_reload_hi, 0x0C));
    llCallParams.push_back(WRITE1_A(1, mfrc522_reg_tim_mode));
    llCallParams.push_back(WRITE1_A(1, mfrc522_reg_control_reg));
    mfrc522UpdateLowLevelExpectations(llCallParams);

    auto status = mfrc522_drv_tim_start(&conf, &timerConf);
    CHECK_EQUAL(mfrc522_drv_status_ok, status);
}

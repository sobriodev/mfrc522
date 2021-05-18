#include "TestCommon.h"
#include "mfrc522_drv.h"
#include "Mockable.h" /* Provide mocks */

using namespace testing;

/* ------------------------------------------------------------ */
/* ------------------------ Test cases ------------------------ */
/* ------------------------------------------------------------ */

TEST(TestMfrc522DrvTimer, mfrc522_drv_tim_set__NullCases)
{
    auto status = mfrc522_drv_tim_set(nullptr, 10);
    ASSERT_EQ(mfrc522_drv_status_nullptr, status);
}

TEST(TestMfrc522DrvTimer, mfrc522_drv_tim_set__PeriodEqualsToZero__Failure)
{
    mfrc522_drv_tim_conf conf;

    auto status = mfrc522_drv_tim_set(&conf, 0);
    ASSERT_EQ(mfrc522_drv_status_tim_prd_err, status);
}

TEST(TestMfrc522DrvTimer, mfrc522_drv_tim_set__PeriodTooLong__Failure)
{
    mfrc522_drv_tim_conf conf;

    auto status = mfrc522_drv_tim_set(&conf, MFRC522_DRV_TIM_MAX_PERIOD + 1);
    ASSERT_EQ(mfrc522_drv_status_tim_prd_err, status);
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
        ASSERT_EQ(mfrc522_drv_status_ok, status);
        ASSERT_EQ(1694, conf.prescaler);
        ASSERT_EQ(mfrc522_drv_tim_psl_even, conf.prescaler_type);
        ASSERT_EQ(row.second, conf.reload_val);
    }
}

TEST(TestMfrc522DrvTimer, mfrc522_drv_tim_start__NullCases)
{
    mfrc522_drv_conf conf;
    mfrc522_drv_tim_conf timConf;

    auto status = mfrc522_drv_tim_start(nullptr, &timConf);
    ASSERT_EQ(mfrc522_drv_status_nullptr, status);

    status = mfrc522_drv_tim_start(&conf, nullptr);
    ASSERT_EQ(mfrc522_drv_status_nullptr, status);
}

TEST(TestMfrc522DrvTimer, mfrc522_drv_tim_start__TypicalCase__TimerStarted)
{
    INSTALL_HOOK(mfrc522_drv_init, mfrc522_drv_init__STUB);
    mfrc522_drv_conf conf;
    mfrc522_drv_init(&conf);

    mfrc522_drv_tim_conf timerConf;
    timerConf.prescaler_type = mfrc522_drv_tim_psl_odd;
    timerConf.prescaler = 0xFABC;
    timerConf.reload_val = 0x0CA0;
    timerConf.periodic = true;

    /* Fill low-level calls */
    INSTALL_MOCK(mfrc522_ll_send);
    InSequence s;
    CUTIE_EXPECT_CALL(mfrc522_ll_send, mfrc522_reg_tim_prescaler, 1, Pointee(0xBC))
        .WillOnce(Return(mfrc522_ll_status_ok));
    CUTIE_EXPECT_CALL(mfrc522_ll_send, mfrc522_reg_tim_mode, 1, Pointee(0x0A))
        .WillOnce(Return(mfrc522_ll_status_ok));
    CUTIE_EXPECT_CALL(mfrc522_ll_send, mfrc522_reg_demod, 1, _).WillOnce(Return(mfrc522_ll_status_ok));
    CUTIE_EXPECT_CALL(mfrc522_ll_send, mfrc522_reg_tim_reload_lo, 1, Pointee(0xA0))
        .WillOnce(Return(mfrc522_ll_status_ok));
    CUTIE_EXPECT_CALL(mfrc522_ll_send, mfrc522_reg_tim_reload_hi, 1, Pointee(0x0C))
        .WillOnce(Return(mfrc522_ll_status_ok));
    CUTIE_EXPECT_CALL(mfrc522_ll_send, mfrc522_reg_tim_mode, 1, _).WillOnce(Return(mfrc522_ll_status_ok));
    CUTIE_EXPECT_CALL(mfrc522_ll_send, mfrc522_reg_control_reg, 1, _).WillOnce(Return(mfrc522_ll_status_ok));

    auto status = mfrc522_drv_tim_start(&conf, &timerConf);
    ASSERT_EQ(mfrc522_drv_status_ok, status);
}

TEST(TestMfrc522DrvTimer, mfrc522_drv_tim_stop__NullCases)
{
    auto status = mfrc522_drv_tim_stop(nullptr);
    ASSERT_EQ(mfrc522_drv_status_nullptr, status);
}

TEST(TestMfrc522DrvTimer, mfrc522_drv_tim_stop__TimerStopped)
{
    INSTALL_HOOK(mfrc522_drv_init, mfrc522_drv_init__STUB);
    mfrc522_drv_conf conf;
    mfrc522_drv_init(&conf);

    /* Set expectations */
    InSequence s;
    INSTALL_EXPECT_CALL(mfrc522_ll_send, mfrc522_reg_control_reg, 1, _).WillOnce(Return(mfrc522_ll_status_ok));;

    auto status = mfrc522_drv_tim_stop(&conf);
    ASSERT_EQ(mfrc522_drv_status_ok, status);
}
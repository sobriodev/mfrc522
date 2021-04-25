#include "TestRunner.h"
#include "mfrc522_drv.h"
#include "Mfrc522MockWrapper.h"
#include "mfrc522_conf.h"

/* ------------------------------------------------------------ */
/* ------------------------ Test groups ----------------------- */
/* ------------------------------------------------------------ */

TEST_GROUP(TestMfrc522DrvNoLlDelay)
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
};

/* ------------------------------------------------------------ */
/* ------------------------ Test cases ------------------------ */
/* ------------------------------------------------------------ */

TEST(TestMfrc522DrvNoLlDelay, mfrc522_drv_read_until__LowLevelDelayDisabled__RetryCountIncreased)
{
    /* Populate fake responses */
    const auto retryCnt = 10;
    lowLevelCallParams.push_back(READ(1, mfrc522_reg_version, 0x9B));
    lowLevelCallParams.push_back(READ(retryCnt * MFRC522_CONF_RETRY_CNT_MUL + 1, mfrc522_reg_fifo_data_reg, 0x00));
    mfrc522UpdateLowLevelExpectations(lowLevelCallParams);

    /* Init device */
    mfrc522_drv_conf conf;
    auto status = mfrc522_drv_init(&conf);
    CHECK_EQUAL(mfrc522_drv_status_ok, status);

    mfrc522_drv_read_until_conf ruConf;
    ruConf.addr = mfrc522_reg_fifo_data_reg;
    ruConf.exp_payload = 0xFF;
    ruConf.mask = 0x00;
    ruConf.retry_cnt = retryCnt;

    status = mfrc522_drv_read_until(&conf, &ruConf);
    CHECK_EQUAL(mfrc522_drv_status_dev_rtr_err, status);
}
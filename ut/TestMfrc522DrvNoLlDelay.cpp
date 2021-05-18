#include "TestCommon.h"
#include "mfrc522_drv.h"
#include "mfrc522_conf.h"

using namespace testing;

/*
 * As delay function is not present in the scope it is not possible to include Mockable.h file.
 * Thus declare mocks manually.
 */
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpedantic" /* Disable pedantic flag due to problem inside Cutie library */

DECLARE_MOCKABLE(mfrc522_ll_recv, 2);
DECLARE_HOOKABLE(mfrc522_drv_init);

#pragma GCC diagnostic pop

/* ------------------------------------------------------------ */
/* ------------------------ Test cases ------------------------ */
/* ------------------------------------------------------------ */

TEST(TestMfrc522DrvNoLlDelay, mfrc522_drv_read_until__LowLevelDelayDisabled__RetryCountIncreased)
{
    /* Init device */
    INSTALL_HOOK(mfrc522_drv_init, mfrc522_drv_init__STUB);
    mfrc522_drv_conf conf;
    mfrc522_drv_init(&conf);

    /* Populate fake responses */
    const auto retryCnt = 10;
    INSTALL_EXPECT_CALL(mfrc522_ll_recv, mfrc522_reg_fifo_data_reg, _).Times(retryCnt * MFRC522_CONF_RETRY_CNT_MUL + 1)
        .WillRepeatedly(DoAll(SetArgPointee<1>(0x00), Return(mfrc522_ll_status_ok)));

    mfrc522_drv_read_until_conf ruConf;
    ruConf.addr = mfrc522_reg_fifo_data_reg;
    ruConf.exp_payload = 0xFF;
    ruConf.mask = 0x00;
    ruConf.retry_cnt = retryCnt;

    auto status = mfrc522_drv_read_until(&conf, &ruConf);
    ASSERT_EQ(mfrc522_drv_status_dev_rtr_err, status);
}
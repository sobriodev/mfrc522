#include "mfrc522_drv.h"
#include "mfrc522_conf.h"
#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "common/Mockable.h"

using namespace testing;

/* ------------------------------------------------------------ */
/* ------------------------ Test cases ------------------------ */
/* ------------------------------------------------------------ */

TEST(TestMfrc522DrvNoLlDelay, mfrc522_drv_read_until__LowLevelDelayDisabled__RetryCountIncreased)
{
    /* Init device */
    auto conf = initDevice();

    /* Populate fake responses */
    MOCK(mfrc522_ll_recv);
    const auto retryCnt = 10;
    MOCK_CALL(mfrc522_ll_recv, mfrc522_reg_fifo_data, _).Times(retryCnt * MFRC522_CONF_RETRY_CNT_MUL + 1)
        .WillRepeatedly(DoAll(SetArgPointee<1>(0x00), Return(mfrc522_ll_status_ok)));

    mfrc522_drv_read_until_conf ruConf;
    ruConf.addr = mfrc522_reg_fifo_data;
    ruConf.exp_payload = 0xFF;
    ruConf.mask = 0x00;
    ruConf.retry_cnt = retryCnt;

    auto status = mfrc522_drv_read_until(&conf, &ruConf);
    ASSERT_EQ(mfrc522_drv_status_dev_rtr_err, status);
}
#include "TestRunner.h"
#include "mfrc522_drv.h"

/* ------------------------------------------------------------ */
/* ------------------------ Test groups ----------------------- */
/* ------------------------------------------------------------ */

TEST_GROUP(TestMfrc522DrvNoLlDelay)
{
    void setup() override
    {
        /* Nothing to do here */
    }

    void teardown() override
    {
        mock().clear();
    }
};

/* ------------------------------------------------------------ */
/* ------------------------ Test cases ------------------------ */
/* ------------------------------------------------------------ */

TEST(TestMfrc522DrvNoLlDelay, to_be_deleted)
{
    u8 payload = 0x9B;

    mock().strictOrder();

    mock().expectOneCall("mfrc522_ll_recv")
    .withParameter("addr", mfrc522_reg_version)
    .withParameter("bytes", 1)
    .withOutputParameterReturning("payload", &payload, sizeof(payload));

    mfrc522_drv_conf conf;
    auto status = mfrc522_drv_init(&conf);
    CHECK_EQUAL(mfrc522_drv_status_ok, status);

    payload = 0x00;
    mock().expectNCalls(101, "mfrc522_ll_recv")
            .withParameter("addr", mfrc522_reg_fifo_data_reg)
            .withParameter("bytes", 1)
            .withOutputParameterReturning("payload", &payload, sizeof(payload));

    mfrc522_drv_read_until_conf ruConf;
    ruConf.addr = mfrc522_reg_fifo_data_reg;
    ruConf.exp_payload = 0xFF;
    ruConf.field_mask = 0xFF;
    ruConf.retry_cnt = 10;

    status = mfrc522_drv_read_until(&conf, &ruConf);
    CHECK_EQUAL(mfrc522_drv_status_dev_rtr_err, status);

    mock().checkExpectations();
}
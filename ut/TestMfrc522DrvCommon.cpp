#include "TestRunner.h"
#include "mfrc522_drv.h"
#include "StubCommon.h"

/* ------------------------------------------------------------ */
/* ------------------------ Test groups ----------------------- */
/* ------------------------------------------------------------ */

TEST_GROUP(TestMfrc522DrvCommon)
{
    void setup() override {}
    void teardown() override {}
};

/* ------------------------------------------------------------ */
/* ------------------------ Test cases ------------------------ */
/* ------------------------------------------------------------ */

TEST(TestMfrc522DrvCommon, mfrc522_drv_init__NullCases)
{
    CHECK_EQUAL(mfrc522_drv_status_nullptr, mfrc522_drv_init(nullptr));
}

TEST(TestMfrc522DrvCommon, mfrc522_drv_init__LlReceiveError__LlErrorIsGenerated)
{
    mfrc522_drv_conf conf;
    conf.ll_send = stub_mfrc522_ll_send_always_err;
    conf.ll_recv = stub_mfrc522_ll_recv_always_err;

    auto status = mfrc522_drv_init(&conf);
    CHECK_EQUAL(mfrc522_drv_status_ll_err, status);
}

/* TODO add tests to check if init phase returns ok status */
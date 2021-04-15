#include "TestRunner.h"
#include "mfrc522_drv.h"
#include "StubCommon.h"
#include "Mfrc522FakeDev.h"

/* ------------------------------------------------------------ */
/* ------------------------ Test groups ----------------------- */
/* ------------------------------------------------------------ */

TEST_GROUP(TestMfrc522DrvCommon)
{
    void setup() override
    {
        /* Initialize fake device */
        Mfrc522FakeDev::setupFakeDev();
    }

    void teardown() override
    {
        /* De-initialize fake device */
        size res = Mfrc522FakeDev::tearDownFakeDev();
        if (res > 0) {
            FAIL("TestMfrc522DrvCommon: teardown requested but Mfrc522FakeDev has remaining responses!");
        }
    }

    /* Populate config struct with default (faked) low-level calls */
    static void getDefaultConf(mfrc522_drv_conf& conf)
    {
        conf.ll_send = Mfrc522FakeDev::sendImpl;
        conf.ll_recv = Mfrc522FakeDev::recvImpl;
    }
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

TEST(TestMfrc522DrvCommon, mfrc522_drv_init__DeviceFound__Success)
{
    /* Populate fake responses vector */
    const auto fakeDev = Mfrc522FakeDev::getFakeDev();
    fakeDev->addResponse(0x37, {0x9b});

    mfrc522_drv_conf conf;
    getDefaultConf(conf);

    CHECK_EQUAL(mfrc522_drv_status_ok, mfrc522_drv_init(&conf));
}
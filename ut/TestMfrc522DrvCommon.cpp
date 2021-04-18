#include "TestRunner.h"
#include "mfrc522_drv.h"
#include "mfrc522_reg.h"
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
        conf.ll_delay = stubMfrc522Delay;
    }

    /* Return initialized device structure by value. The function must be called prior to initializing fake device */
    static auto initDevice()
    {
        /* Add fake response to return version number */
        const u8 version = 0x9B;
        const auto fakeDev = Mfrc522FakeDev::getFakeDev();
        fakeDev->addResponse(0x37, {version});

        mfrc522_drv_conf conf;
        getDefaultConf(conf);

        auto status = mfrc522_drv_init(&conf);
        CHECK_EQUAL(mfrc522_drv_status_ok, status);
        return conf;
    }
};

/* ------------------------------------------------------------ */
/* ------------------------ Test cases ------------------------ */
/* ------------------------------------------------------------ */

TEST(TestMfrc522DrvCommon, mfrc522_drv_init__NullCases)
{
    auto status = mfrc522_drv_init(nullptr);
    CHECK_EQUAL(mfrc522_drv_status_nullptr, status);
}

TEST(TestMfrc522DrvCommon, mfrc522_drv_init__LlReceiveError__LlErrorIsGenerated)
{
    mfrc522_drv_conf conf;
    conf.ll_send = stubMfrc522SendError;
    conf.ll_recv = stubMfrc522ReceiveError;
    conf.ll_delay = stubMfrc522Delay;

    auto status = mfrc522_drv_init(&conf);
    CHECK_EQUAL(mfrc522_drv_status_ll_err, status);

    /* In a case low-level API returns an error 'chip_version' field should be equal to MFRC522_REG_VERSION_INVALID */
    CHECK_EQUAL(MFRC522_REG_VERSION_INVALID, conf.chip_version);
}

TEST(TestMfrc522DrvCommon, mfrc522_drv_init__DeviceNotSupported__Failure)
{
    /* Populate fake device */
    const auto fakeDev = Mfrc522FakeDev::getFakeDev();
    fakeDev->addResponse(0x37, {0xAA});

    mfrc522_drv_conf conf;
    getDefaultConf(conf);

    auto status = mfrc522_drv_init(&conf);
    CHECK_EQUAL(mfrc522_drv_status_dev_err, status);
}

TEST(TestMfrc522DrvCommon, mfrc522_drv_init__DeviceFound__Success)
{
    /* Populate fake responses vector */
    const u8 version = 0x9B;
    const auto fakeDev = Mfrc522FakeDev::getFakeDev();
    fakeDev->addResponse(0x37, {version});

    mfrc522_drv_conf conf;
    getDefaultConf(conf);

    auto status = mfrc522_drv_init(&conf);
    CHECK_EQUAL(mfrc522_drv_status_ok, status);

    /* Check if 'chip_version' field was set */
    CHECK_EQUAL(version, conf.chip_version);
}

TEST(TestMfrc522DrvCommon, mfrc522_drv_write__NullCases)
{
    auto status = mfrc522_drv_write(nullptr, mfrc522_reg_fifo_data_reg, 0xAB);
    CHECK_EQUAL(mfrc522_ll_status_send_err, status);
}

TEST(TestMfrc522DrvCommon, mfrc522_drv_read__NullCases)
{
    mfrc522_drv_conf conf;
    u8 pl;

    auto status = mfrc522_drv_read(&conf, mfrc522_reg_fifo_data_reg, 1, nullptr);
    CHECK_EQUAL(mfrc522_ll_status_recv_err, status);

    status = mfrc522_drv_read(nullptr, mfrc522_reg_fifo_data_reg, 1, &pl);
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
    const auto conf = initDevice();

    mfrc522_drv_read_until_conf ruConf;
    ruConf.addr = mfrc522_reg_fifo_data_reg;
    ruConf.exp_payload = 0x0E;
    ruConf.field_mask = 0x0F;
    ruConf.retry_cnt = MFRC522_DRV_RETRY_CNT_INF;
    ruConf.delay = 1;

    const auto fakeDev = Mfrc522FakeDev::getFakeDev();
    /* Add a few invalid responses */
    fakeDev->addResponse(ruConf.addr, {0x00}, 100);
    /* Add final response */
    fakeDev->addResponse(ruConf.addr, {ruConf.exp_payload});

    auto status = mfrc522_drv_read_until(&conf, &ruConf);
    CHECK_EQUAL(mfrc522_drv_status_ok, status);
    CHECK_EQUAL(ruConf.exp_payload, ruConf.payload & ruConf.field_mask);
}

TEST(TestMfrc522DrvCommon, mfrc522_drv_read_until__AnswerOnFirstTry__Success)
{
    const auto conf = initDevice();

    mfrc522_drv_read_until_conf ruConf;
    ruConf.addr = mfrc522_reg_fifo_data_reg;
    ruConf.exp_payload = 0xF0;
    ruConf.field_mask = 0xF0;
    ruConf.retry_cnt = 0;
    ruConf.delay = 1;

    const auto fakeDev = Mfrc522FakeDev::getFakeDev();
    fakeDev->addResponse(ruConf.addr, {ruConf.exp_payload});

    auto status = mfrc522_drv_read_until(&conf, &ruConf);
    CHECK_EQUAL(mfrc522_drv_status_ok, status);
    CHECK_EQUAL(ruConf.exp_payload, ruConf.payload & ruConf.field_mask);
}

TEST(TestMfrc522DrvCommon, mfrc522_drv_read_until__AnswerOnThirdTime__Success)
{
    const auto conf = initDevice();

    mfrc522_drv_read_until_conf ruConf;
    ruConf.addr = mfrc522_reg_fifo_data_reg;
    ruConf.exp_payload = 0x80;
    ruConf.field_mask = 0xC0;
    ruConf.retry_cnt = 2; /* One try + two retries */
    ruConf.delay = 1;

    const auto fakeDev = Mfrc522FakeDev::getFakeDev();
    fakeDev->addResponse(ruConf.addr, {0x00});
    fakeDev->addResponse(ruConf.addr, {0x40});
    fakeDev->addResponse(ruConf.addr, {0x8F});

    auto status = mfrc522_drv_read_until(&conf, &ruConf);
    CHECK_EQUAL(mfrc522_drv_status_ok, status);
    CHECK_EQUAL(ruConf.exp_payload, ruConf.payload & ruConf.field_mask);
}

TEST(TestMfrc522DrvCommon, mfrc522_drv_read_until__NoAnswerAfterAllTries__Failure)
{
    const auto conf = initDevice();

    mfrc522_drv_read_until_conf ruConf;
    ruConf.addr = mfrc522_reg_fifo_data_reg;
    ruConf.exp_payload = 0xAA;
    ruConf.field_mask = 0xFF;
    ruConf.retry_cnt = 10;
    ruConf.delay = 1;

    const auto fakeDev = Mfrc522FakeDev::getFakeDev();
    fakeDev->addResponse(ruConf.addr, {0x00}, Mfrc522FakeDev::INFINITE_RESPONSE);

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
#include "TestRunner.h"
#include "mfrc522_drv.h"

/* ------------------------------------------------------------ */
/* ------------------------ Test groups ----------------------- */
/* ------------------------------------------------------------ */

TEST_GROUP(TestMfrc522DrvLlPtr)
{
    void setup() override {}
    void teardown() override {}
};

/* ------------------------------------------------------------ */
/* ------------------------ Test cases ------------------------ */
/* ------------------------------------------------------------ */

TEST(TestMfrc522DrvLlPtr, mfrc522_drv_init__NullCases)
{
    /* Pass null as send/receive pointers */
    mfrc522_drv_conf conf;
    conf.ll_recv = nullptr;
    conf.ll_send = nullptr;
    conf.ll_delay = nullptr;
    CHECK_EQUAL(mfrc522_drv_status_nullptr, mfrc522_drv_init(&conf));
}

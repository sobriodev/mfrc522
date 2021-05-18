#include "TestCommon.h"
#include "mfrc522_drv.h"

/* ------------------------------------------------------------ */
/* --------------------- Private functions -------------------- */
/* ------------------------------------------------------------ */

/* Dummy implementation of low-level functions */
static mfrc522_ll_status dummySend(u8 addr, size bytes, u8* payload) // NOLINT(readability-non-const-parameter)
{
    static_cast<void>(addr);
    static_cast<void>(bytes);
    static_cast<void>(payload);
    return mfrc522_ll_status_ok;
}

static mfrc522_ll_status dummyRecv(u8 addr, u8* payload) // NOLINT(readability-non-const-parameter)
{
    static_cast<void>(addr);
    static_cast<void>(payload);
    return mfrc522_ll_status_ok;
}

static void dummyDelay(u32 period)
{
    static_cast<void>(period);
}

/* ------------------------------------------------------------ */
/* ------------------------ Test cases ------------------------ */
/* ------------------------------------------------------------ */

TEST(TestMfrc522DrvLlPtr, mfrc522_drv_init__NullCases)
{
    /* Stage 1: Pass NULL as low-level send */
    mfrc522_drv_conf conf;
    conf.ll_recv = dummyRecv;
    conf.ll_send = nullptr;
    conf.ll_delay = dummyDelay;
    ASSERT_EQ(mfrc522_drv_status_nullptr, mfrc522_drv_init(&conf));

    /* Stage 2: Pass NULL as low-level receive */
    conf.ll_recv = nullptr;
    conf.ll_send = dummySend;
    conf.ll_delay = dummyDelay;
    ASSERT_EQ(mfrc522_drv_status_nullptr, mfrc522_drv_init(&conf));

    /* Stage 2: Pass NULL as low-level delay */
    conf.ll_recv = dummyRecv;
    conf.ll_send = dummySend;
    conf.ll_delay = nullptr;
    ASSERT_EQ(mfrc522_drv_status_nullptr, mfrc522_drv_init(&conf));
}

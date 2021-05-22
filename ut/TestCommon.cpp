#include "TestCommon.h"
#include "mfrc522_conf.h"

using namespace testing;

mfrc522_drv_status mfrc522_drv_init__STUB(mfrc522_drv_conf* conf)
{
    conf->chip_version = MFRC522_CONF_CHIP_TYPE;
    return mfrc522_drv_status_ok;
}

/* Dummy implementation of low-level functions needed by mock system */
mfrc522_ll_status mfrc522_ll_send(u8 addr, size bytes, u8* payload) // NOLINT(readability-non-const-parameter)
{
    static_cast<void>(addr);
    static_cast<void>(bytes);
    static_cast<void>(payload);
    return mfrc522_ll_status_ok;
}

mfrc522_ll_status mfrc522_ll_recv(u8 addr, u8* payload) // NOLINT(readability-non-const-parameter)
{
    static_cast<void>(addr);
    *payload = 0x00;
    return mfrc522_ll_status_ok;
}

void mfrc522_ll_delay(u32 period)
{
    static_cast<void>(period);
}

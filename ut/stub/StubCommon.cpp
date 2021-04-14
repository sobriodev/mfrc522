#include "StubCommon.h"
#pragma clang diagnostic push

/* ------------------------------------------------------------ */
/* --------------------- Public functions --------------------- */
/* ------------------------------------------------------------ */

mfrc522_ll_status stub_mfrc522_ll_send_always_err(u8 addr, u8 payload)
{
    (void)addr;
    (void)payload;
    return mfrc522_ll_status_send_err;
}

#pragma clang diagnostic push
#pragma ide diagnostic ignored "readability-non-const-parameter"
mfrc522_ll_status stub_mfrc522_ll_recv_always_err(u8 addr, size bytes, u8* payload)
{
    (void)addr;
    (void)bytes;
    (void)payload;
    return mfrc522_ll_status_recv_err;
}
#pragma clang diagnostic pop

mfrc522_ll_status stub_mfrc522_ll_send_always_ok(u8 addr, u8 payload)
{
    (void)addr;
    (void)payload;
    return mfrc522_ll_status_ok;
}

#pragma clang diagnostic push
#pragma ide diagnostic ignored "readability-non-const-parameter"
mfrc522_ll_status stub_mfrc522_ll_recv_always_ok(u8 addr, size bytes, u8* payload)
{
    (void)addr;
    (void)bytes;
    (void)payload;
    return mfrc522_ll_status_ok;
}
#pragma clang diagnostic pop
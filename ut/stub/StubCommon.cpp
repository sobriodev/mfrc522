#include "StubCommon.h"

/* ------------------------------------------------------------ */
/* --------------------- Public functions --------------------- */
/* ------------------------------------------------------------ */

mfrc522_ll_status stubMfrc522SendError(u8 addr, u8 payload)
{
    (void)addr;
    (void)payload;
    return mfrc522_ll_status_send_err;
}

#pragma clang diagnostic push
#pragma ide diagnostic ignored "readability-non-const-parameter"
mfrc522_ll_status stubMfrc522ReceiveError(u8 addr, size bytes, u8* payload)
{
    (void)addr;
    (void)bytes;
    (void)payload;
    return mfrc522_ll_status_recv_err;
}
#pragma clang diagnostic pop

void stubMfrc522Delay(u32 period)
{
    (void)period;
    /* Nothing to do here */
}

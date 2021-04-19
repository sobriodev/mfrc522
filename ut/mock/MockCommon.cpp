#include "TestRunner.h"
#include "mfrc522_ll.h"

mfrc522_ll_status mfrc522_ll_send(u8 addr, size bytes, u8* payload)
{
    static auto defaultRet = mfrc522_ll_status_ok;

    mock().actualCall("mfrc522_ll_send")
    .withParameter("addr", addr)
    .withParameter("bytes", bytes)
    .withMemoryBufferParameter("payload", payload, bytes);
    return *static_cast<mfrc522_ll_status*>(mock().returnPointerValueOrDefault(&defaultRet));
}

mfrc522_ll_status mfrc522_ll_recv(u8 addr, u8* payload)
{
    static auto defaultRet = mfrc522_ll_status_ok;
    mock().actualCall("mfrc522_ll_recv")
    .withParameter("addr", addr)
    .withOutputParameter("payload", payload);
    return *static_cast<mfrc522_ll_status*>(mock().returnPointerValueOrDefault(&defaultRet));
}

void mfrc522_ll_delay(u32 period)
{
    mock().actualCall("mfrc522_ll_delay")
    .withParameter("period", period);
}

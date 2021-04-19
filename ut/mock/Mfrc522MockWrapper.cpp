#include <algorithm>
#include "mock/Mfrc522MockWrapper.h"

void mfrc522UpdateLowLevelExpectations(const LowLevelCallParams& params)
{
    mock().strictOrder();
    mock().ignoreOtherCalls();

    for (const auto& llc : params) {
        if (llc.read) {
            mock().expectNCalls(llc.nTimes, "mfrc522_ll_recv")
            .withParameter("addr", llc.addr)
            .withOutputParameterReturning("payload", llc.payload, 1);
        } else {
            mock().expectNCalls(llc.nTimes, "mfrc522_ll_send")
            .withParameter("addr", llc.addr)
            .withParameter("bytes", llc.bytes)
            .withMemoryBufferParameter("payload", llc.payload, llc.bytes);
        }
    }
}

void mfrc522DestroyLowLevelParams(LowLevelCallParams& params)
{
    for (auto& param : params) {
        delete[] param.payload;
    }
    params.clear();
}
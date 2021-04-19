#include <algorithm>
#include "mock/Mfrc522MockWrapper.h"

void mfrc522UpdateLowLevelExpectations(const LowLevelCallParams& params)
{
    mock().strictOrder();
    mock().ignoreOtherCalls();

    for (const auto& llc : params) {
        auto type = std::get<0>(llc);
        auto n = std::get<1>(llc);
        auto addr = std::get<2>(llc);
        auto payload = std::get<3>(llc);

        if (type) {
            mock().expectNCalls(n, "mfrc522_ll_recv")
            .withParameter("addr", addr)
            .withParameter("bytes", 1) /* FIXME To be deleted */
            .withOutputParameterReturning("payload", payload, sizeof(*payload));
        } else {
            mock().expectNCalls(n, "mfrc522_ll_send")
            .withParameter("addr", addr)
            .withParameter("payload", *payload);
        }
    }
}

void mfrc522DestroyLowLevelParams(LowLevelCallParams& params)
{
    for (auto& param : params) {
        delete std::get<3>(param);
    }
    params.clear();
}
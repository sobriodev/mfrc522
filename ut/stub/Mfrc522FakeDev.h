#ifndef MFRC522_MFRC522FAKEDEV_H
#define MFRC522_MFRC522FAKEDEV_H

#include <utility>
#include <vector>
#include <memory>

#include "mfrc522_ll.h"

using ResponseBytes = std::vector<u8>;
using FakeResponse = std::pair<u8, ResponseBytes>;
using VectorOfFakeResponses = std::vector<FakeResponse>;

/* NOTE: The class cannot be used in parallel tests because of static shared fake device */
class Mfrc522FakeDev
{
private:
    static inline std::shared_ptr<Mfrc522FakeDev> fakeDev = nullptr;
    VectorOfFakeResponses responses;
    bool strict = true;
public:
    void addResponse(u8 address, const ResponseBytes& bytes);
    bool getNextResponse(u8 address, size bytes, u8* payload);
    void clearResponses();

    [[nodiscard]] auto isStrict() const;
    void setStrict(bool s);
    [[nodiscard]] auto countResponses() const;

    static void setupFakeDev();
    static size tearDownFakeDev();
    static std::shared_ptr<Mfrc522FakeDev> getFakeDev();

    static mfrc522_ll_status sendImpl(u8 addr, u8 payload);
    static mfrc522_ll_status recvImpl(u8 addr, size bytes, u8* payload);
};

#endif //MFRC522_MFRC522FAKEDEV_H

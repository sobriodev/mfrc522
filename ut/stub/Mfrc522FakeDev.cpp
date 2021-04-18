#include "Mfrc522FakeDev.h"

void Mfrc522FakeDev::addResponse(u8 address, const ResponseBytes& bytes, u32 numTimes)
{
    switch (numTimes) {
        case 0:
            numTimes = 1;
            break;
        case INFINITE_RESPONSE:
            setStrict(false); /* Set strict to false to avoid errors on test teardown */
            break;
        default:
            break;
    }

    responses.push_back({numTimes, {address, bytes}});
}

bool Mfrc522FakeDev::getNextResponse(u8 address, size bytes, u8* payload)
{
    /* If no more responses exist throw an error */
    if (responses.empty()) {
        return false;
    }

    auto& response = responses.at(0); /* Take by reference to modify it */
    auto& numTimes = response.first;
    auto responseAddress = std::get<0>(response.second);
    const auto responseBytes = std::get<1>(response.second);

    /* Handle multiple responses */
    if (numTimes > 1) {
        if (numTimes != INFINITE_RESPONSE) {
            --numTimes;
        }
    } else {
        responses.erase(responses.begin());
    }

    /* Throw an error if addresses do not equal or payload vector is too small/big */
    if (address != responseAddress || bytes != responseBytes.size()) {
        return false;
    }

    /* Copy payload */
    for (size i = 0; i < bytes; ++i) {
        payload[i] = responseBytes.at(i);
    }
    return true;
}

void Mfrc522FakeDev::clearResponses()
{
    responses.clear();
}

auto Mfrc522FakeDev::isStrict() const
{
    return strict;
}

void Mfrc522FakeDev::setStrict(bool s)
{
    strict = s;
}

auto Mfrc522FakeDev::countResponses() const
{
    return responses.size();
}

void Mfrc522FakeDev::setupFakeDev()
{
    fakeDev = std::make_shared<Mfrc522FakeDev>();
}

size Mfrc522FakeDev::tearDownFakeDev()
{
    size responses = 0;
    if (fakeDev->isStrict()) {
        responses = fakeDev->countResponses();
    }
    fakeDev.reset();
    return responses;
}

std::shared_ptr<Mfrc522FakeDev> Mfrc522FakeDev::getFakeDev()
{
    return fakeDev;
}

mfrc522_ll_status Mfrc522FakeDev::sendImpl(u8 addr, u8 payload)
{
    (void)addr;
    (void)payload;

    /* Nothing to do here - just return status code */
    return mfrc522_ll_status_ok;
}

mfrc522_ll_status Mfrc522FakeDev::recvImpl(u8 addr, size bytes, u8* payload) {
    /* If device was not assigned throw an error immediately */
    if (!fakeDev) {
        return mfrc522_ll_status_recv_err;
    }
    return (fakeDev->getNextResponse(addr, bytes, payload)) ? mfrc522_ll_status_ok : mfrc522_ll_status_recv_err;
}
#ifndef MFRC522_MFRC522MOCKWRAPPER_H
#define MFRC522_MFRC522MOCKWRAPPER_H

#include <tuple>
#include <vector>
#include "TestRunner.h"
#include "type.h"

#define LL_READ(N, ADDR, PAYLOAD) {true, (N), (ADDR), new u8((PAYLOAD))}
#define LL_WRITE(N, ADDR, PAYLOAD) {false, (N), (ADDR), new u8((PAYLOAD))}

using LowLevelCallParams = std::vector<std::tuple<bool, u32, u8, u8*>>;

void mfrc522UpdateLowLevelExpectations(const LowLevelCallParams& params);

/* The function has to be called after all operations */
void mfrc522DestroyLowLevelParams(LowLevelCallParams& params);

#endif //MFRC522_MFRC522MOCKWRAPPER_H

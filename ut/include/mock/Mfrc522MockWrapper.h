#ifndef MFRC522_MFRC522MOCKWRAPPER_H
#define MFRC522_MFRC522MOCKWRAPPER_H

#include <tuple>
#include <vector>
#include "TestRunner.h"
#include "mfrc522_reg.h"
#include "type.h"

/**
 * Helper macro to create read low-level mocked call
 */
#define READ(N, ADDR, PAYLOAD) {true, (N), (ADDR), 1, new u8[1] {(PAYLOAD)}, false}

/**
 * Helper macro to create single-byte write low-level mocked call
 */
#define WRITE1(N, ADDR, PAYLOAD) {false, (N), (ADDR), 1, new u8[1] {(PAYLOAD)}, false}

/**
 * Helper macro to create multi-byte write low-level mocked call
 */
#define WRITE_N(N, ADDR, SZ, PAYLOAD) {false, (N), (ADDR), (SZ), new u8[(SZ)] PAYLOAD, false}

/**
 * Helper macro to create read low-level mocked call that always returns 0x00 as a payload
 */
#define READ_A(N, ADDR) {true, (N), (ADDR), 1, new u8[1] {0x00}, true}

/**
 * Helper macro to create single-byte write low-level mocked call that checks only address passed to a function.
 */
#define WRITE1_A(N, ADDR) {false, (N), (ADDR), 0, nullptr, true}

/**
 * Low-level call parameters
 */
struct LowLevelCallParam
{
    bool read; /**< True for read operation, false for write */
    size nTimes; /**< The number of times mocked function is expected to be called */
    mfrc522_reg addr; /**< Register address */
    size bytes; /**< Valid only for low-level write. Number of bytes passed as a payload */
    u8* payload; /**< For read-call address of a buffer where output is stored, for write-call payload array */
    bool checkOnlyAddr; /**< If enabled, only register address is compared, thus 'bytes' and 'payload' fields are
                             ignored. Low-level read call always returns 0x00 as a payload */
};

using LowLevelCallParams = std::vector<LowLevelCallParam>;

/**
 * Update mock expected calls.
 *
 * @param params Call parameters.
 */
void mfrc522UpdateLowLevelExpectations(const LowLevelCallParams& params);

/**
 * Destroy parameters vector.
 *
 * @param params Call parameters.
 */
void mfrc522DestroyLowLevelParams(LowLevelCallParams& params);

#endif //MFRC522_MFRC522MOCKWRAPPER_H

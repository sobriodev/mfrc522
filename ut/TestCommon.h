#ifndef MFRC522_TESTCOMMON_H
#define MFRC522_TESTCOMMON_H

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wreorder" /* Disable reorder flag due to problem inside Cutie library */
#include "mock.hpp"
#include "hook.hpp"
#pragma GCC diagnostic pop

#include "mfrc522_drv.h"

/* ------------------------------------------------------------ */
/* ---------------------------- Macros ------------------------ */
/* ------------------------------------------------------------ */

/* Install low-level mocks */
#define INSTALL_LL_MOCKS() \
INSTALL_MOCK(mfrc522_ll_send); \
INSTALL_MOCK(mfrc522_ll_recv); \
INSTALL_MOCK(mfrc522_ll_delay) /* Do not put semicolon here, hence require it in test codes */

/* Macro to ignore excess low-level calls. Must be called after mocks have been installed */
#define IGNORE_REDUNDANT_LL_CALLS() \
do { \
    CUTIE_EXPECT_CALL(mfrc522_ll_recv, _, _).Times(AnyNumber()) \
        .WillRepeatedly(DoAll(SetArgPointee<1>(0x00), Return(mfrc522_ll_status_ok))); \
    CUTIE_EXPECT_CALL(mfrc522_ll_send, _, _, _).Times(AnyNumber()) \
        .WillRepeatedly(Return(mfrc522_ll_status_ok)); \
    CUTIE_EXPECT_CALL(mfrc522_ll_delay, _).Times(AnyNumber()); \
} while (0)

/* For convenience only */
#define MOCK_STRICT_ORDER() InSequence s

/* Macro to initialize mock subsystem */
#define MOCK_COMMON_INIT() \
INSTALL_LL_MOCKS(); \
IGNORE_REDUNDANT_LL_CALLS(); \
MOCK_STRICT_ORDER() /* Do not put semicolon here, hence require it in test codes */

/* ------------------------------------------------------------ */
/* ----------------------- Public functions ------------------- */
/* ------------------------------------------------------------ */

/* Stub for init function */
mfrc522_drv_status mfrc522_drv_init__STUB(mfrc522_drv_conf* conf); // NOLINT(bugprone-reserved-identifier)

#endif //MFRC522_TESTCOMMON_H

#ifndef MFRC522_TESTCOMMON_H
#define MFRC522_TESTCOMMON_H

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wreorder" /* Disable reorder flag due to problem inside Cutie library */
#include "mock.hpp"
#include "hook.hpp"
#pragma GCC diagnostic pop

#include "mfrc522_drv.h"

/* ------------------------------------------------------------ */
/* ----------------------- Public functions ------------------- */
/* ------------------------------------------------------------ */

/* Stub for init function */
mfrc522_drv_status mfrc522_drv_init__STUB(mfrc522_drv_conf* conf); // NOLINT(bugprone-reserved-identifier)

#endif //MFRC522_TESTCOMMON_H

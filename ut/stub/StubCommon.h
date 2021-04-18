#ifndef MFRC522_STUBCOMMON_H
#define MFRC522_STUBCOMMON_H

#include "mfrc522_ll.h"

/* ------------------------------------------------------------ */
/* --------------------- Public functions --------------------- */
/* ------------------------------------------------------------ */

/* Stub low-level definitions that return send/receive errors */
mfrc522_ll_status stubMfrc522SendError(u8 addr, u8 payload);
mfrc522_ll_status stubMfrc522ReceiveError(u8 addr, size bytes, u8* payload);

/* Stub low-level delay function */
void stubMfrc522Delay(u32 period);

#endif //MFRC522_STUBCOMMON_H

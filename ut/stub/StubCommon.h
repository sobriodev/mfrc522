#ifndef MFRC522_STUBCOMMON_H
#define MFRC522_STUBCOMMON_H

#include "mfrc522_ll.h"

/* ------------------------------------------------------------ */
/* --------------------- Public functions --------------------- */
/* ------------------------------------------------------------ */

/* Stub low-level definitions that return send/receive errors */
mfrc522_ll_status stub_mfrc522_ll_send_always_err(u8 addr, u8 payload);
mfrc522_ll_status stub_mfrc522_ll_recv_always_err(u8 addr, size bytes, u8* payload);

/* Stub low-level definitions that always return ok codes */
mfrc522_ll_status stub_mfrc522_ll_send_always_ok(u8 addr, u8 payload);
mfrc522_ll_status stub_mfrc522_ll_recv_always_ok(u8 addr, size bytes, u8* payload);

#endif //MFRC522_STUBCOMMON_H

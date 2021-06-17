#ifndef MFRC522_TESTCOMMON_H
#define MFRC522_TESTCOMMON_H

#include "mfrc522_picc.h"

/* ------------------------------------------------------------ */
/* ----------------------- Public functions ------------------- */
/* ------------------------------------------------------------ */

/* PICC verification function that accepts any ATQA */
bool piccAcceptAny(u16 atqa);

/* PICC verification function that accepts only MF1S503yX card (ATQA = 0x0004) */
bool piccAccept0x0004(u16 atqa);

#endif //MFRC522_TESTCOMMON_H

#ifndef MFRC522_MOCKABLE_H
#define MFRC522_MOCKABLE_H

#include "mfrc522_drv.h"

/*
 * Caution!
 * DECLARE_MOCKABLE() and DECLARE_HOOKABLE() macros mix declarations and definitions internally.
 * Thus it would be reasonably to keep calls to the macros inside source file.
 * Keeping them inside header file just for convenience.
 * Do not include the file in more than one source file or 'multiple definitions' error will be definitely reported!
 */

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpedantic" /* Disable pedantic flag due to problem inside Cutie library */

/* Mocks */
DECLARE_MOCKABLE(mfrc522_drv_init, 1);
DECLARE_MOCKABLE(mfrc522_ll_send, 3);
DECLARE_MOCKABLE(mfrc522_ll_recv, 2);
DECLARE_MOCKABLE(mfrc522_ll_delay, 1);

/* Stubs */
DECLARE_HOOKABLE(mfrc522_drv_init);

#pragma GCC diagnostic pop

#endif //MFRC522_MOCKABLE_H

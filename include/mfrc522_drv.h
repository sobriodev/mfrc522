#ifndef MFRC522_MFRC522_DRV_H
#define MFRC522_MFRC522_DRV_H

#include "type.h"
#include "mfrc522_ll.h"

/* ------------------------------------------------------------ */
/* ---------------------------- Macros ------------------------ */
/* ------------------------------------------------------------ */

/** Major version */
#define MFRC522_DRV_API_VERSION_MAJOR 0
/** Minor version */
#define MFRC522_DRV_API_VERSION_MINOR 0
/** Revision version */
#define MFRC522_DRV_VERSION_REVISION 0

/* ------------------------------------------------------------ */
/* -------------------------- Data types ---------------------- */
/* ------------------------------------------------------------ */

/**
 * Status codes used by API functions
 */
typedef enum mfrc522_drv_status_
{
    mfrc522_drv_status_ok = 0, /**< Success */
    mfrc522_drv_status_nok, /**< Generic error code */
    mfrc522_drv_status_nullptr, /**< Unexpected NULL pointer */
    mfrc522_drv_status_ll_err, /**< An error occurred during low-level call */

    /* Bus errors */
    mfrc522_drv_status_bus_err /**< Device not found */
} mfrc522_drv_status;

/**
 * MFRC522 configuration values
 */
typedef struct mfrc522_drv_conf_
{
#if MFRC522_LL_PTR
    mfrc522_ll_send ll_send; /**< Low-level send function pointer */
    mfrc522_ll_recv ll_recv; /**< Low-level receive function pointer */
#endif
    /* Read-only fields. Do not modify manually */
    u8 chip_version; /**< [Read-only] Chip version number */
} mfrc522_drv_conf;

#ifdef __cplusplus
extern "C" {
#endif

/* ------------------------------------------------------------ */
/* ----------------------- Public functions ------------------- */
/* ------------------------------------------------------------ */

/* todo add doxygen */
mfrc522_drv_status mfrc522_drv_init(mfrc522_drv_conf* conf);

#ifdef __cplusplus
}
#endif

#endif //MFRC522_MFRC522_DRV_H

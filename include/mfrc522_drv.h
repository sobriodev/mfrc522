#ifndef MFRC522_MFRC522_DRV_H
#define MFRC522_MFRC522_DRV_H

#include "type.h"
#include "mfrc522_ll.h"

#ifdef __cplusplus
extern "C" {
#endif

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
    mfrc522_drv_status_dev_err /**< Device not found */
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

/* ------------------------------------------------------------ */
/* ----------------------- Public functions ------------------- */
/* ------------------------------------------------------------ */

/**
 * Initialize MFRC522 device.
 *
 * The function has to be called prior to other API calls. It performs all initialization steps and check if device is
 * available on the bus. On success 'chip_version' in the conf struct contains chip version returned by
 * a device. The function returns 'ok' status code when 9xh value is read from a device, indicating that MFRC522 chip
 * type was returned which is supported by the library.
 *
 * After receiving an error from low-level receive function, 'chip_version' field is set to MFRC522_REG_VERSION_INVALID.
 *
 * In a case 'pointer' low-level calls are used, 'll_send and 'll_recv' pointers need to be set in a configuration
 * structure. Otherwise an error will be returned.
 *
 * @param conf Pointer to a configuration structure. Mandatory fields have to be set prior to call to this function.
 * @return Status of the operation. Valid responses are:
 *         - mfrc522_drv_status_nullptr when NULL was passed instead of a valid pointer
 *         - mfrc522_drv_status_dev_err when valid device was not found
 *         - mfrc522_drv_status_ok on success
 */
mfrc522_drv_status mfrc522_drv_init(mfrc522_drv_conf* conf);

#ifdef __cplusplus
}
#endif

#endif //MFRC522_MFRC522_DRV_H

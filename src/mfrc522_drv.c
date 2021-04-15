#include "mfrc522_drv.h"
#include "common.h"
#include "mfrc522_reg.h"

/* ------------------------------------------------------------ */
/* ------------------------ Private macros -------------------- */
/* ------------------------------------------------------------ */

/* Helper macros to choose between 'pointer' and 'definition' low-level calls automatically */
#if MFRC522_LL_PTR
#define SEND(CONF, ...) ((CONF)->ll_send)(__VA_ARGS__)
#define RECV(CONF, ...) ((CONF)->ll_recv)(__VA_ARGS__)
#elif MFRC522_LL_DEF
#define SEND(CONF, ...) mfrc522_ll_send(__VA_ARGS__)
#define RECV(CONF, ...) mfrc522_ll_recv(__VA_ARGS__)
#endif

/* Try to send bytes to device with returning a generic error code if necessary */
#define TRY_SEND(...)  \
do { \
    if (UNLIKELY(mfrc522_ll_status_ok != SEND(__VA_ARGS__))) return mfrc522_drv_status_ll_err; \
} while (0)

/* Try to receive bytes from device with returning a generic error code if necessary */
#define TRY_RECV(...) \
do { \
    if (UNLIKELY(mfrc522_ll_status_ok != RECV(__VA_ARGS__))) return mfrc522_drv_status_ll_err; \
} while (0)

/* ------------------------------------------------------------ */
/* ----------------------- Public functions ------------------- */
/* ------------------------------------------------------------ */

mfrc522_drv_status mfrc522_drv_init(mfrc522_drv_conf* conf)
{
    ERROR_IF(conf, NULL, mfrc522_drv_status_nullptr);

    /* In case 'pointer' low-level calls are used check for NULL also */
#if MFRC522_LL_PTR
    ERROR_IF(conf->ll_send, NULL, mfrc522_drv_status_nullptr);
    ERROR_IF(conf->ll_send, NULL, mfrc522_drv_status_nullptr);
#endif

    /* Try to get chip version from a device */
    conf->chip_version = MFRC522_REG_VERSION_INVALID;
    TRY_RECV(conf, mfrc522_reg_version, 1, &conf->chip_version);

    /* Chiptype 9xh stands for MFRC522 */
    if ((conf->chip_version & MFRC522_REG_VERSION_CHIP_TYPE_MSK) != 0x90) {
        return mfrc522_drv_status_dev_err;
    }
    return mfrc522_drv_status_ok;
}


#include "mfrc522_drv.h"
#include "common.h"

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

    /* TODO replace with specific API call when ready */
    conf->chip_version = 0xFF;
    mfrc522_ll_status status = RECV(conf, 0x37, 1, &conf->chip_version);
    if (UNLIKELY(mfrc522_ll_status_ok != status)) {
        return mfrc522_drv_status_ll_err;
    }
    if ((conf->chip_version >> 4) != 0x09) {
        return mfrc522_drv_status_bus_err;
    }
    return mfrc522_drv_status_ok;
}


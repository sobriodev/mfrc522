#ifndef MFRC522_MFRC522_DRV_H
#define MFRC522_MFRC522_DRV_H

#include "type.h"
#include "common.h"
#include "mfrc522_ll.h"
#include "mfrc522_reg.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ------------------------------------------------------------ */
/* ---------------------------- Macros ------------------------ */
/* ------------------------------------------------------------ */

/**
 * Infinity flag recognized by 'mfrc522_drv_read_until_conf'
 */
#define MFRC522_DRV_RETRY_CNT_INF 0xFFFFFFFF

/**
 * Default number of retries when condition is not fulfilled
 */
#define MFRC522_DRV_DEF_RETRY_CNT 10

/**
 * Maximum timer period in milliseconds
 */
#define MFRC522_DRV_TIM_MAX_PERIOD 16383

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

    /* Device errors */
    mfrc522_drv_status_dev_err, /**< Device not found */
    mfrc522_drv_status_dev_rtr_err, /**< Maximum number of read retries reached */

    /* Timer related */
    mfrc522_drv_status_tim_prd_err /**< Given timer period is too long */
} mfrc522_drv_status;

/**
 * MFRC522 configuration values
 */
typedef struct mfrc522_drv_conf_
{
#if MFRC522_LL_PTR
    mfrc522_ll_send ll_send; /**< Low-level send function pointer */
    mfrc522_ll_recv ll_recv; /**< Low-level receive function pointer */
#if MFRC522_LL_DELAY
    mfrc522_ll_delay ll_delay; /**< Low-level delay function pointer */
#endif
#endif
    /* Read-only fields. Do not modify manually */
    u8 chip_version; /**< Chip version number */
} mfrc522_drv_conf;

/**
 * Configuration structure used by 'mfrc522_drv_read_until()' function
 */
typedef struct mfrc522_drv_read_until_conf_
{
    mfrc522_reg addr; /**< Register address to read from */
    u8 payload; /**< Payload buffer to write to */
    u8 mask; /**< Field bitmask. Set to 0xFF if all bits should be compared */
    u8 exp_payload; /**< Expected payload. Used as the exit criterion. */
    u32 delay; /**< Optional delay in microseconds between reads. Valid only if MFRC522_LL_DELAY is set */
    u32 retry_cnt; /**< Maximum number of retries */
} mfrc522_drv_read_until_conf;

/**
 * Prescaler types for timer unit
 */
typedef enum mfrc522_drv_tim_psl_type_
{
    mfrc522_drv_tim_psl_odd, /**< Odd prescaler */
    mfrc522_drv_tim_psl_even /**< Even prescaler */
} mfrc522_drv_tim_psl_type;

/**
 * Timer configuration structure
 */
typedef struct mfrc522_drv_tim_conf_
{
    u16 prescaler; /**< Prescaler value. Only 12 lower bits are used */
    mfrc522_drv_tim_psl_type prescaler_type; /**< Prescaler type */
    u16 reload_val; /**< Reload value */
    bool periodic; /**< Periodicity flag */
} mfrc522_drv_tim_conf;

/**
 * Interrupt request system settings
 */
typedef struct mfrc522_drv_irq_conf_
{
    bool irq_signal_inv; /**< When true, signal on IRQ pin is inverted with respect to IRQ bit */
    bool irq_push_pull; /**< When true, IRQ pin is a CMOS output pin. When false, IRQ pin is open-drain output pin */
} mfrc522_drv_irq_conf;

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
 * structure. Otherwise an error is returned.
 *
 * @param conf Pointer to a configuration structure. Mandatory fields have to be set prior to call to this function.
 * @return Status of the operation. Valid responses are:
 *         - mfrc522_drv_status_nullptr when NULL is passed instead of a valid pointer
 *         - mfrc522_drv_status_dev_err when valid device is not found
 *         - mfrc522_drv_status_ok on success
 */
mfrc522_drv_status mfrc522_drv_init(mfrc522_drv_conf* conf);

/**
 * Write multiple bytes to a PCD's register.
 *
 * The function performs write to a PCD (Proximity Coupling Device). Depending on low-level call mechanism, 'pointer'
 * or 'definition' method is used.
 *
 * Note that depending on compiler used, the function may be in fact not inlined.
 * In a case when either 'conf' is NULL, mfrc522_ll_status_send_err is returned.
 *
 * @param conf Pointer to a configuration structure.
 * @param addr Register address.
 * @param sz Number of payload bytes.
 * @param payload Payload bytes.
 * @return An instance of mfrc522_ll_status.
 */
static inline mfrc522_ll_status mfrc522_drv_write(const mfrc522_drv_conf* conf, mfrc522_reg addr, size sz, u8* payload)
{
    ERROR_IF_EQ(conf, NULL, mfrc522_ll_status_send_err);
#if MFRC522_LL_PTR
    return conf->ll_send(addr, sz, payload);
#elif MFRC522_LL_DEF
    (void)conf; /* Make compiler happy */
    return mfrc522_ll_send(addr, sz, payload);
#endif
}

/**
 * Write single byte to a PCD's register.
 *
 * The function is a wrapper on 'mfrc522_drv_write()' function.
 *
 * @param conf Pointer to a configuration structure.
 * @param addr Register address.
 * @param payload Payload byte.
 * @return An instance of mfrc522_ll_status. Refer to 'mfrc522_drv_write()' function for more details.
 */
static inline mfrc522_ll_status mfrc522_drv_write_byte(const mfrc522_drv_conf* conf, mfrc522_reg addr, u8 payload)
{
    return mfrc522_drv_write(conf, addr, 1, &payload);
}

/**
 * Perform masked write to a PCD's register.
 *
 * The function sets new value of a register regarding value, position and mask. It is useful in cases when a single
 * register contains multiple fields and only one of them should be changed. Only bits selected in mask are affected.
 * In a case when either 'conf' is NULL, mfrc522_ll_status_send_err is returned.
 *
 * @param conf Pointer to a configuration structure.
 * @param addr Register address.
 * @param val Field bits.
 * @param mask Field mask.
 * @param pos Field position in a register.
 * @return An instance of mfrc522_ll_status.
 */
mfrc522_ll_status mfrc522_drv_write_masked(const mfrc522_drv_conf* conf, mfrc522_reg addr, u8 val, u8 mask, u8 pos);

/**
 * Read from a PCD.
 *
 * The function performs read from a PCD (Proximity Coupling Device). Depending on low-level call mechanism, 'pointer'
 * or 'definition' method is used.
 *
 * Note that depending on compiler used, the function may be in fact not inlined.
 * In a case when either 'conf' or 'payload' is NULL, mfrc522_ll_status_recv_err is returned.
 *
 * @param conf Pointer to a configuration structure.
 * @param addr Register address.
 * @param payload Pointer to a buffer where incoming bytes are written. Must be big enough to store all data.
 * @return An instance of mfrc522_ll_status.
 */
static inline mfrc522_ll_status mfrc522_drv_read(const mfrc522_drv_conf* conf, mfrc522_reg addr, u8* payload)
{
    ERROR_IF_EQ(conf, NULL, mfrc522_ll_status_recv_err);
    ERROR_IF_EQ(payload, NULL, mfrc522_ll_status_recv_err);
#if MFRC522_LL_PTR
    return conf->ll_recv(addr, payload);
#elif MFRC522_LL_DEF
    (void)conf; /* Make compiler happy */
    return mfrc522_ll_recv(addr, payload);
#endif
}

/**
 * Poll a PCD's register until certain value is returned.
 *
 * The function keeps reading from a register until requested value is returned.
 * It is possible to pass maximum number of retries, after which an error code is returned or force 'infinite' version
 * by setting 'retry_cnt' field to MFRC522_DRV_RETRY_CNT_INF. The real number of retries may vary, depending on
 * MFRC522_CONF_RETRY_CNT_MUL configuration macro, when MFRC522_LL_DELAY macro is not enabled.
 *
 * After the operation, recent payload is kept in 'payload' field. Its validity depends on status code returned.
 * The function does nothing when either 'conf' or 'ru_conf' parameter is NULL.
 *
 * @param conf Pointer to a configuration structure.
 * @param ru_conf Pointer to a read settings structure.
 * @return Status code of the operation. Valid responses are:
 *         - mfrc522_drv_status_nullptr if either 'conf' or 'ru_conf' parameter is NULL
 *         - mfrc522_drv_status_ll_err when low-level call failed
 *         - mfrc522_drv_status_dev_rtr_err when maximum number of retries was reached
 *         - mfrc522_drv_status_ok on success
 */
mfrc522_drv_status mfrc522_drv_read_until(const mfrc522_drv_conf* conf, mfrc522_drv_read_until_conf* ru_conf);

/**
 * Perform soft reset on a PCD.
 *
 * The function waits until no command is executed.
 * An error code is returned when NULL was passed instead of a valid 'conf' pointer.
 *
 * @param conf Pointer to a configuration structure.
 * @return An instance of 'mfrc522_drv_status'. On success, mfrc522_drv_status_ok is returned.
 */
mfrc522_drv_status mfrc522_drv_soft_reset(const mfrc522_drv_conf* conf);

/**
 * Initialize MFRC522 timer.
 *
 * The function performs initialization of 'tim_conf' fields according to 'period' parameter.
 * Minimum timer period equals to 1ms.
 * Maximum timer period is limited by MFRC522_DRV_TIM_MAX_PERIOD macro.
 * Note that the function does not modify all fields, e.g. periodicity flag has to be set manually.
 *
 * The function returns error when NUll was passed instead of a 'tim_conf' pointer.
 *
 * @param tim_conf Pointer to a timer configuration structure, where calculated values are stored.
 * @param period Timer period in milliseconds.
 * @return Status of the operation:
 *         - mfrc522_drv_status_nullptr when NULL was passed instead of a valid pointer
 *         - mfrc522_drv_status_tim_prd_err when timer period was incorrect
 *         - mfrc522_drv_status_ok on success
 */
mfrc522_drv_status mfrc522_drv_tim_set(mfrc522_drv_tim_conf* tim_conf, u16 period);

/**
 * Start MFRC522 timer.
 *
 * The configuration structure passed as 'tim_conf' parameter has to be initialized prior to calling this function.
 * The function returns error code when NULL was passed instead of a valid pointer.
 *
 * @param conf Pointer to a MFRC522 configuration structure.
 * @param tim_conf Pointer to a timer configuration structure.
 * @return Status of the operation. On success mfrc522_drv_status_ok is returned.
 */
mfrc522_drv_status mfrc522_drv_tim_start(const mfrc522_drv_conf* conf, const mfrc522_drv_tim_conf* tim_conf);

/**
 * Initialize interrupt subsystem.
 *
 * The function sets up all registers and clear all interrupt flags.
 * An error is returned if either 'conf' or 'irq_conf' parameter is null.
 *
 * @param conf Pointer to a configuration structure.
 * @param irq_conf Pointer to an interrupt configuration structure.
 * @return Status of the operation. On success mfrc522_drv_status_ok is returned.
 */
mfrc522_drv_status mfrc522_drv_irq_init(const mfrc522_drv_conf* conf, const mfrc522_drv_irq_conf* irq_conf);

/**
 * Clear interrupt request.
 *
 * The function may be used to clear specific interrupt. It is also possible to clear all available interrupt
 * requests by passing 'mfrc522_reg_irq_all' enum constant.
 * The function returns error when NUll was passed instead of a 'conf' pointer.
 *
 * @param conf Pointer to a configuration structure.
 * @param irq Interrupt source to be cleared.
 * @return Status of the operation. On success mfrc522_drv_status_ok is returned.
 */
mfrc522_drv_status mfrc522_drv_irq_clr(const mfrc522_drv_conf* conf, mfrc522_reg_irq irq);

/**
 * Enable or disable interrupt.
 *
 * The function allows specific interrupt to be propagated to IRQ pin.
 * Note that in opposition to function 'mfrc522_drv_irq_clr()', it is not possible to enable or disable all
 * interrupts in a single call. Passing 'mfrc522_reg_irq_all' throws 'mfrc522_drv_status_nok' error code.
 *
 * The function returns error when NUll was passed instead of a 'conf' pointer.
 *
 * @param conf Pointer to a configuration structure.
 * @param irq Interrupt number.
 * @param enable True if interrupt should be enabled, false if disabled.
 * @return Status of the operation. On success mfrc522_drv_status_ok is returned.
 */
mfrc522_drv_status mfrc522_drv_irq_en(const mfrc522_drv_conf* conf, mfrc522_reg_irq irq, bool enable);

#ifdef __cplusplus
}
#endif

#endif //MFRC522_MFRC522_DRV_H

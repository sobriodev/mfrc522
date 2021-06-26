#ifndef MFRC522_MFRC522_DRV_H
#define MFRC522_MFRC522_DRV_H

#include "type.h"
#include "common.h"
#include "mfrc522_ll.h"
#include "mfrc522_reg.h"
#include "mfrc522_picc.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ------------------------------------------------------------ */
/* ---------------------------- Macros ------------------------ */
/* ------------------------------------------------------------ */

/**
 * Magic number regarded as an unique scope number that identifies this module
 */
#ifdef SCOPE_MAGIC
#undef SCOPE_MAGIC
#endif
#define SCOPE_MAGIC 0xC0B0

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

/**
 * The number of bytes returned in self test mode
 */
#define MFRC522_DRV_SELF_TEST_FIFO_SZ 64

/**
 * Total number of bytes returned by 'Random' command
 */
#define MFRC522_DRV_RAND_TOTAL 25

/**
 * The number of random bytes returned by 'Random' command
 */
#define MFRC522_DRV_RAND_BYTES 10

/* ------------------------------------------------------------ */
/* -------------------------- Data types ---------------------- */
/* ------------------------------------------------------------ */

/**
 * Status codes used by API functions
 */
typedef enum mfrc522_drv_status_
{
    /**<
     * Status code returned when operation succeeded
     */
    mfrc522_drv_status_ok = MAKE_STATUS(0x00, status_severity_none),
    /**<
     * Generic status regarded as non-critical error
     */
    mfrc522_drv_status_nok = MAKE_STATUS(0x01, status_severity_non_critical),
    /**<
     * Unexpected NULL pointer
     */
    mfrc522_drv_status_nullptr = MAKE_STATUS(0x02, status_severity_fatal),
    /**<
     * An error occurred during low-level call.
     */
    mfrc522_drv_status_ll_err = MAKE_STATUS(0x03, status_severity_fatal),
    /**<
     * Mismatch between expected and actual FIFO output during device self-test
     */
    mfrc522_drv_status_self_test_err = MAKE_STATUS(0x04, status_severity_critical),
    /**<
     * Device not found
     */
    mfrc522_drv_status_dev_err = MAKE_STATUS(0x05, status_severity_critical),
    /**
     * Maximum number of read retries reached
     */
    mfrc522_drv_status_dev_rtr_err = MAKE_STATUS(0x06, status_severity_critical),
    /**<
     * Given timer period is too long
     */
    mfrc522_drv_status_tim_prd_err = MAKE_STATUS(0x07, status_severity_non_critical),
    /**<
     * Timeout during transceive command
     */
    mfrc522_drv_status_transceive_timeout = MAKE_STATUS(0x08, status_severity_critical),
    /**<
     * At least one error present after transceive command
     */
    mfrc522_drv_status_transceive_err = MAKE_STATUS(0x09, status_severity_critical),
    /**<
     * A mismatch between expected and actual RX data size
     */
    mfrc522_drv_status_transceive_rx_mism = MAKE_STATUS(0x0A, status_severity_critical),
    /**<
     * Unknown ATQA returned after REQA command
     */
    mfrc522_drv_status_picc_vrf_err = MAKE_STATUS(0x0B, status_severity_non_critical),
    /**<
     * Checksum error during anticollision
     */
    mfrc522_drv_status_anticoll_chksum_err = MAKE_STATUS(0x0C, status_severity_critical),
    /**<
     * Computed CRC does not match up with expected one
     */
    mfrc522_drv_status_crc_err = MAKE_STATUS(0x0D, status_severity_critical),
    /**<
     * Invalid state of the crypto unit
     */
    mfrc522_drv_status_crypto_err = MAKE_STATUS(0x0E, status_severity_critical),
    /**<
     * An error when halting a PICC
     */
    mfrc522_drv_status_halt_err = MAKE_STATUS(0x0F, status_severity_critical)
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
    mfrc522_picc_atqa_verify_fn atqa_verify_fn; /**< Pointer to optional ATQA verification function. Can be NULL */
    /* Read-only fields. Do not modify manually */
    u8 chip_version; /**< Chip version number */
    u8 self_test_out[MFRC522_DRV_SELF_TEST_FIFO_SZ]; /**< Self test bytes */
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

/**
 * CRC coprocessor preset values
 */
typedef enum mfrc522_drv_crc_preset_
{
    mfrc522_drv_crc_preset_0000 = 0x00, /**< 0x0000 */
    mfrc522_drv_crc_preset_6363 = 0x01, /**< 0x6363 */
    mfrc522_drv_crc_preset_A671 = 0x02, /**< 0xA671 */
    mfrc522_drv_crc_preset_FFFF = 0x03 /**< 0xFFFF */
} mfrc522_drv_crc_preset;

/**
 * CRC coprocessor settings
 */
typedef struct mfrc522_drv_crc_conf_
{
    mfrc522_drv_crc_preset preset; /**< Preset value */
    bool msb_first; /**< If true, the CRC unit computes CRC with MSB first */
} mfrc522_drv_crc_conf;

/**
 * Configuration of contactless UART/analog interface
 */
typedef struct mfrc522_drv_ext_itf_conf_
{
    u8 dummy; /**< Dummy byte. Nothing to put here at the moment */
} mfrc522_drv_ext_itf_conf;

/**
 * Configuration used with transceive command
 */
typedef struct mfrc522_drv_transceive_conf_
{
    u8* tx_data; /**< TX data */
    size tx_data_sz; /**< Size of TX data in bytes */
    u8* rx_data; /**< Pointer where RX data will be stored. vMust be large enough to contain the number of 'rx_data_sz'
                      bytes. Can be NULL when 'rx_data_sz' equals to zero */
    size rx_data_sz; /**< Expected number of RX bytes. Can be zero (no data is expected on RX side) */
    mfrc522_reg_cmd command; /**< Command used to transceive the data.
                                  Valid ones are 'mfrc522_reg_cmd_transceive' and 'mfrc522_reg_cmd_authent' */
} mfrc522_drv_transceive_conf;

/**
 * Authentication parameters
 */
typedef struct mfrc522_drv_auth_conf_
{
    u8* serial; /**< Serial number got during anticollision procedure */
    mfrc522_picc_sector sector; /**< Sector number */
    mfrc522_picc_block block; /**< Block number */
    mfrc522_picc_key key_type; /**< Key type */
    u8* key; /**< Either Key A or Key B depending on 'key_type' setting */
} mfrc522_drv_auth_conf;

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
mfrc522_drv_status
mfrc522_drv_init(mfrc522_drv_conf* conf);

/**
 * Write multiple bytes to a PCD's register.
 *
 * The function performs write to a PCD (Proximity Coupling Device). Depending on low-level call mechanism, 'pointer'
 * or 'definition' method is used.
 *
 * Note that depending on compiler used, the function may be in fact not inlined.
 * In a case when either 'conf' or 'payload' is NULL, mfrc522_drv_status_nullptr is returned.
 *
 * @param conf Pointer to a configuration structure.
 * @param addr Register address.
 * @param sz Number of payload bytes.
 * @param payload Payload bytes.
 * @return An instance of mfrc522_drv_status. On success mfrc522_drv_ok is returned.
 */
static inline mfrc522_drv_status
mfrc522_drv_write(const mfrc522_drv_conf* conf, mfrc522_reg addr, size sz, const u8* payload)
{
    ERROR_IF_EQ(conf, NULL, mfrc522_drv_status_nullptr);
    ERROR_IF_EQ(conf, NULL, mfrc522_drv_status_nullptr);
#if MFRC522_LL_PTR
    return (mfrc522_ll_status_ok == conf->ll_send(addr, sz, payload)) ? mfrc522_drv_status_ok
                                                                      : mfrc522_drv_status_ll_err;
#elif MFRC522_LL_DEF
    (void)conf; /* Make compiler happy */
    return (mfrc522_ll_status_ok == mfrc522_ll_send(addr, sz, payload)) ? mfrc522_drv_status_ok
                                                                        : mfrc522_drv_status_ll_err;
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
 * @return An instance of mfrc522_drv_status. Refer to 'mfrc522_drv_write()' function for more details.
 */
static inline mfrc522_drv_status
mfrc522_drv_write_byte(const mfrc522_drv_conf* conf, mfrc522_reg addr, u8 payload)
{
    return mfrc522_drv_write(conf, addr, 1, &payload);
}

/**
 * Perform masked write to a PCD's register.
 *
 * The function sets new value of a register regarding value, position and mask. It is useful in cases when a single
 * register contains multiple fields and only one of them should be changed. Only bits selected in mask are affected.
 * In a case when 'conf' is NULL, mfrc522_drv_status_nullptr is returned.
 *
 * @param conf Pointer to a configuration structure.
 * @param addr Register address.
 * @param val Field bits.
 * @param mask Field mask.
 * @param pos Field position in a register.
 * @return An instance of mfrc522_drv_status. On success mfrc522_drv_status_ok is returned.
 */
mfrc522_drv_status
mfrc522_drv_write_masked(const mfrc522_drv_conf* conf, mfrc522_reg addr, u8 val, u8 mask, u8 pos);

/**
 * Read from a PCD.
 *
 * The function performs read from a PCD (Proximity Coupling Device). Depending on low-level call mechanism, 'pointer'
 * or 'definition' method is used.
 *
 * Note that depending on compiler used, the function may be in fact not inlined.
 * In a case when either 'conf' or 'payload' is NULL, mfrc522_drv_status_nullptr is returned.
 *
 * @param conf Pointer to a configuration structure.
 * @param addr Register address.
 * @param payload Pointer to a buffer where incoming bytes are written. Must be big enough to store all data.
 * @return An instance of mfrc522_drv_status. On success mfrc522_drv_status_ok is returned.
 */
static inline mfrc522_drv_status
mfrc522_drv_read(const mfrc522_drv_conf* conf, mfrc522_reg addr, u8* payload)
{
    ERROR_IF_EQ(conf, NULL, mfrc522_drv_status_nullptr);
    ERROR_IF_EQ(payload, NULL, mfrc522_drv_status_nullptr);
#if MFRC522_LL_PTR
    return (mfrc522_ll_status_ok == conf->ll_recv(addr, payload)) ? mfrc522_drv_status_ok : mfrc522_drv_status_ll_err;
#elif MFRC522_LL_DEF
    (void)conf; /* Make compiler happy */
    return (mfrc522_ll_status_ok == mfrc522_ll_recv(addr, payload)) ? mfrc522_drv_status_ok : mfrc522_drv_status_ll_err;
#endif
}

/**
 * Perform masked read from a PCD's register.
 *
 * The function read from a register and extracts only desired set of bits.
 * It may be useful when only one field from a register needs to be read.
 *
 * @param conf Device's configuration.
 * @param addr Address to read from.
 * @param out Pointer to a buffer to store the result in.
 * @param mask Field mask.
 * @param pos Position of the field.
 * @return An instance of mfrc522_drv_status. On success mfrc522_drv_status_ok is returned.
 */
mfrc522_drv_status
mfrc522_drv_read_masked(const mfrc522_drv_conf* conf, mfrc522_reg addr, u8* out, u8 mask, u8 pos);

/**
 * Store single byte in the FIFO buffer.
 *
 * @param conf Pointer to a device configuration structure.
 * @param byte Byte to be written.
 * @return 'mfrc522_drv_status_ok' on success, 'mfrc522_drv_status_ll_err' on failure.
 */
static inline mfrc522_drv_status
mfrc522_drv_fifo_store(const mfrc522_drv_conf* conf, u8 byte)
{
    return mfrc522_drv_write_byte(conf, mfrc522_reg_fifo_data, byte);
}

/**
 * Store multiple bytes in the FIFO buffer.
 *
 * @param conf Pointer to a device configuration structure.
 * @param bytes Vector of bytes to be written
 * @param sz The number of bytes to be written
 * @return 'mfrc522_drv_status_ok' on success, 'mfrc522_drv_status_ll_err' on failure.
 */
static inline mfrc522_drv_status
mfrc522_drv_fifo_store_mul(const mfrc522_drv_conf* conf, u8* bytes, size sz)
{
    return mfrc522_drv_write(conf, mfrc522_reg_fifo_data, sz, bytes);
}

/**
 * Read single byte from the FIFO buffer.
 *
 * Call to this function is valid only when FIFO buffer contains at least single byte inside.
 * Otherwise garbage value may be returned.
 *
 * @param conf Pointer to a device configuration structure.
 * @param out Buffer to write output byte to.
 * @return 'mfrc522_drv_status_ok' on success, 'mfrc522_drv_status_ll_err' on failure.
 */
static inline mfrc522_drv_status
mfrc522_drv_fifo_read(const mfrc522_drv_conf* conf, u8* out)
{
    return mfrc522_drv_read(conf, mfrc522_reg_fifo_data, out);
}

/**
 * Flush the FIFO buffer.
 *
 * @param conf Pointer to a device configuration structure.
 * @return 'mfrc522_drv_status_ok' on success, 'mfrc522_drv_status_ll_err' on failure.
 */
static inline mfrc522_drv_status
mfrc522_drv_fifo_flush(const mfrc522_drv_conf* conf)
{
    return mfrc522_drv_write_masked(conf, mfrc522_reg_fifo_level, 1, MFRC522_REG_FIELD(FIFOLEVEL_FLUSH));
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
mfrc522_drv_status
mfrc522_drv_read_until(const mfrc522_drv_conf* conf, mfrc522_drv_read_until_conf* ru_conf);

/**
 * Perform soft reset on a PCD.
 *
 * Note that the function does not wait until currently active command terminates.
 * An error code is returned when NULL was passed instead of a valid 'conf' pointer.
 *
 * @param conf Pointer to a configuration structure.
 * @return An instance of 'mfrc522_drv_status'. On success, mfrc522_drv_status_ok is returned.
 */
mfrc522_drv_status
mfrc522_drv_soft_reset(const mfrc522_drv_conf* conf);

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
mfrc522_drv_status
mfrc522_drv_tim_set(mfrc522_drv_tim_conf* tim_conf, u16 period);

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
mfrc522_drv_status
mfrc522_drv_tim_start(const mfrc522_drv_conf* conf, const mfrc522_drv_tim_conf* tim_conf);

/**
 * Stop MFRC522 timer.
 *
 * The function stops timer unit immediately.
 * The function returns error code when NULL was passed instead of a valid configuration pointer.
 *
 * @param conf Pointer to a configuration structure.
 * @return Status of the operation. On success mfrc522_drv_status_ok is returned.
 */
mfrc522_drv_status
mfrc522_drv_tim_stop(const mfrc522_drv_conf* conf);

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
mfrc522_drv_status
mfrc522_drv_irq_init(const mfrc522_drv_conf* conf, const mfrc522_drv_irq_conf* irq_conf);

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
mfrc522_drv_status
mfrc522_drv_irq_clr(const mfrc522_drv_conf* conf, mfrc522_reg_irq irq);

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
mfrc522_drv_status
mfrc522_drv_irq_en(const mfrc522_drv_conf* conf, mfrc522_reg_irq irq, bool enable);

/**
 * Get states of all interrupts.
 *
 * The function read states of IRQs in Com and Div registers. As an output 2 bytes are returned.
 * The LSB contains states of Com register and MSB contains states of Div register respectively.
 * The output is valid only when 'ok' status is returned.
 * The function returns error when NUll was passed instead of a valid pointer.
 *
 * @param conf Pointer to a device configuration structure.
 * @param out Pointer of a buffer where the states are stored.
 * @return Status of the operation. On success mfrc522_drv_status_ok is returned.
 */
mfrc522_drv_status
mfrc522_irq_states(const mfrc522_drv_conf* conf, u16* out);

/**
 * Check if a certain IRQ is pending.
 *
 * For performance reasons this function does not read interrupt states from the registers.
 * To get the IRQ states, call 'mfrc522_irq_states()'.
 *
 * @param irq_states States of all IRQs
 * @param irq IRQ number.
 * @return True if IRQ is pending, false otherwise.
 */
bool
mfrc522_drv_irq_pending(u16 irq_states, mfrc522_reg_irq irq);

/**
 * Perform device self-test procedure.
 *
 * The function returns 'mfrc522_drv_status_self_test_err' if received test data differs from expected
 * bytes (stored inside 'MFRC522_CONF_SELF_TEST_FIFO_OUT' configuration macro).
 * After self-test procedure, the output data is stored inside device's configuration structure.
 *
 * The function returns error when NUll was passed instead of a 'conf' pointer.
 *
 * @param conf Pointer to a configuration structure.
 * @return Status of the operation. On success mfrc522_drv_status_ok is returned.
 */
mfrc522_drv_status
mfrc522_drv_self_test(mfrc522_drv_conf* conf);

/**
 * Invoke a MFRC522 command.
 *
 * In a case, when finite operation should be executed, the function waits until idle command is active back.
 * The function returns error when NUll was passed instead of a 'conf' pointer.
 *
 * @param conf Pointer to a configuration structure.
 * @param cmd Command to be invoked.
 * @return Status of the operation. On success mfrc522_drv_status_ok is returned.
 */
mfrc522_drv_status
mfrc522_drv_invoke_cmd(const mfrc522_drv_conf* conf, mfrc522_reg_cmd cmd);

/**
 * Initialize CRC coprocessor.
 *
 * The CRC polynomial is fixed to x^16 + x^12 + x^5 + 1.
 * The function returns error when either 'conf' or 'crc_conf' is NULL.
 *
 * @param conf Pointer to a device configuration structure.
 * @param crc_conf Pointer to a CRC configuration structure.
 * @return Status of the operation. On success 'mfrc522_drv_status_ok' is returned.
 */
mfrc522_drv_status
mfrc522_drv_crc_init(const mfrc522_drv_conf* conf, const mfrc522_drv_crc_conf* crc_conf);

/**
 * Compute 16-bit CRC.
 *
 * The function calculates CRC based on FIFO contents. The CRC coprocessor shall be initialized prior to call to this
 * function, as well as FIFO buffer should contain desired data.
 * Computed CRC value is only valid, when 'ok' status code was returned.
 *
 * @param conf Pointer to a device's configuration structure.
 * @param out Buffer to store the outcome in.
 * @return Status of the operation. On success 'mfrc522_drv_status_ok' is returned.
 */
mfrc522_drv_status
mfrc522_drv_crc_compute(const mfrc522_drv_conf* conf, u16* out);

/**
 * Generate random ID.
 *
 * The function uses on-chip Random Number Generator to generate n-bytes random ID.
 * Up to ten random bytes can be returned via single call of this function.
 * When 'num_rand' is greater than ten, ten bytes are returned.
 * Passing zero as 'num_rand' is also valid - none bytes are written into the buffer then.
 * The output buffer has to be large enough to accommodate output data.
 *
 * Note: Data written to the buffer is valid only when 'ok' status code was returned.
 * The function return an error code in a case when NULL was passed instead of a valid pointer.
 *
 * @param conf Pointer to a device's configuration structure.
 * @param out A buffer to store outcome data.
 * @param num_rand Number of random bytes to be returned.
 * @return Status of the operation. On success 'mfrc522_drv_status_ok' is returned.
 */
mfrc522_drv_status
mfrc522_drv_generate_rand(const mfrc522_drv_conf* conf, u8* out, size num_rand);

/**
 * Check if a certain error is present in Error Register.
 *
 * In a case, when presence of any error bit is desired, 'mfrc522_reg_err_any' may be passed.
 *
 * @param error_reg A byte read from Error Register.
 * @param err An error to be checked.
 * @return True if error is active, false otherwise.
 */
bool
mfrc522_drv_check_error(u8 error_reg, mfrc522_reg_err err);

/**
 * Transceive data between PCD and PICC.
 *
 * The function performs the transmission of data from the FIFO buffer and the reception of data from the RF field.
 * It is recommended to not use this function directly, since different configuration ought to be used depending on
 * actual demands. Call higher level APIs instead that populates the fields of 'mfrc522_drv_transceive_conf' struct
 * in the right way.
 *
 * @param conf Pointer to a device configuration struct.
 * @param tr_conf Pointer to a transceive configuration struct.
 * @return Status of the operation. On success 'mfrc522_drv_status_ok' is returned.
 */
mfrc522_drv_status
mfrc522_drv_transceive(const mfrc522_drv_conf* conf, mfrc522_drv_transceive_conf* tr_conf);

/**
 * Initialize contactless external interfaces (contactless UART, analog interface).
 *
 * The function has to be called before using any of APIs that communicate with the RF field.
 * Nothing is done, when NULL was passed instead of a valid pointer.
 *
 * @param conf Pointer to a device configuration struct.
 * @param itf_conf Configuration of external interfaces.
 * @return Status of the operation. On success 'mfrc522_drv_status_ok' is returned.
 */
mfrc522_drv_status
mfrc522_drv_ext_itf_init(const mfrc522_drv_conf* conf, const mfrc522_drv_ext_itf_conf* itf_conf);

/**
 * Perform Request (REQA) command.
 *
 * The function sends REQA into the aether. If a PICC is present in the RF field it will respond with 2-byte ATQA code.
 *
 * If 'ok' status code is returned, the data inside 'atqa' buffer is valid and can be used by other APIs.
 * In case, when an error occurred during data transmission/reception, 'atqa' buffer is populated with
 * MFRC522_PICC_ATQA_INV constant.
 * It is possible to narrow down the set of valid ATQA responses by setting 'atqa_verify_fn' pointer in device's
 * configuration structure. 'mfrc522_drv_status_picc_vrf_err' is returned if ATQA is blacklisted then.
 *
 * The function does nothing, in case NULL was passed instead of a valid pointer.
 *
 * @param conf Device's configuration.
 * @param atqa 2-byte output buffer to store ATQA response
 * @return Status of the operation. On success 'mfrc522_drv_status_ok' is returned.
 */
mfrc522_drv_status
mfrc522_drv_reqa(const mfrc522_drv_conf* conf, u16* atqa);

/**
 * Perform anticollision procedure.
 *
 * The anticollision procedure allows to select only single PICC, in a case when more than one card is present in the
 * RF field. As a result a vector of 5 bytes is returned (called serial output).
 * Serial data consist of 5 bytes: NUID (4 bytes) + checksum (1 byte) and is used in further API calls.
 * The output is valid only when 'ok' status was returned.
 *
 * The function has to be called after ATQA response was collected during REQA command.
 * The function does nothing, when NULL was passed instead of a valid pointer.
 *
 * @param conf Device configuration structure.
 * @param serial Output buffer where serial data will be stored.
 * @return Status of the operation. On success 'mfrc522_drv_status_ok' is returned.
 */
mfrc522_drv_status
mfrc522_drv_anticollision(const mfrc522_drv_conf* conf, u8* serial);

/**
 * Select a PICC to communicate with.
 *
 * The function selects a PICC for further actions. As an input serial data returned from the anticollision sequence
 * shall be passed. Thus this function has to be called directly after anticollision procedure.
 * The PICC answers with SAK response (1 byte of exact SAK + 2 bytes of CRC).
 * The function assumes that 'serial' vector contains exactly 5 bytes. In other case undefined behaviour is guaranteed.
 * The CRC coprocessor has to be initialized prior to calling this function.
 *
 * The function does nothing when NULL was passed instead of a valid pointer.
 *
 * @param conf Device configuration struct.
 * @param serial Serial data of a PICC.
 * @param sak Buffer to store SAK response in.
 * @return Status of the operation. On success 'mfrc522_drv_status_ok' is returned.
 */
mfrc522_drv_status
mfrc522_drv_select(const mfrc522_drv_conf* conf, const u8* serial, u8* sak);

/**
 * Authenticate PICC block.
 *
 * This function manages authentication to enable a secure communication to any card by using either secret key A or B.
 * After successful call to this function, the internal crypto unit goes to the active state.
 * The function shall be called after certain PICC was selected.
 *
 * The function does nothing when NULL was passed instead of a valid pointer.
 *
 * @param conf Device configuration struct.
 * @param auth_conf Authentication parameters.
 * @return Status of the operation. On success 'mfrc522_drv_status_ok' is returned.
 */
mfrc522_drv_status
mfrc522_drv_authenticate(const mfrc522_drv_conf* conf, const mfrc522_drv_auth_conf* auth_conf);

/**
 * Halt a PICC.
 *
 * After successful call to this function, the internal crypto unit is turned off and the PCD is ready to perform REQA
 * again, thus polling for another PICC is possible at this stage.
 * The function does nothing when NULL was passed instead of a valid pointer.
 *
 * @param conf Device configuration.
 * @return Status of the operation. On success 'mfrc522_drv_status_ok' is returned.
 */
mfrc522_drv_status
mfrc522_drv_halt(const mfrc522_drv_conf* conf);

#ifdef __cplusplus
}
#endif

#endif //MFRC522_MFRC522_DRV_H

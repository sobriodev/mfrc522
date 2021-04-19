#ifndef MFRC522_MFRC522_LL_H
#define MFRC522_MFRC522_LL_H

#include "type.h"

/* Ensure that either 'definition' or 'pointer' low-level communication method is used */
#if MFRC522_LL_DEF
#if MFRC522_LL_PTR
#error "MFRC522 driver: select either MFRC522_LL_DEF or MFRC522_LL_PTR but not both!"
#endif
#elif MFRC522_LL_PTR
#if MFRC522_LL_DEF
#error "MFRC522 driver: select either MFRC522_LL_DEF or MFRC522_LL_PTR but not both!"
#endif
#else
#error "MFRC522 driver: select either MFRC522_LL_DEF or MFRC522_LL_PTR low-level communication method!"
#endif

#ifdef __cplusplus
extern "C" {
#endif

/* ------------------------------------------------------------ */
/* ---------------------------- Macros ------------------------ */
/* ------------------------------------------------------------ */

/** Major version */
#define MFRC522_LL_API_VERSION_MAJOR 0
/** Minor version */
#define MFRC522_LL_API_VERSION_MINOR 0
/** Revision version */
#define MFRC522_LL_VERSION_REVISION 0

/* ------------------------------------------------------------ */
/* -------------------------- Data types ---------------------- */
/* ------------------------------------------------------------ */

/**
 * Status codes used by low-level calls
 */
typedef enum mfrc522_ll_status_
{
    mfrc522_ll_status_ok = 0, /**< Success */
    mfrc522_ll_status_send_err, /**< An error while sending data */
    mfrc522_ll_status_recv_err /**< An error while receiving data */
} mfrc522_ll_status;

#if MFRC522_LL_PTR
/**
 * Low-level send function type. From high-level point of view the library does not assume which digital interface is
 * used. Beneath the function any bus can be used, e.g. SPI, I2C or UART. Please refer to device datasheet to check
 * supported digital interface(s).
 *
 * @param addr MFRC522 register address.
 * @param payload Payload byte.
 * @return An instance of mfrc522_ll_status. Valid return codes are:
 *         - mfrc522_ll_status_send_err on error
 *         - mfrc522_ll_status_ok on success
 */
typedef mfrc522_ll_status (*mfrc522_ll_send)(u8 addr, u8 payload);

/**
 * Low-level receive function type. From high-level point of view the library does not assume which digital interface is
 * used. Beneath the function any bus can be used, e.g. SPI, I2C or UART. Please refer to device datasheet to check
 * supported digital interface(s).
 *
 * @param addr MFRC522 register address.
 * @param payload Address of a buffer to store received data.
 * @return An instance of mfrc522_ll_status. Valid return codes are:
 *         - mfrc522_ll_status_recv_err on error
 *         - mfrc522_ll_status_ok on success
 */
typedef mfrc522_ll_status (*mfrc522_ll_recv)(u8 addr, u8* payload);

#if MFRC522_LL_DELAY
/**
 * Low-level delay function type. When enabled, API calls can use it in certain situations as a 'sleep call' while
 * waiting for an event, thus reducing bus congestion. When disabled, the library pools a device until condition is met,
 * resulting in increased bus workload.
 *
 * @param period Number of microseconds to sleep.
 */
typedef void (*mfrc522_ll_delay)(u32 period);
#endif

#endif

/* ------------------------------------------------------------ */
/* ----------------------- Public functions ------------------- */
/* ------------------------------------------------------------ */

#ifdef MFRC522_LL_DEF
/**
 * Low-level function to send data to a device. From high-level point of view the library does not assume which digital
 * interface is used. Beneath the function any bus can be used, e.g. SPI, I2C or UART. Please refer to device datasheet
 * to check supported digital interface(s).
 *
 * @param addr MFRC522 register address.
 * @param payload Payload byte.
 * @return An instance of mfrc522_ll_status. Valid return codes are:
 *         - mfrc522_ll_status_send_err on error
 *         - mfrc522_ll_status_ok on success
 */
mfrc522_ll_status mfrc522_ll_send(u8 addr, u8 payload);

/**
 * Low-level function to receive data from a device. From high-level point of view the library does not assume which
 * digital interface is used. Beneath the function any bus can be used, e.g. SPI, I2C or UART. Please refer to device
 * datasheet to check supported digital interface(s).
 *
 * @param addr MFRC522 register address.
 * @param payload Address of a buffer to store received data.
 * @return An instance of mfrc522_ll_status. Valid return codes are:
 *         - mfrc522_ll_status_recv_err on error
 *         - mfrc522_ll_status_ok on success
 */
mfrc522_ll_status mfrc522_ll_recv(u8 addr, u8* payload);

#if MFRC522_LL_DELAY
/**
 * Low-level delay function type. When enabled, API calls can use it in certain situations as a 'sleep call' while
 * waiting for an event, thus reducing bus congestion. When disabled, the library pools a device until condition is met,
 * resulting in increased bus workload.
 *
 * @param period Number of microseconds to sleep.
 */
void mfrc522_ll_delay(u32 period);
#endif

#endif

#ifdef __cplusplus
}
#endif

#endif //MFRC522_MFRC522_LL_H

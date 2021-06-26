#ifndef MFRC522_COMMON_H
#define MFRC522_COMMON_H

#ifdef __cplusplus
extern "C" {
#endif

/* ------------------------------------------------------------ */
/* -------------------------- Macros -------------------------- */
/* ------------------------------------------------------------ */

/**
 * Likely branch prediction
 */
#define LIKELY(X) __builtin_expect(!!(X), 1)

/**
 * Unlikely branch prediction
 */
#define UNLIKELY(X) __builtin_expect(!!(X), 0)

/**
 * Return an error if a value is not valid
 */
#define ERROR_IF_EQ(ACT, EXP, RET) do { if (UNLIKELY((EXP) == (ACT))) return (RET); } while (0)

/**
 * Return a value if it differs from expected one
 */
#define ERROR_IF_NEQ(ACT, EXP) do { if (UNLIKELY((EXP) != (ACT))) return (ACT); } while (0)

/**
 * Calculate size of an array
 */
#define SIZE_ARRAY(ARR) (sizeof((ARR)) / sizeof((ARR)[0]))

/**
 * Mask for sequential number bits in status code
 */
#define STATUS_SEQ_MASK 0xFF

/**
 * Position of sequential number bits in status code
 */
#define STATUS_SEQ_POS 0

/**
 * Mask for severity bits in status code
 */
#define STATUS_SEVERITY_MASK 0x0F

/**
 * Position of severity bits in status code
 */
#define STATUS_SEVERITY_POS 8

/**
 * Mask for scope bits in status code
 */
#define STATUS_SCOPE_MASK 0xFFFF

/**
 * Position of scope bits in status code
 */
#define STATUS_SCOPE_POS 12

/**
 * Macro to create status code. It assumes that SCOPE_MAGIC macro is set.
 * Up to 255 distinct codes may be created under the same scope.
 */
#define MAKE_STATUS(SEQ, SEVERITY) \
    (((SCOPE_MAGIC) & STATUS_SCOPE_MASK) << STATUS_SCOPE_POS) | \
    (((SEVERITY) & STATUS_SEVERITY_MASK) << STATUS_SEVERITY_POS) | \
    (((SEQ) & STATUS_SEQ_MASK) << STATUS_SEQ_POS)

/* ------------------------------------------------------------ */
/* ------------------------ Data types ------------------------ */
/* ------------------------------------------------------------ */

/**
 * Severities of status code
 */
typedef enum status_severity_
{
    /**<
     * Special value to represent no severity (e.g. 'ok' status code)
     */
    status_severity_none = 0x00,
    /**<
     * Fatal error that needs attention immediately.
     * Lefts the system in broken or not logically consistent state when unhandled
    */
    status_severity_fatal = 0x0E,
    /**<
     * Critical error that shall not occur in normal procedure.
     * Does not make the system broken
     */
    status_severity_critical = 0x0C,
    /**<
     * Non-critical error.
     * May occur in normal procedure
     */
    status_severity_non_critical = 0x0A
} status_severity;

/* ------------------------------------------------------------ */
/* --------------------- Public functions --------------------- */
/* ------------------------------------------------------------ */

/**
 * Check if a status code is fatal, hence shall be handled urgently.
 *
 * @param status Status to be checked.
 * @return True if the status is fatal, false otherwise.
 */
static inline bool status_fatal(u32 status)
{
    return (status_severity_fatal == ((status >> STATUS_SEVERITY_POS) & STATUS_SEVERITY_MASK));
}

#ifdef __cplusplus
}
#endif

#endif //MFRC522_COMMON_H

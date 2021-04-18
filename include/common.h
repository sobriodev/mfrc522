#ifndef MFRC522_COMMON_H
#define MFRC522_COMMON_H

#ifdef __cplusplus
extern "C" {
#endif

/* ------------------------------------------------------------ */
/* -------------------------- Macros -------------------------- */
/* ------------------------------------------------------------ */

/** Likely branch prediction */
#define LIKELY(X)   __builtin_expect(!!(X), 1)
/** Unlikely branch prediction */
#define UNLIKELY(X) __builtin_expect(!!(X), 0)

/** Return an error if a value is not valid */
#define ERROR_IF_EQ(ACT, EXP, RET) do { if (UNLIKELY((EXP) == (ACT))) return (RET); } while (0)
/** Return a value if it differs from expected one */
#define ERROR_IF_NEQ(ACT, EXP) do { if (UNLIKELY((EXP) != (ACT))) return (ACT); } while (0)

#ifdef __cplusplus
}
#endif

#endif //MFRC522_COMMON_H

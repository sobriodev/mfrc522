#ifndef MFRC522_TYPE_H
#define MFRC522_TYPE_H

#ifdef __cplusplus

#include <cstdint>
#include <cstddef>

#else
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

/* ------------------------------------------------------------ */
/* ------------------------ Data types ------------------------ */
/* ------------------------------------------------------------ */

/** 8-bit width unsigned type */
typedef uint8_t u8;
/** 16-bit width unsigned type */
typedef uint16_t u16;
/** 32-bit width unsigned type */
typedef uint32_t u32;
/** 64-bit width unsigned type */
typedef uint64_t u64;
/** Size type */
typedef size_t size;

/** 8-bit width signed type */
typedef int8_t i8;
/** 16-bit width signed type */
typedef int16_t i16;
/** 32-bit width signed type */
typedef int32_t i32;
/** 64-bit width signed type */
typedef int64_t i64;

#ifdef __cplusplus
}
#endif

#endif //MFRC522_TYPE_H

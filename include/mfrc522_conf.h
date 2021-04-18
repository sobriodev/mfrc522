#ifndef MFRC522_MFRC522_CONF_H
#define MFRC522_MFRC522_CONF_H

/**
 * High nibble of VersionReg. The library cooperates with MFRC522 devices only (VersionReg=9xh).
 * If your device returns different chip type and you are sure it is based on MFRC522 you can change it via below macro.
 * Low nibble stands for version number and it is not directly used by the library (should be set to zero).
 */
#define MFRC522_CONF_CHIP_TYPE 0x90

/**
 * Retry count multiplier used only if MFRC522_LL_DELAY is not set.
 * The value is used to increase number of retry count on different operations. For example:
 *
 * Case 1:
 * Delay function is taken into use and retry count is set to some value. In general, total number of time in which
 * device has to respond is: delay * retry count.
 *
 * Case 2:
 * Delay function is disabled and retry count is set ot some value. In general, total number of time in which device has
 * to respond is: retry count * time of executing one iteration of a loop. In some circumstances this may be not enough.
 * The multiplier has to be tuned, depending on bus settings, e.g. clock frequency and other links.
 */
#define MFRC522_CONF_RETRY_CNT_MUL 10

#endif //MFRC522_MFRC522_CONF_H

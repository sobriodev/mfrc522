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
 * Delay function is disabled and retry count is set to some value. In general, total number of time in which device has
 * to respond is: retry count * time of executing one iteration of a loop. In some circumstances this may be not enough.
 * The multiplier has to be tuned, depending on bus settings, e.g. clock frequency and other links.
 */
#define MFRC522_CONF_RETRY_CNT_MUL 10

/**
 * According to MFRC522 documentation, the formula to calculate timer period is:
 * TPeriod = ((TPrescaler * 2 + Fixed) * (TReload + 1)) / 13.56 MHz,
 * where 'Fixed' equals to 1 when odd prescaler is used, otherwise 'Fixed' equals to 2.
 *
 * However, tests shown that TReload is not increased by one. Maybe it depends on module version or some Chinese
 * products behave in a different way. When below the macro is set, following formula is taken into use:
 *
 * TPeriod = ((TPrescaler * 2 + Fixed) * (TReload)) / 13.56 MHz,
 * where 'Fixed' equals to 1 when odd prescaler is used, otherwise 'Fixed' equals to 2.
 *
 * The macro is used only in functions that convert given time period to TPrescaler and TReload. It does not apply when
 * these values are set manually.
 */
#define MFRC522_CONF_TIM_RELOAD_FIXED 1

/**
 * Expected FIFO output during self test procedure. Refer to datasheet of your device to check expected output.
 * The size of output vector is 64 bytes. Do not use any braces in this macro.
 */
#define MFRC522_CONF_SELF_TEST_FIFO_OUT \
0x00, 0xBC, 0x0A, 0x27, 0xF7, 0x6F, 0x13, 0xD9, \
0x64, 0x0C, 0x1C, 0x18, 0xD2, 0x90, 0x5B, 0xED, \
0x43, 0xA3, 0x51, 0xE0, 0x3D, 0x91, 0x39, 0x01, \
0x10, 0x44, 0x43, 0x11, 0xA5, 0x31, 0xCE, 0x8D, \
0xF7, 0xDD, 0x46, 0x11, 0x56, 0x63, 0xE2, 0xE1, \
0xB2, 0x5B, 0x79, 0xC1, 0xC9, 0x20, 0xE4, 0x32, \
0xED, 0x20, 0x1B, 0x74, 0x65, 0x9C, 0xFD, 0x32, \
0xEF, 0xCB, 0x9B, 0x01, 0x68, 0xE9, 0x78, 0xEC

#endif //MFRC522_MFRC522_CONF_H

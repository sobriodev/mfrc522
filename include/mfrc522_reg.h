#ifndef MFRC522_MFRC522_REG_H
#define MFRC522_MFRC522_REG_H

#ifdef __cplusplus
extern "C" {
#endif

/* ------------------------------------------------------------ */
/* -------------------------- Macros -------------------------- */
/* ------------------------------------------------------------ */

/** Invalid chip version number */
#define MFRC522_REG_VERSION_INVALID 0xFF

/** Bit mask for Chiptype field in version register */
#define MFRC522_REG_VERSION_CHIP_TYPE_MSK 0xF0

/* ------------------------------------------------------------ */
/* ------------------------ Data types ------------------------ */
/* ------------------------------------------------------------ */

/**
 * MFRC522 register map
 */
typedef enum mfrc522_reg_
{
    /* Page 0: Command and Status */
    mfrc522_reg_reserved0 = 0x00, /**< Reserved */
    mfrc522_reg_command = 0x01, /**< CommandReg */
    mfrc522_reg_com_irq_en = 0x02, /**< ComIEnReg */
    mfrc522_reg_div_irq_en = 0x03, /**< DivLEnReg */
    mfrc522_reg_com_irq = 0x04, /**< ComIrqReg */
    mfrc522_reg_div_irq = 0x05, /**< DivIrqReg */
    mfrc522_reg_error_reg = 0x06, /**< ErrorReg */
    mfrc522_reg_status1_reg = 0x07, /**< Status1Reg */
    mfrc522_reg_status2_reg = 0x08, /**< Status2Reg */
    mfrc522_reg_fifo_data_reg = 0x09, /**< FIFODataReg */
    mfrc522_reg_fifo_level_reg = 0x0A, /**< FIFOLevelReg */
    mfrc522_reg_water_level_reg = 0x0B, /**< WaterLevelReg */
    mfrc522_reg_control_reg = 0x0C, /**< ControlReg */
    mfrc522_reg_bit_framing_reg = 0x0D, /**< BitFramingReg */
    mfrc522_reg_coll_reg = 0x0E, /**< CollReg */
    mfrc522_reg_reserved1 = 0x0F, /**< Reserved */

    /* Page 1: Command */
    mfrc522_reg_reserved2 = 0x10, /**< Reserved */
    mfrc522_reg_mode = 0x11, /**< ModeReg */
    mfrc522_reg_tx_mode = 0x12, /**< TxModeReg */
    mfrc522_reg_rx_mode = 0x13, /**< RxModeReg */
    mfrc522_reg_tx_control = 0x14, /**< TxControlReg */
    mfrc522_reg_tx_ask = 0x15, /**< TxASKReg */
    mfrc522_reg_tx_sel = 0x16, /**< TxSelReg */
    mfrc522_reg_rx_sel = 0x17, /**< RxSelReg */
    mfrc522_reg_rx_threshold = 0x18, /**< RxThresholdReg */
    mfrc522_reg_demod = 0x19, /**< DemodReg */
    mfrc522_reg_reserved3 = 0x1A, /**< Reserved */
    mfrc522_reg_reserved4 = 0x1B, /**< Reserved */
    mfrc522_reg_mf_tx = 0x1C, /**< MfTxReg */
    mfrc522_reg_mf_rx = 0x1D, /**< MfRxReg */
    mfrc522_reg_reserved5 = 0x1E, /**< Reserved */
    mfrc522_reg_serial_speed = 0x1F, /**< SerialSpeedReg */

    /* Page 2: Configuration */
    mfrc522_reg_reserved6 = 0x20, /**< Reserved */
    mfrc522_reg_crc_result_msb = 0x21, /**< CRCResultReg */
    mfrc522_reg_crc_result_lsb = 0x22, /**< CRCResultReg */
    mfrc522_reg_reserved7 = 0x23, /**< Reserved */
    mfrc522_reg_mod_width = 0x24, /**< ModWidthReg */
    mfrc522_reg_reserved8 = 0x25, /**< Reserved */
    mfrc522_reg_rf_cfg = 0x26, /**< RFCfgReg */
    mfrc522_reg_gs_n = 0x27, /**< GsNReg */
    mfrc522_reg_cw_gs = 0x28, /**< CWGsPReg */
    mfrc522_reg_mod_gs = 0x29, /**< ModGsPReg */
    mfrc522_reg_tim_mode = 0x2A, /**< TModeReg */
    mfrc522_reg_tim_prescaler = 0x2B, /**< TPrescalerReg */
    mfrc522_reg_tim_reload_hi = 0x2C, /**< TReloadReg */
    mfrc522_reg_tim_reload_lo = 0x2D, /**< TReloadReg */
    mfrc522_reg_tim_counter_val_hi = 0x2E, /**< TCounterValReg */
    mfrc522_reg_tim_counter_val_lo = 0x2F, /**< TCounterValReg */

    /* Page 3: Test */
    mfrc522_reg_reserved9 = 0x30, /**< Reserved */
    mfrc522_reg_test_sel1 = 0x31, /**< TestSel1Reg */
    mfrc522_reg_test_sel2 = 0x32, /**< TestSel2Reg */
    mfrc522_reg_test_pin_en = 0x33, /**< TestPinEnReg */
    mfrc522_reg_test_pin_value = 0x34, /**< TestPinValueReg */
    mfrc522_reg_test_bus = 0x35, /**< TestBusReg */
    mfrc522_reg_auto_test = 0x36, /**< AutoTestReg */
    mfrc522_reg_version = 0x37, /**< VersionReg */
    mfrc522_reg_analog_test = 0x38, /**< AnalogTestReg */
    mfrc522_reg_test_dac1 = 0x39, /**< TestDAC1Reg */
    mfrc522_reg_test_dac2 = 0x3A, /**< TestDAC2Reg */
    mfrc522_reg_test_adc = 0x3B, /**< TestADCReg */
    mfrc522_reg_reserved10 = 0x3C, /**< Reserved */
    mfrc522_reg_reserved11 = 0x3D, /**< Reserved */
    mfrc522_reg_reserved12 = 0x3E, /**< Reserved */
    mfrc522_reg_reserved13 = 0x3F, /**< Reserved */
} mfrc522_reg;

#ifdef __cplusplus
}
#endif

#endif //MFRC522_MFRC522_REG_H

#include "Mockable.h"

/* Required by C Mock library */
DEFINE_MOCKABLE(mfrc522_ll_status, mfrc522_ll_send, (u8, size, const u8*));
DEFINE_MOCKABLE(mfrc522_ll_status, mfrc522_ll_recv, (u8, u8*));
DEFINE_MOCKABLE(void, mfrc522_ll_delay, (u32));
DEFINE_MOCKABLE(mfrc522_drv_status, mfrc522_drv_init, (mfrc522_drv_conf*));
DEFINE_MOCKABLE(mfrc522_drv_status, mfrc522_drv_soft_reset, (const mfrc522_drv_conf*));
DEFINE_MOCKABLE(mfrc522_drv_status, mfrc522_drv_invoke_cmd, (const mfrc522_drv_conf*, mfrc522_reg_cmd));
DEFINE_MOCKABLE(mfrc522_drv_status, mfrc522_irq_states, (const mfrc522_drv_conf*, u16*));
DEFINE_MOCKABLE(mfrc522_drv_status, mfrc522_drv_transceive, (const mfrc522_drv_conf*, mfrc522_drv_transceive_conf*));
DEFINE_MOCKABLE(mfrc522_drv_status, mfrc522_drv_crc_compute, (const mfrc522_drv_conf*, u16*));
DEFINE_MOCKABLE(mfrc522_drv_status, mfrc522_drv_irq_clr, (const mfrc522_drv_conf*, mfrc522_reg_irq));
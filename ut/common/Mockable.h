#ifndef MFRC522_MOCKABLE_H
#define MFRC522_MOCKABLE_H

#include "mfrc522_drv.h"
#include "mfrc522_ll.h"
#include <cmock/cmock.h>

/* ------------------------------------------------------------ */
/* -------------------------- Macros -------------------------- */
/* ------------------------------------------------------------ */

/* Just for convenience */
#define DECLARE_MOCKABLE(RET, FUNC, ARGS) \
class FUNC##__MOCK : public CMockMocker<FUNC##__MOCK> \
{ \
public:\
    CMOCK_MOCK_METHOD(RET, FUNC, ARGS) \
}

#define DEFINE_MOCKABLE(RET, FUNC, ARGS) \
CMOCK_MOCK_FUNCTION(FUNC##__MOCK, RET, FUNC, ARGS) static_assert(true, "Semicolon required")

#define MOCK(FUNC) FUNC##__MOCK FUNC##__mocked
#define STUB(FUNC) NiceMock<FUNC##__MOCK> FUNC##__mocked
#define MOCK_CALL(FUNC, ...) EXPECT_CALL(FUNC##__mocked, FUNC(__VA_ARGS__))
#define STUB_CALL(FUNC, ...) ON_CALL(FUNC##__mocked, FUNC(__VA_ARGS__))
#define IGNORE_REDUNDANT_LL_RECV_CALLS() \
MOCK_CALL(mfrc522_ll_recv, _, _).Times(AnyNumber()) \
    .WillRepeatedly(DoAll(SetArgPointee<1>(0x00), Return(mfrc522_ll_status_ok)))
#define IGNORE_REDUNDANT_LL_SEND_CALLS() \
MOCK_CALL(mfrc522_ll_send, _, _, _).Times(AnyNumber()) \
    .WillRepeatedly(Return(mfrc522_ll_status_ok))

/* ------------------------------------------------------------ */
/* -------------------------- Data types ---------------------- */
/* ------------------------------------------------------------ */

/* Put mock declarations here */
DECLARE_MOCKABLE(mfrc522_ll_status, mfrc522_ll_send, (u8, size, const u8*));
DECLARE_MOCKABLE(mfrc522_ll_status, mfrc522_ll_recv, (u8, u8*));
DECLARE_MOCKABLE(void, mfrc522_ll_delay, (u32));
DECLARE_MOCKABLE(mfrc522_drv_status, mfrc522_drv_init, (mfrc522_drv_conf*));
DECLARE_MOCKABLE(mfrc522_drv_status, mfrc522_drv_soft_reset, (const mfrc522_drv_conf*));
DECLARE_MOCKABLE(mfrc522_drv_status, mfrc522_drv_invoke_cmd, (const mfrc522_drv_conf*, mfrc522_reg_cmd));
DECLARE_MOCKABLE(mfrc522_drv_status, mfrc522_drv_irq_states, (const mfrc522_drv_conf*, u16*));
DECLARE_MOCKABLE(mfrc522_drv_status, mfrc522_drv_transceive, (const mfrc522_drv_conf*, mfrc522_drv_transceive_conf*));
DECLARE_MOCKABLE(mfrc522_drv_status, mfrc522_drv_crc_compute, (const mfrc522_drv_conf*, u16*));
DECLARE_MOCKABLE(mfrc522_drv_status, mfrc522_drv_irq_clr, (const mfrc522_drv_conf*, mfrc522_reg_irq));

/* ------------------------------------------------------------ */
/* ----------------------- Public functions ------------------- */
/* ------------------------------------------------------------ */

/* Stub function to initialize mfrc522 device */
mfrc522_drv_conf initDevice();

#endif //MFRC522_MOCKABLE_H

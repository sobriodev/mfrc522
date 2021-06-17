#include <gmock/gmock.h>
#include "Mockable.h"
#include "TestCommon.h"
#include "mfrc522_drv.h"
#include "mfrc522_conf.h"

using namespace testing;

/* ------------------------------------------------------------ */
/* ----------------------- Public functions ------------------- */
/* ------------------------------------------------------------ */

mfrc522_drv_conf initDevice()
{
    /* Configuration structure returned by mocked init */
    mfrc522_drv_conf conf;
    conf.chip_version = MFRC522_CONF_CHIP_TYPE;
    conf.atqa_verify_fn = piccAcceptAny;

    STUB(mfrc522_drv_init);
    STUB_CALL(mfrc522_drv_init, _)
            .WillByDefault(DoAll(SetArgPointee<0>(conf), Return(mfrc522_drv_status_ok)));

    mfrc522_drv_conf ret;
    mfrc522_drv_init(&ret);
    return ret;
}

bool piccAcceptAny(u16 atqa)
{
    static_cast<void>(atqa); /* Satisfy compiler */
    return true;
}

bool piccAccept0x0004(u16 atqa)
{
    return (0x0004 == atqa);
}
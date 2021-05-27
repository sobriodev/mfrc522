#include <gmock/gmock.h>
#include "Mockable.h"
#include "mfrc522_drv.h"
#include "mfrc522_conf.h"

using namespace testing;

mfrc522_drv_conf initDevice()
{
    /* Configuration structure returned by mocked init */
    mfrc522_drv_conf conf;
    conf.chip_version = MFRC522_CONF_CHIP_TYPE;

    STUB(mfrc522_drv_init);
    STUB_CALL(mfrc522_drv_init, _)
            .WillByDefault(DoAll(SetArgPointee<0>(conf), Return(mfrc522_drv_status_ok)));

    mfrc522_drv_conf ret;
    mfrc522_drv_init(&ret);
    return ret;
}

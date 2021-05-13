#include "TestRunner.h"
#include "mfrc522_drv.h"
#include "mfrc522_reg.h"
#include "Mfrc522MockWrapper.h"

/* ------------------------------------------------------------ */
/* ------------------------ Test groups ----------------------- */
/* ------------------------------------------------------------ */

TEST_GROUP(TestMfrc522DrvIrq)
{
    LowLevelCallParams llCallParams = {};

    void setup() override
    {
        /* Nothing to do here */
    }

    void teardown() override
    {
        mock().checkExpectations();
        mfrc522DestroyLowLevelParams(llCallParams);
        mock().clear();
    }

    /* Return initialized device structure by value. The function must be called prior to initializing fake device */
    auto initDevice()
    {
        /* Add fake response to return version number */
        llCallParams.push_back(READ(1, mfrc522_reg_version, 0x9B));
        mfrc522UpdateLowLevelExpectations(llCallParams);

        mfrc522_drv_conf conf;
        auto status = mfrc522_drv_init(&conf);
        CHECK_EQUAL(mfrc522_drv_status_ok, status);

        /* Flush params vector and check expectations */
        mock().checkExpectations();
        mfrc522DestroyLowLevelParams(llCallParams);
        mock().clear();
        return conf;
    }
};

/* ------------------------------------------------------------ */
/* ------------------------ Test cases ------------------------ */
/* ------------------------------------------------------------ */

TEST(TestMfrc522DrvIrq, mfrc522_drv_irq_init__NullCases)
{
    mfrc522_drv_conf conf;
    mfrc522_drv_irq_conf irqConf;

    auto status = mfrc522_drv_irq_init(nullptr, &irqConf);
    CHECK_EQUAL(mfrc522_drv_status_nullptr, status);

    status = mfrc522_drv_irq_init(&conf, nullptr);
    CHECK_EQUAL(mfrc522_drv_status_nullptr, status);
}

TEST(TestMfrc522DrvIrq, mfrc522_drv_irq_init__Success)
{
    auto conf = initDevice();

    /* Populate IRQ config fields */
    mfrc522_drv_irq_conf irqConf;
    irqConf.irq_push_pull = true;
    irqConf.irq_signal_inv = false;

    /* Expect that all IRQs are cleared */
    llCallParams.push_back(WRITE1_A(1, mfrc522_reg_com_irq));
    llCallParams.push_back(WRITE1_A(1, mfrc522_reg_div_irq));
    /* Expect calls that configure IRQ registers */
    llCallParams.push_back(WRITE1_A(1, mfrc522_reg_com_irq_en));
    llCallParams.push_back(WRITE1_A(1, mfrc522_reg_div_irq_en));
    mfrc522UpdateLowLevelExpectations(llCallParams);

    auto status = mfrc522_drv_irq_init(&conf, &irqConf);
    CHECK_EQUAL(mfrc522_drv_status_ok, status);
}

TEST(TestMfrc522DrvIrq, mfrc522_drv_irq_clr__NullCases)
{
    auto status = mfrc522_drv_irq_clr(nullptr, mfrc522_reg_irq_all);
    CHECK_EQUAL(mfrc522_drv_status_nullptr, status);
}

TEST(TestMfrc522DrvIrq, mfrc522_drv_irq_clr__ClearAllIrqs__BothRegistersAreUpdated)
{
    auto conf = initDevice();

    /* Expect both registers to be updated */
    u8 word1_irq_bits = 0x7F;
    u8 word2_irq_bits = 0x014;
    llCallParams.push_back(WRITE1(1, mfrc522_reg_com_irq, word1_irq_bits));
    llCallParams.push_back(WRITE1(1, mfrc522_reg_div_irq, word2_irq_bits));
    mfrc522UpdateLowLevelExpectations(llCallParams);

    auto status = mfrc522_drv_irq_clr(&conf, mfrc522_reg_irq_all);
    CHECK_EQUAL(mfrc522_drv_status_ok, status);
}

TEST(TestMfrc522DrvIrq, mfrc522_drv_irq_clr__ClearIrqFromComReg__Success)
{
    auto conf = initDevice();

    /* Expect ComIrqReg to be updated */
    llCallParams.push_back(WRITE1(1, mfrc522_reg_com_irq, 1 << mfrc522_reg_irq_hi_alert));
    mfrc522UpdateLowLevelExpectations(llCallParams);

    auto status = mfrc522_drv_irq_clr(&conf, mfrc522_reg_irq_hi_alert);
    CHECK_EQUAL(mfrc522_drv_status_ok, status);
}

TEST(TestMfrc522DrvIrq, mfrc522_drv_irq_clr__ClearIrqFromDivReg__Success)
{
    auto conf = initDevice();

    /* Expect DivIrqReg to be updated */
    u8 crc_irq_bit = 2;
    llCallParams.push_back(WRITE1(1, mfrc522_reg_div_irq, (u8)(1 << crc_irq_bit)));
    mfrc522UpdateLowLevelExpectations(llCallParams);

    auto status = mfrc522_drv_irq_clr(&conf, mfrc522_reg_irq_crc);
    CHECK_EQUAL(mfrc522_drv_status_ok, status);
}

TEST(TestMfrc522DrvIrq, mfrc522_drv_irq_en__NullCases)
{
    auto status = mfrc522_drv_irq_en(nullptr, mfrc522_reg_irq_idle, true);
    CHECK_EQUAL(mfrc522_drv_status_nullptr, status);
}

TEST(TestMfrc522DrvIrq, mfrc522_drv_irq_en__IrqAllPassedAsAnArgument__ErrorIsReturned)
{
    auto conf = initDevice();

    auto status = mfrc522_drv_irq_en(&conf, mfrc522_reg_irq_all, true);
    CHECK_EQUAL(mfrc522_drv_status_nok, status);
}

TEST(TestMfrc522DrvIrq, mfrc522_drv_irq_en__EnableIrqInComReg__Success)
{
    auto conf = initDevice();

    /* Pretend that some IRQs are enabled */
    u8 actualIrqs = 0x41;
    llCallParams.push_back(READ(1, mfrc522_reg_com_irq_en, actualIrqs));
    llCallParams.push_back(WRITE1(1, mfrc522_reg_com_irq_en, (u8)(actualIrqs | (1 << mfrc522_reg_irq_idle))));
    mfrc522UpdateLowLevelExpectations(llCallParams);

    auto status = mfrc522_drv_irq_en(&conf, mfrc522_reg_irq_idle, true);
    CHECK_EQUAL(mfrc522_drv_status_ok, status);
}

TEST(TestMfrc522DrvIrq, mfrc522_drv_irq_en__DisableIrqInComReg__Success)
{
    auto conf = initDevice();

    /* Pretend that IRQ was enabled, this 0x00 should be written back */
    u8 actualIrqs = 1 << mfrc522_reg_irq_idle;
    llCallParams.push_back(READ(1, mfrc522_reg_com_irq_en, actualIrqs));
    llCallParams.push_back(WRITE1(1, mfrc522_reg_com_irq_en, 0x00));
    mfrc522UpdateLowLevelExpectations(llCallParams);

    auto status = mfrc522_drv_irq_en(&conf, mfrc522_reg_irq_idle, false);
    CHECK_EQUAL(mfrc522_drv_status_ok, status);
}

TEST(TestMfrc522DrvIrq, mfrc522_drv_irq_en__EnableIrqInDivReg__Success)
{
    auto conf = initDevice();

    llCallParams.push_back(READ(1, mfrc522_reg_div_irq_en, 0));
    llCallParams.push_back(WRITE1(1, mfrc522_reg_div_irq_en, 1 << (mfrc522_reg_irq_crc & ~MFRC522_REG_IRQ_DIV)));
    mfrc522UpdateLowLevelExpectations(llCallParams);

    auto status = mfrc522_drv_irq_en(&conf, mfrc522_reg_irq_crc, true);
    CHECK_EQUAL(mfrc522_drv_status_ok, status);
}

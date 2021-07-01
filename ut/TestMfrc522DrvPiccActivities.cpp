#include "mfrc522_drv.h"
#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "common/TestCommon.h"
#include "common/Mockable.h" /* Provides mocks */

using namespace testing;

/* ------------------------------------------------------------ */
/* ---------------------- Custom matchers --------------------- */
/* ------------------------------------------------------------ */

/* Matcher to check if configuration struct for transceive function contains valid fields */
MATCHER_P(TransceiveStructInputMatcher, expected, "Transceive struct input matcher")
{
    return (arg->tx_data_sz == expected->tx_data_sz) && /* Compare TX sizes */
           (arg->rx_data_sz == expected->rx_data_sz) &&  /* Compare RX sizes */
           !memcmp(arg->tx_data, expected->tx_data, expected->tx_data_sz) && /* Compare TX data */
           (arg->command == expected->command); /* Compare commands */
}

/* ------------------------------------------------------------ */
/* ----------------------- Custom Actions --------------------- */
/* ------------------------------------------------------------ */

/* Action to set RX bytes inside mocked transceive function */
ACTION_P(TransceiveAction, expected)
{
    /* Copy RX data from expected structure */
    memcpy(arg1->rx_data, expected->rx_data, expected->rx_data_sz);
}

/* ------------------------------------------------------------ */
/* ------------------------ Test cases ------------------------ */
/* ------------------------------------------------------------ */

TEST(TestMfrc522DrvCommon, mfrc522_drv_transceive__NullCases)
{
    auto device = initDevice();
    mfrc522_drv_transceive_conf transceiveConf;

    auto status = mfrc522_drv_transceive(&device, nullptr);
    ASSERT_EQ(mfrc522_drv_status_nullptr, status);
    status = mfrc522_drv_transceive(nullptr, &transceiveConf);
    ASSERT_EQ(mfrc522_drv_status_nullptr, status);
}

TEST(TestMfrc522DrvCommon, mfrc522_drv_transceive__InvalidCommand)
{
    auto device = initDevice();
    /* Do not fill any other fields - needless in this test */
    mfrc522_drv_transceive_conf transceiveConf;
    transceiveConf.command = mfrc522_reg_cmd_idle;

    auto status = mfrc522_drv_transceive(&device, &transceiveConf);
    ASSERT_EQ(mfrc522_drv_status_nok, status);
}

TEST(TestMfrc522DrvCommon, mfrc522_drv_transceive__NoIrqAfterAllRetries__TimeoutError)
{
    auto device = initDevice();

    /* Populate configuration struct */
    u8 tx = 0xCF;
    u8 rx = 0xAA;
    mfrc522_drv_transceive_conf transceiveConf;
    transceiveConf.tx_data = &tx;
    transceiveConf.tx_data_sz = 1;
    transceiveConf.rx_data = &rx;
    transceiveConf.rx_data_sz = 1;
    transceiveConf.command = mfrc522_reg_cmd_transceive;

    /* Set expectations */
    MOCK(mfrc522_ll_send);
    MOCK(mfrc522_ll_recv);
    MOCK(mfrc522_drv_irq_states);
    MOCK(mfrc522_drv_invoke_cmd);
    MOCK(mfrc522_drv_irq_clr);
    IGNORE_REDUNDANT_LL_RECV_CALLS();
    InSequence s;
    /* All IRQs shall be cleared */
    MOCK_CALL(mfrc522_drv_irq_clr, &device, mfrc522_reg_irq_all).WillOnce(Return(mfrc522_drv_status_ok));
    /* FIFO buffer should be flushed and populated with new data */
    MOCK_CALL(mfrc522_ll_send, mfrc522_reg_fifo_level, 1, NotNull()).WillOnce(Return(mfrc522_ll_status_ok));
    MOCK_CALL(mfrc522_ll_send, mfrc522_reg_fifo_data, 1, Pointee(tx)).WillOnce(Return(mfrc522_ll_status_ok));
    /* Start transceive command and transmission of data */
    MOCK_CALL(mfrc522_drv_invoke_cmd, &device, mfrc522_reg_cmd_transceive).WillOnce(Return(mfrc522_drv_status_ok));
    MOCK_CALL(mfrc522_ll_send, mfrc522_reg_bit_framing, 1, NotNull()).WillOnce(Return(mfrc522_ll_status_ok));
    /* Simulate that no IRQ is set (states = 0x0000) */
    MOCK_CALL(mfrc522_drv_irq_states, &device, NotNull()).Times(AtLeast(1))
            .WillRepeatedly(DoAll(SetArgPointee<1>(0x0000), Return(mfrc522_drv_status_ok)));
    /* Stop transmission of data and enter Idle state back */
    MOCK_CALL(mfrc522_ll_send, mfrc522_reg_bit_framing, 1, NotNull()).WillOnce(Return(mfrc522_ll_status_ok));
    MOCK_CALL(mfrc522_drv_invoke_cmd, &device, mfrc522_reg_cmd_idle).WillOnce(Return(mfrc522_drv_status_ok));

    /* Check results */
    auto status = mfrc522_drv_transceive(&device, &transceiveConf);
    ASSERT_EQ(mfrc522_drv_status_transceive_timeout, status);
    ASSERT_EQ(0xAA, rx); /* RX data shall not be touched */
}

TEST(TestMfrc522DrvCommon, mfrc522_drv_transceive__ErrorBitIsSet__ErrorReturned)
{
    auto device = initDevice();

    /* Populate configuration struct */
    u8 tx = 0xCF;
    u8 rx = 0xAA;
    mfrc522_drv_transceive_conf transceiveConf;
    transceiveConf.tx_data = &tx;
    transceiveConf.tx_data_sz = 1;
    transceiveConf.rx_data = &rx;
    transceiveConf.rx_data_sz = 1;
    transceiveConf.command = mfrc522_reg_cmd_transceive;

    /* Set expectations */
    MOCK(mfrc522_ll_send);
    MOCK(mfrc522_ll_recv);
    MOCK(mfrc522_drv_irq_states);
    MOCK(mfrc522_drv_invoke_cmd);
    MOCK(mfrc522_drv_irq_clr);
    IGNORE_REDUNDANT_LL_RECV_CALLS();
    InSequence s;
    /* All IRQs shall be cleared */
    MOCK_CALL(mfrc522_drv_irq_clr, &device, mfrc522_reg_irq_all).WillOnce(Return(mfrc522_drv_status_ok));
    /* FIFO buffer should be flushed and populated with new data */
    MOCK_CALL(mfrc522_ll_send, mfrc522_reg_fifo_level, 1, NotNull()).WillOnce(Return(mfrc522_ll_status_ok));
    MOCK_CALL(mfrc522_ll_send, mfrc522_reg_fifo_data, 1, Pointee(tx)).WillOnce(Return(mfrc522_ll_status_ok));
    /* Start transceive command and transmission of data */
    MOCK_CALL(mfrc522_drv_invoke_cmd, &device, mfrc522_reg_cmd_transceive).WillOnce(Return(mfrc522_drv_status_ok));
    MOCK_CALL(mfrc522_ll_send, mfrc522_reg_bit_framing, 1, NotNull()).WillOnce(Return(mfrc522_ll_status_ok));
    /* Simulate that error IRQ is set (low byte = 0x02) */
    MOCK_CALL(mfrc522_drv_irq_states, &device, NotNull()).Times(AtLeast(1))
            .WillRepeatedly(DoAll(SetArgPointee<1>(0x0002), Return(mfrc522_drv_status_ok)));
    /* Stop transmission of data and enter Idle state back */
    MOCK_CALL(mfrc522_ll_send, mfrc522_reg_bit_framing, 1, NotNull()).WillOnce(Return(mfrc522_ll_status_ok));
    MOCK_CALL(mfrc522_drv_invoke_cmd, &device, mfrc522_reg_cmd_idle).WillOnce(Return(mfrc522_drv_status_ok));

    /* Check results */
    auto status = mfrc522_drv_transceive(&device, &transceiveConf);
    ASSERT_EQ(mfrc522_drv_status_transceive_err, status);
    ASSERT_EQ(0xAA, rx); /* RX data shall not be touched */
}

TEST(TestMfrc522DrvCommon, mfrc522_drv_transceive__InvalidNumberOfRxValidBits__Error)
{
    auto device = initDevice();

    /* Populate configuration struct */
    u8 tx = 0xCF;
    u8 rx = 0xAA;
    mfrc522_drv_transceive_conf transceiveConf;
    transceiveConf.tx_data = &tx;
    transceiveConf.tx_data_sz = 1;
    transceiveConf.rx_data = &rx;
    transceiveConf.rx_data_sz = 1;
    transceiveConf.command = mfrc522_reg_cmd_transceive;

    /* Set expectations */
    MOCK(mfrc522_ll_send);
    MOCK(mfrc522_ll_recv);
    MOCK(mfrc522_drv_irq_states);
    MOCK(mfrc522_drv_invoke_cmd);
    MOCK(mfrc522_drv_irq_clr);
    IGNORE_REDUNDANT_LL_RECV_CALLS();
    InSequence s;
    /* All IRQs shall be cleared */
    MOCK_CALL(mfrc522_drv_irq_clr, &device, mfrc522_reg_irq_all).WillOnce(Return(mfrc522_drv_status_ok));
    /* FIFO buffer should be flushed and populated with new data */
    MOCK_CALL(mfrc522_ll_send, mfrc522_reg_fifo_level, 1, NotNull()).WillOnce(Return(mfrc522_ll_status_ok));
    MOCK_CALL(mfrc522_ll_send, mfrc522_reg_fifo_data, 1, Pointee(tx)).WillOnce(Return(mfrc522_ll_status_ok));
    /* Start transceive command and transmission of data */
    MOCK_CALL(mfrc522_drv_invoke_cmd, &device, mfrc522_reg_cmd_transceive).WillOnce(Return(mfrc522_drv_status_ok));
    MOCK_CALL(mfrc522_ll_send, mfrc522_reg_bit_framing, 1, NotNull()).WillOnce(Return(mfrc522_ll_status_ok));
    MOCK_CALL(mfrc522_drv_irq_states, &device, NotNull()).Times(AtLeast(1))
            .WillRepeatedly(DoAll(SetArgPointee<1>(1 << 5), Return(mfrc522_drv_status_ok)));
    /* Stop transmission of data and enter Idle state back */
    MOCK_CALL(mfrc522_ll_send, mfrc522_reg_bit_framing, 1, NotNull()).WillOnce(Return(mfrc522_ll_status_ok));
    MOCK_CALL(mfrc522_drv_invoke_cmd, &device, mfrc522_reg_cmd_idle).WillOnce(Return(mfrc522_drv_status_ok));
    /* Simulate that RX last bits = 0x01 */
    MOCK_CALL(mfrc522_ll_recv, mfrc522_reg_control, NotNull())
            .WillOnce(DoAll(SetArgPointee<1>(0x01), Return(mfrc522_ll_status_ok)));

    /* Check results */
    auto status = mfrc522_drv_transceive(&device, &transceiveConf);
    ASSERT_EQ(mfrc522_drv_status_transceive_rx_mism, status);
    ASSERT_EQ(0xAA, rx); /* RX data shall not be touched */
}

TEST(TestMfrc522DrvCommon, mfrc522_drv_transceive__NoDataOnRxSide__Error)
{
    auto device = initDevice();

    /* Populate configuration struct */
    u8 tx = 0xCF;
    u8 rx = 0xAA;
    mfrc522_drv_transceive_conf transceiveConf;
    transceiveConf.tx_data = &tx;
    transceiveConf.tx_data_sz = 1;
    transceiveConf.rx_data = &rx;
    transceiveConf.rx_data_sz = 1;
    transceiveConf.command = mfrc522_reg_cmd_transceive;

    /* Set expectations */
    MOCK(mfrc522_ll_send);
    MOCK(mfrc522_ll_recv);
    MOCK(mfrc522_drv_irq_states);
    MOCK(mfrc522_drv_invoke_cmd);
    MOCK(mfrc522_drv_irq_clr);
    IGNORE_REDUNDANT_LL_RECV_CALLS();
    InSequence s;
    /* All IRQs shall be cleared */
    MOCK_CALL(mfrc522_drv_irq_clr, &device, mfrc522_reg_irq_all).WillOnce(Return(mfrc522_drv_status_ok));
    /* FIFO buffer should be flushed and populated with new data */
    MOCK_CALL(mfrc522_ll_send, mfrc522_reg_fifo_level, 1, NotNull()).WillOnce(Return(mfrc522_ll_status_ok));
    MOCK_CALL(mfrc522_ll_send, mfrc522_reg_fifo_data, 1, Pointee(tx)).WillOnce(Return(mfrc522_ll_status_ok));
    /* Start transceive command and transmission of data */
    MOCK_CALL(mfrc522_drv_invoke_cmd, &device, mfrc522_reg_cmd_transceive).WillOnce(Return(mfrc522_drv_status_ok));
    MOCK_CALL(mfrc522_ll_send, mfrc522_reg_bit_framing, 1, NotNull()).WillOnce(Return(mfrc522_ll_status_ok));
    MOCK_CALL(mfrc522_drv_irq_states, &device, NotNull()).Times(AtLeast(1))
            .WillRepeatedly(DoAll(SetArgPointee<1>(1 << 5), Return(mfrc522_drv_status_ok)));
    /* Stop transmission of data and enter Idle state back */
    MOCK_CALL(mfrc522_ll_send, mfrc522_reg_bit_framing, 1, NotNull()).WillOnce(Return(mfrc522_ll_status_ok));
    MOCK_CALL(mfrc522_drv_invoke_cmd, &device, mfrc522_reg_cmd_idle).WillOnce(Return(mfrc522_drv_status_ok));
    /* Get number of RX last bits and FIFO level */
    MOCK_CALL(mfrc522_ll_recv, mfrc522_reg_control, NotNull())
            .WillOnce(DoAll(SetArgPointee<1>(0x00), Return(mfrc522_ll_status_ok)));
    MOCK_CALL(mfrc522_ll_recv, mfrc522_reg_fifo_level, NotNull())
            .WillOnce(DoAll(SetArgPointee<1>(0x00), Return(mfrc522_ll_status_ok))); /* Lack of RX bytes */

    /* Check results */
    auto status = mfrc522_drv_transceive(&device, &transceiveConf);
    ASSERT_EQ(mfrc522_drv_status_transceive_rx_mism, status);
    ASSERT_EQ(0xAA, rx); /* RX data shall not be touched */
}

TEST(TestMfrc522DrvCommon, mfrc522_drv_transceive__TypicalCase__Success)
{
    auto device = initDevice();

    /* Populate configuration struct */
    u8 tx = 0xCF;
    u8 rx[2] = {0x00};
    mfrc522_drv_transceive_conf transceiveConf;
    transceiveConf.tx_data = &tx;
    transceiveConf.tx_data_sz = 1;
    transceiveConf.rx_data = &rx[0];
    transceiveConf.rx_data_sz = 2;
    transceiveConf.command = mfrc522_reg_cmd_transceive;

    /* Set expectations */
    MOCK(mfrc522_ll_send);
    MOCK(mfrc522_ll_recv);
    MOCK(mfrc522_drv_irq_states);
    MOCK(mfrc522_drv_invoke_cmd);
    MOCK(mfrc522_drv_irq_clr);
    IGNORE_REDUNDANT_LL_RECV_CALLS();
    InSequence s;
    /* All IRQs shall be cleared */
    MOCK_CALL(mfrc522_drv_irq_clr, &device, mfrc522_reg_irq_all).WillOnce(Return(mfrc522_drv_status_ok));
    /* FIFO buffer should be flushed and populated with new data */
    MOCK_CALL(mfrc522_ll_send, mfrc522_reg_fifo_level, 1, NotNull()).WillOnce(Return(mfrc522_ll_status_ok));
    MOCK_CALL(mfrc522_ll_send, mfrc522_reg_fifo_data, 1, Pointee(tx)).WillOnce(Return(mfrc522_ll_status_ok));
    /* Start transceive command and transmission of data */
    MOCK_CALL(mfrc522_drv_invoke_cmd, &device, mfrc522_reg_cmd_transceive).WillOnce(Return(mfrc522_drv_status_ok));
    MOCK_CALL(mfrc522_ll_send, mfrc522_reg_bit_framing, 1, NotNull()).WillOnce(Return(mfrc522_ll_status_ok));
    MOCK_CALL(mfrc522_drv_irq_states, &device, NotNull()).Times(AtLeast(1))
            .WillRepeatedly(DoAll(SetArgPointee<1>(1 << 5), Return(mfrc522_drv_status_ok)));
    /* Stop transmission of data and enter Idle state back */
    MOCK_CALL(mfrc522_ll_send, mfrc522_reg_bit_framing, 1, NotNull()).WillOnce(Return(mfrc522_ll_status_ok));
    MOCK_CALL(mfrc522_drv_invoke_cmd, &device, mfrc522_reg_cmd_idle).WillOnce(Return(mfrc522_drv_status_ok));
    /* Get number of RX last bits and FIFO level */
    MOCK_CALL(mfrc522_ll_recv, mfrc522_reg_control, NotNull())
            .WillOnce(DoAll(SetArgPointee<1>(0x00), Return(mfrc522_ll_status_ok)));
    MOCK_CALL(mfrc522_ll_recv, mfrc522_reg_fifo_level, NotNull())
            .WillOnce(DoAll(SetArgPointee<1>(0x02), Return(mfrc522_ll_status_ok)));
    /* Read RX data */
    MOCK_CALL(mfrc522_ll_recv, mfrc522_reg_fifo_data, NotNull())
            .WillOnce(DoAll(SetArgPointee<1>(0xBB), Return(mfrc522_ll_status_ok)))
            .WillOnce(DoAll(SetArgPointee<1>(0xCC), Return(mfrc522_ll_status_ok)));

    /* Check results */
    auto status = mfrc522_drv_transceive(&device, &transceiveConf);
    ASSERT_EQ(mfrc522_drv_status_ok, status);
    ASSERT_EQ(0xBB, rx[0]);
    ASSERT_EQ(0xCC, rx[1]);
}

TEST(TestMfrc522DrvCommon, mfrc522_drv_reqa__NullCases)
{
    auto device = initDevice();
    u16 atqa;

    auto status = mfrc522_drv_reqa(&device, nullptr);
    ASSERT_EQ(mfrc522_drv_status_nullptr, status);

    status = mfrc522_drv_reqa(nullptr, &atqa);
    ASSERT_EQ(mfrc522_drv_status_nullptr, status);
}

TEST(TestMfrc522DrvCommon, mfrc522_drv_reqa__TransceiveError__InvalidATQAReturned)
{
    auto device = initDevice();

    /* Set the same expectations for each test case */
    MOCK(mfrc522_ll_send);
    MOCK(mfrc522_drv_transceive);
    /* TX last bits should be set to 0x07 in each test cases */
    MOCK_CALL(mfrc522_ll_send, mfrc522_reg_bit_framing, 1, Pointee(0x07)).Times(3)
            .WillRepeatedly(Return(mfrc522_ll_status_ok));
    /* Simulate failure in transceive process */
    MOCK_CALL(mfrc522_drv_transceive, &device, NotNull())
            .WillOnce(Return(mfrc522_drv_status_transceive_timeout))
            .WillOnce(Return(mfrc522_drv_status_transceive_err))
            .WillOnce(Return(mfrc522_drv_status_transceive_rx_mism));

    /* Case 1 */
    u16 atqa1;
    auto status1 = mfrc522_drv_reqa(&device, &atqa1);
    ASSERT_EQ(mfrc522_drv_status_transceive_timeout, status1);
    ASSERT_EQ(MFRC522_PICC_ATQA_INV, atqa1);

    /* Case 2 */
    u16 atqa2;
    auto status2 = mfrc522_drv_reqa(&device, &atqa2);
    ASSERT_EQ(mfrc522_drv_status_transceive_err, status2);
    ASSERT_EQ(MFRC522_PICC_ATQA_INV, atqa2);

    /* Case 3 */
    u16 atqa3;
    auto status3 = mfrc522_drv_reqa(&device, &atqa3);
    ASSERT_EQ(mfrc522_drv_status_transceive_rx_mism, status3);
    ASSERT_EQ(MFRC522_PICC_ATQA_INV, atqa3);
}

TEST(TestMfrc522DrvCommon, mfrc522_drv_reqa__TypicalCase__Success)
{
    auto device = initDevice();
    u16 atqa;

    /* Set expectations */
    MOCK(mfrc522_ll_send);
    MOCK(mfrc522_drv_transceive);
    InSequence s;
    u8 txData[1] = {mfrc522_picc_cmd_reqa}; /* TX data (expected input to mocked function) */
    u8 rxData[2] = {0x04, 0x00}; /* RX data (expected output from mocked function) */
    mfrc522_drv_transceive_conf transceiveConf;
    transceiveConf.tx_data = &txData[0];
    transceiveConf.tx_data_sz = 1;
    transceiveConf.rx_data = &rxData[0];
    transceiveConf.rx_data_sz = 2;
    transceiveConf.command = mfrc522_reg_cmd_transceive;
    /* TX last bits should be set to 0x07 */
    MOCK_CALL(mfrc522_ll_send, mfrc522_reg_bit_framing, 1, Pointee(0x07)).WillOnce(Return(mfrc522_ll_status_ok));
    MOCK_CALL(mfrc522_drv_transceive, &device, TransceiveStructInputMatcher(&transceiveConf))
            .WillOnce(DoAll(TransceiveAction(&transceiveConf), Return(mfrc522_drv_status_ok)));

    auto status = mfrc522_drv_reqa(&device, &atqa);
    ASSERT_EQ(mfrc522_drv_status_ok, status);
    ASSERT_EQ(0x0004, atqa);
}

TEST(TestMfrc522DrvCommon, mfrc522_drv_reqa__BlacklistedAtqaReturned__Error)
{
    auto device = initDevice();
    device.atqa_verify_fn = piccAccept0x0004; /* Accept PICC with 0x0004 ATQA response */
    u16 atqa;

    /* Set expectations */
    MOCK(mfrc522_ll_send);
    MOCK(mfrc522_drv_transceive);
    InSequence s;
    u8 txData[1] = {mfrc522_picc_cmd_reqa}; /* TX data (expected input to mocked function) */
    u8 rxData[2] = {0xFF, 0xAA}; /* RX data (expected output from mocked function) */
    mfrc522_drv_transceive_conf transceiveConf;
    transceiveConf.tx_data = &txData[0];
    transceiveConf.tx_data_sz = 1;
    transceiveConf.rx_data = &rxData[0];
    transceiveConf.rx_data_sz = 2;
    transceiveConf.command = mfrc522_reg_cmd_transceive;
    /* TX last bits should be set to 0x07 */
    MOCK_CALL(mfrc522_ll_send, mfrc522_reg_bit_framing, 1, Pointee(0x07)).WillOnce(Return(mfrc522_ll_status_ok));
    MOCK_CALL(mfrc522_drv_transceive, &device, TransceiveStructInputMatcher(&transceiveConf))
            .WillOnce(DoAll(TransceiveAction(&transceiveConf), Return(mfrc522_drv_status_ok)));

    auto status = mfrc522_drv_reqa(&device, &atqa);
    ASSERT_EQ(mfrc522_drv_status_picc_vrf_err, status);
    ASSERT_EQ(0xAAFF, atqa);
}

TEST(TestMfrc522DrvCommon, mfrc522_drv_anticollision__NullCases)
{
    auto device = initDevice();
    u8 serial[5];

    auto status = mfrc522_drv_anticollision(nullptr, &serial[0]);
    ASSERT_EQ(mfrc522_drv_status_nullptr, status);
    status = mfrc522_drv_anticollision(&device, nullptr);
    ASSERT_EQ(mfrc522_drv_status_nullptr, status);
}

TEST(TestMfrc522DrvCommon, mfrc522_drv_anticollision__ErrorDuringTransceiveCommand__Error)
{
    auto device = initDevice();
    u8 serial[5] = {0x00};

    /* Set expectations */
    MOCK(mfrc522_ll_send);
    MOCK(mfrc522_drv_transceive);
    /* TX last bits = 0x00 (whole byte valid) */
    MOCK_CALL(mfrc522_ll_send, mfrc522_reg_bit_framing, 1, Pointee(0x00)).WillOnce(Return(mfrc522_ll_status_ok));
    /* Simulate an error during transmission/reception of the data */
    MOCK_CALL(mfrc522_drv_transceive, &device, NotNull()).WillOnce(Return(mfrc522_drv_status_transceive_rx_mism));

    auto status = mfrc522_drv_anticollision(&device, &serial[0]);
    ASSERT_EQ(mfrc522_drv_status_transceive_rx_mism, status); /* The error should be forwarded as an output */
    for (const auto& byte : serial) {
        ASSERT_EQ(0x00, byte); /* Serial data shall not be touched */
    }
}

TEST(TestMfrc522DrvCommon, mfrc522_drv_anticollision__InvalidChecksum__Error)
{
    auto device = initDevice();
    u8 serial[5] = {0x00};

    /* Set expectations */
    MOCK(mfrc522_ll_send);
    MOCK(mfrc522_drv_transceive);
    u8 txData[2] = {0x93, 0x20};
    u8 rxData[5] = {0x73, 0xEF, 0xD7, 0x18, 0xAB}; /* 0xAB is invalid in this case */
    mfrc522_drv_transceive_conf transceiveConf;
    transceiveConf.tx_data = &txData[0];
    transceiveConf.tx_data_sz = 2;
    transceiveConf.rx_data = &rxData[0];
    transceiveConf.rx_data_sz = 5;
    transceiveConf.command = mfrc522_reg_cmd_transceive;
    /* TX last bits = 0x00 (whole byte valid) */
    MOCK_CALL(mfrc522_ll_send, mfrc522_reg_bit_framing, 1, Pointee(0x00)).WillOnce(Return(mfrc522_ll_status_ok));
    MOCK_CALL(mfrc522_drv_transceive, &device, TransceiveStructInputMatcher(&transceiveConf))
            .WillOnce(DoAll(TransceiveAction(&transceiveConf), Return(mfrc522_drv_status_ok)));

    auto status = mfrc522_drv_anticollision(&device, &serial[0]);
    ASSERT_EQ(mfrc522_drv_status_anticoll_chksum_err, status);
}

TEST(TestMfrc522DrvCommon, mfrc522_drv_anticollision__ValidNUIDAndChecksumReturned__Success)
{
    /*
     * Example valid serial outputs after anticollision command:
     * 1) 0x73 0xef 0xd7 0x18 0x53
     * 2) 0x83 0x35 0x19 0x16 0xb9
     */

    auto device = initDevice();
    u8 serial[5] = {0x00};

    /* Set expectations */
    MOCK(mfrc522_ll_send);
    MOCK(mfrc522_drv_transceive);
    u8 txData[2] = {0x93, 0x20};
    u8 rxData[5] = {0x73, 0xEF, 0xD7, 0x18, 0x53};
    mfrc522_drv_transceive_conf transceiveConf;
    transceiveConf.tx_data = &txData[0];
    transceiveConf.tx_data_sz = 2;
    transceiveConf.rx_data = &rxData[0];
    transceiveConf.rx_data_sz = 5;
    transceiveConf.command = mfrc522_reg_cmd_transceive;
    /* TX last bits = 0x00 (whole byte valid) */
    MOCK_CALL(mfrc522_ll_send, mfrc522_reg_bit_framing, 1, Pointee(0x00)).WillOnce(Return(mfrc522_ll_status_ok));
    MOCK_CALL(mfrc522_drv_transceive, &device, TransceiveStructInputMatcher(&transceiveConf))
            .WillOnce(DoAll(TransceiveAction(&transceiveConf), Return(mfrc522_drv_status_ok)));

    auto status = mfrc522_drv_anticollision(&device, &serial[0]);
    ASSERT_EQ(mfrc522_drv_status_ok, status);
    ASSERT_EQ(0, memcmp(&serial[0], &rxData[0], 5)); /* Compare output serial with expected one */
}

TEST(TestMfrc522DrvCommon, mfrc522_drv_select__NullCases)
{
    auto device = initDevice();
    u8 serial[5] = {0x00};
    u8 sak;

    auto status = mfrc522_drv_select(nullptr, &serial[0], &sak);
    ASSERT_EQ(mfrc522_drv_status_nullptr, status);
    status = mfrc522_drv_select(&device, nullptr, &sak);
    ASSERT_EQ(mfrc522_drv_status_nullptr, status);
    status = mfrc522_drv_select(&device, &serial[0], nullptr);
    ASSERT_EQ(mfrc522_drv_status_nullptr, status);
}

TEST(TestMfrc522DrvCommon, mfrc522_drv_select__CRCDoesNotMatchUp__Error)
{
    auto device = initDevice();
    u8 serial[5] = {0x73, 0xEF, 0xD7, 0x18, 0x53}; /* Got from PICC */
    u8 sak;

    /* Set expectations */
    MOCK(mfrc522_drv_transceive);
    MOCK(mfrc522_drv_crc_compute);
    u8 tx[9] =
    {
        0x93, 0x70, /* General command */
        0x73, 0xEF, 0xD7, 0x18, 0x53, /* Serial data */
        0x95, 0xEF /* CRC */
    };
    u8 rx[3] =
    {
        0x08, /* SAK */
        0xB6, 0xDD /* CRC */
    };
    mfrc522_drv_transceive_conf transceiveConf;
    transceiveConf.tx_data = &tx[0];
    transceiveConf.tx_data_sz = SIZE_ARRAY(tx);
    transceiveConf.rx_data = &rx[0];
    transceiveConf.rx_data_sz = SIZE_ARRAY(rx);
    transceiveConf.command = mfrc522_reg_cmd_transceive;
    InSequence s;
    /* Compute CRC of TX data */
    MOCK_CALL(mfrc522_drv_crc_compute, &device, NotNull())
            .WillOnce(DoAll(SetArgPointee<1>(0xEF95), Return(mfrc522_drv_status_ok)));
    /* Transceive the data */
    MOCK_CALL(mfrc522_drv_transceive, &device, TransceiveStructInputMatcher(&transceiveConf))
            .WillOnce(DoAll(TransceiveAction(&transceiveConf), Return(mfrc522_drv_status_ok)));
    /* Compute CRC of RX data */
    MOCK_CALL(mfrc522_drv_crc_compute, &device, NotNull())
            .WillOnce(DoAll(SetArgPointee<1>(0xFFFF), Return(mfrc522_drv_status_ok))); /* Return wrong value */

    auto status = mfrc522_drv_select(&device, &serial[0], &sak);
    ASSERT_EQ(mfrc522_drv_status_crc_err, status);
}

TEST(TestMfrc522DrvCommon, mfrc522_drv_select__TypicalCase__Success)
{
    auto device = initDevice();
    u8 serial[5] = {0x73, 0xEF, 0xD7, 0x18, 0x53}; /* Got from PICC */
    u8 sak;

    /* Set expectations */
    MOCK(mfrc522_drv_transceive);
    MOCK(mfrc522_drv_crc_compute);
    u8 tx[9] =
    {
        0x93, 0x70, /* General command */
        0x73, 0xEF, 0xD7, 0x18, 0x53, /* Serial data */
        0x95, 0xEF /* CRC */
    };
    u8 rx[3] =
    {
        0x08, /* SAK */
        0xB6, 0xDD /* CRC */
    };
    mfrc522_drv_transceive_conf transceiveConf;
    transceiveConf.tx_data = &tx[0];
    transceiveConf.tx_data_sz = SIZE_ARRAY(tx);
    transceiveConf.rx_data = &rx[0];
    transceiveConf.rx_data_sz = SIZE_ARRAY(rx);
    transceiveConf.command = mfrc522_reg_cmd_transceive;
    InSequence s;
    /* Compute CRC of TX data */
    MOCK_CALL(mfrc522_drv_crc_compute, &device, NotNull())
            .WillOnce(DoAll(SetArgPointee<1>(0xEF95), Return(mfrc522_drv_status_ok)));
    /* Transceive the data */
    MOCK_CALL(mfrc522_drv_transceive, &device, TransceiveStructInputMatcher(&transceiveConf))
            .WillOnce(DoAll(TransceiveAction(&transceiveConf), Return(mfrc522_drv_status_ok)));
    /* Compute CRC of RX data */
    MOCK_CALL(mfrc522_drv_crc_compute, &device, NotNull())
            .WillOnce(DoAll(SetArgPointee<1>(0xDDB6), Return(mfrc522_drv_status_ok))); /* Return wrong value */

    auto status = mfrc522_drv_select(&device, &serial[0], &sak);
    ASSERT_EQ(mfrc522_drv_status_ok, status);
    ASSERT_EQ(0x08, sak);
}

TEST(TestMfrc522DrvCommon, mfrc522_drv_authenticate__NullCases)
{
    auto device = initDevice();
    mfrc522_drv_auth_conf authConf;

    auto status = mfrc522_drv_authenticate(&device, nullptr);
    ASSERT_EQ(mfrc522_drv_status_nullptr, status);
    status = mfrc522_drv_authenticate(nullptr, &authConf);
    ASSERT_EQ(mfrc522_drv_status_nullptr, status);
}

TEST(TestMfrc522DrvCommon, mfrc522_drv_authenticate__TypicalCase__Success)
{
    auto device = initDevice();

    /* Provided as the input */
    mfrc522_drv_auth_conf authConf;
    u8 key[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
    u8 serial[5] = {0x73, 0xEF, 0xD7, 0x18, 0x53}; /* Got from PICC */
    authConf.key = &key[0];
    authConf.block = mfrc522_picc_block0;
    authConf.sector = mfrc522_picc_sector0;
    authConf.key_type = mfrc522_picc_key_a;
    authConf.serial = &serial[0];

    /* Expected parameters passed to transceive command */
    u8 tx[12] =
    {
        mfrc522_picc_key_a, /* Key type */
        0, /* Block number */
        0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, /* Secret key */
        0x73, 0xEF, 0xD7, 0x18, /* Serial number without checksum */
    };
    mfrc522_drv_transceive_conf transceiveConf;
    transceiveConf.tx_data = &tx[0];
    transceiveConf.tx_data_sz = SIZE_ARRAY(tx);
    transceiveConf.rx_data = nullptr;
    transceiveConf.rx_data_sz = 0;
    transceiveConf.command = mfrc522_reg_cmd_authent;

    /* Set expectations */
    MOCK(mfrc522_drv_transceive);
    MOCK(mfrc522_ll_recv);
    InSequence s;
    /* Transceive the data */
    MOCK_CALL(mfrc522_drv_transceive, &device, TransceiveStructInputMatcher(&transceiveConf))
            .WillOnce(DoAll(TransceiveAction(&transceiveConf), Return(mfrc522_drv_status_ok)));
    /* Check is crypto was enabled */
    MOCK_CALL(mfrc522_ll_recv, mfrc522_reg_status2, NotNull())
            .WillOnce(DoAll(SetArgPointee<1>(1 << 3), Return(mfrc522_ll_status_ok)));

    auto status = mfrc522_drv_authenticate(&device, &authConf);
    ASSERT_EQ(mfrc522_drv_status_ok, status);
}

TEST(TestMfrc522DrvCommon, mfrc522_drv_authenticate__CryptoNotEnabled__Error)
{
    auto device = initDevice();

    /* Provided as the input */
    mfrc522_drv_auth_conf authConf;
    u8 key[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
    u8 serial[5] = {0x73, 0xEF, 0xD7, 0x18, 0x53}; /* Got from PICC */
    authConf.key = &key[0];
    authConf.block = mfrc522_picc_block0;
    authConf.sector = mfrc522_picc_sector0;
    authConf.key_type = mfrc522_picc_key_a;
    authConf.serial = &serial[0];

    /* Expected parameters passed to transceive command */
    u8 tx[12] =
    {
        mfrc522_picc_key_a, /* Key type */
        0, /* Block number */
        0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, /* Secret key */
        0x73, 0xEF, 0xD7, 0x18, /* Serial number without checksum */
    };
    mfrc522_drv_transceive_conf transceiveConf;
    transceiveConf.tx_data = &tx[0];
    transceiveConf.tx_data_sz = SIZE_ARRAY(tx);
    transceiveConf.rx_data = nullptr;
    transceiveConf.rx_data_sz = 0;
    transceiveConf.command = mfrc522_reg_cmd_authent;

    /* Set expectations */
    MOCK(mfrc522_drv_transceive);
    MOCK(mfrc522_ll_recv);
    InSequence s;
    /* Transceive the data */
    MOCK_CALL(mfrc522_drv_transceive, &device, TransceiveStructInputMatcher(&transceiveConf))
            .WillOnce(DoAll(TransceiveAction(&transceiveConf), Return(mfrc522_drv_status_ok)));
    /* Simulate that crypto was not enabled even if the previous step returned 'ok' status code */
    MOCK_CALL(mfrc522_ll_recv, mfrc522_reg_status2, NotNull())
            .WillOnce(DoAll(SetArgPointee<1>(0x00), Return(mfrc522_ll_status_ok)));

    auto status = mfrc522_drv_authenticate(&device, &authConf);
    ASSERT_EQ(mfrc522_drv_status_crypto_err, status);
}

TEST(TestMfrc522DrvCommon, mfrc522_drv_halt__NullCases)
{
    auto status = mfrc522_drv_halt(nullptr);
    ASSERT_EQ(mfrc522_drv_status_nullptr, status);
}

TEST(TestMfrc522DrvCommon, mfrc522_drv_halt__TypicalCase__Success)
{
    auto device = initDevice();

    /* Expected parameters */
    u8 tx[4] =
    {
        0x50, 0x00, /* Halt command */
        0x57, 0xCD /* CRC */
    };
    mfrc522_drv_transceive_conf transceiveConf;
    transceiveConf.tx_data = &tx[0];
    transceiveConf.tx_data_sz = SIZE_ARRAY(tx);
    transceiveConf.rx_data = nullptr;
    transceiveConf.rx_data_sz = 0;
    transceiveConf.command = mfrc522_reg_cmd_transceive;

    /* Set expectations */
    MOCK(mfrc522_drv_transceive);
    MOCK(mfrc522_ll_send);
    MOCK(mfrc522_drv_crc_compute);
    IGNORE_REDUNDANT_LL_SEND_CALLS();
    InSequence s;
    /* Calculate CRC */
    MOCK_CALL(mfrc522_drv_crc_compute, &device, NotNull())
            .WillOnce(DoAll(SetArgPointee<1>(0xCD57), Return(mfrc522_drv_status_ok)));
    /* Transceive the data (mfrc522_drv_status_transceive_timeout = success in this case) */
    MOCK_CALL(mfrc522_drv_transceive, &device, TransceiveStructInputMatcher(&transceiveConf))
            .WillOnce(DoAll(TransceiveAction(&transceiveConf), Return(mfrc522_drv_status_transceive_timeout)));
    /* Disable the crypto unit */
    MOCK_CALL(mfrc522_ll_send, mfrc522_reg_status2, 1, Pointee(0x00)).WillOnce(Return(mfrc522_ll_status_ok));

    auto status = mfrc522_drv_halt(&device);
    ASSERT_EQ(mfrc522_drv_status_ok, status);
}

TEST(TestMfrc522DrvCommon, mfrc522_drv_halt__OkStatusAfterTransceiveCommand__Error)
{
    auto device = initDevice();

    /* Expected parameters */
    u8 tx[4] =
    {
        0x50, 0x00, /* Halt command */
        0x57, 0xCD /* CRC */
    };
    mfrc522_drv_transceive_conf transceiveConf;
    transceiveConf.tx_data = &tx[0];
    transceiveConf.tx_data_sz = SIZE_ARRAY(tx);
    transceiveConf.rx_data = nullptr;
    transceiveConf.rx_data_sz = 0;
    transceiveConf.command = mfrc522_reg_cmd_transceive;

    /* Set expectations */
    MOCK(mfrc522_drv_transceive);
    MOCK(mfrc522_ll_send);
    MOCK(mfrc522_drv_crc_compute);
    IGNORE_REDUNDANT_LL_SEND_CALLS();
    InSequence s;
    /* Calculate CRC */
    MOCK_CALL(mfrc522_drv_crc_compute, &device, NotNull())
            .WillOnce(DoAll(SetArgPointee<1>(0xCD57), Return(mfrc522_drv_status_ok)));
    /* Transceive the data (mfrc522_drv_status_transceive_timeout = success in this case) */
    MOCK_CALL(mfrc522_drv_transceive, &device, TransceiveStructInputMatcher(&transceiveConf))
            .WillOnce(DoAll(TransceiveAction(&transceiveConf), Return(mfrc522_drv_status_ok)));

    auto status = mfrc522_drv_halt(&device);
    ASSERT_EQ(mfrc522_drv_status_halt_err, status);
}

TEST(TestMfrc522DrvCommon, mfrc522_drv_halt__ErrorPresentAfterTransceiveCommand__ErrorForwardedAsOutput)
{
    auto device = initDevice();

    /* Expected parameters */
    u8 tx[4] =
    {
        0x50, 0x00, /* Halt command */
        0x57, 0xCD /* CRC */
    };
    mfrc522_drv_transceive_conf transceiveConf;
    transceiveConf.tx_data = &tx[0];
    transceiveConf.tx_data_sz = SIZE_ARRAY(tx);
    transceiveConf.rx_data = nullptr;
    transceiveConf.rx_data_sz = 0;
    transceiveConf.command = mfrc522_reg_cmd_transceive;

    /* Set expectations */
    MOCK(mfrc522_drv_transceive);
    MOCK(mfrc522_ll_send);
    MOCK(mfrc522_drv_crc_compute);
    IGNORE_REDUNDANT_LL_SEND_CALLS();
    InSequence s;
    /* Calculate CRC */
    MOCK_CALL(mfrc522_drv_crc_compute, &device, NotNull())
            .WillOnce(DoAll(SetArgPointee<1>(0xCD57), Return(mfrc522_drv_status_ok)));
    /* Transceive the data (mfrc522_drv_status_transceive_timeout = success in this case) */
    MOCK_CALL(mfrc522_drv_transceive, &device, TransceiveStructInputMatcher(&transceiveConf))
            .WillOnce(DoAll(TransceiveAction(&transceiveConf), Return(mfrc522_drv_status_transceive_err)));

    auto status = mfrc522_drv_halt(&device);
    ASSERT_EQ(mfrc522_drv_status_transceive_err, status);
}
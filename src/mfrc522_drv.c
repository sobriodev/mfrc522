#include "mfrc522_drv.h"
#include "mfrc522_conf.h"

#include <string.h>

/* ------------------------------------------------------------ */
/* ------------------------ Private macros -------------------- */
/* ------------------------------------------------------------ */

/* Timer prescaler when using millisecond as a time unit */
#define TIMER_PRESCALER 1694

/* Masks to handle all possible interrupts */
#define IRQ_ALL_COM_MASK 0x7F
#define IRQ_ALL_DIV_MASK 0x14

/* ------------------------------------------------------------ */
/* ----------------------- Private functions ------------------ */
/* ------------------------------------------------------------ */

/* Private implementation of delay function */
static inline void
delay(const mfrc522_drv_conf* conf, u32 period)
{
#if MFRC522_LL_PTR
#if MFRC522_LL_DELAY
    conf->ll_delay(period);
#else
    /* Make compiler happy */
    (void)conf;
    (void)period;
#endif
#elif MFRC522_LL_DEF
#if MFRC522_LL_DELAY
    (void)conf; /* Make compiler happy */
    mfrc522_ll_delay(period);
#else
    /* Make compiler happy */
    (void)conf;
    (void)period;
#endif
#endif
}

/* Calculate real number of retry count */
static inline u32
get_real_retry_count(u32 rc)
{
#if !MFRC522_LL_DELAY
    rc *= MFRC522_CONF_RETRY_CNT_MUL;
#endif
    return rc;
}

/* Function to read valid number of RX bytes during transceive command */
static mfrc522_drv_status
get_valid_rx_bytes(const mfrc522_drv_conf* conf, u8* rx_bytes)
{
    u8 valid_bits;
    mfrc522_drv_status status;
    status = mfrc522_drv_read_masked(conf, mfrc522_reg_control, &valid_bits, MFRC522_REG_FIELD(CONTROL_RX_LASTBITS));
    ERROR_IF_NEQ(status, mfrc522_drv_status_ok);

    /* In current use cases the whole byte shall be valid. Thus return zero by now */
    if (UNLIKELY(valid_bits)) {
        *rx_bytes = 0;
        return mfrc522_drv_status_ok;
    }

    /* Read FIFO level */
    status = mfrc522_drv_read(conf, mfrc522_reg_fifo_level, rx_bytes);
    ERROR_IF_NEQ(status, mfrc522_drv_status_ok);

    return mfrc522_drv_status_ok;
}

/* Handle getting ATQA */
static inline mfrc522_drv_status
verify_atqa(const mfrc522_drv_conf* conf, const u8* rx_data, u16* atqa)
{
    *atqa = rx_data[0] | (rx_data[1] << 8);

    /* Verify ATQA if enabled */
    if (NULL != conf->atqa_verify_fn) {
        return (conf->atqa_verify_fn(*atqa)) ? mfrc522_drv_status_ok : mfrc522_drv_status_picc_vrf_err;
    }
    return mfrc522_drv_status_ok;
}

/* Helper function to get IRQ number used as the exit criterion during transceive command */
static inline mfrc522_reg_irq
get_awaited_irq_num(mfrc522_reg_cmd cmd)
{
    return (mfrc522_reg_cmd_transceive == cmd) ? mfrc522_reg_irq_rx : mfrc522_reg_irq_idle;
}

/* ------------------------------------------------------------ */
/* ----------------------- Public functions ------------------- */
/* ------------------------------------------------------------ */

mfrc522_drv_status
mfrc522_drv_init(mfrc522_drv_conf* conf)
{
    NOT_NULL(conf, mfrc522_drv_status_nullptr);

    /* In case 'pointer' low-level calls are used check for NULL also */
#if MFRC522_LL_PTR
    NOT_NULL(conf->ll_send, mfrc522_drv_status_nullptr);
    NOT_NULL(conf->ll_recv, mfrc522_drv_status_nullptr);
#if MFRC522_LL_DELAY
    NOT_NULL(conf->ll_delay, mfrc522_drv_status_nullptr);
#endif
#endif

    /* Try to get chip version from a device */
    if (UNLIKELY(mfrc522_drv_status_ok != mfrc522_drv_read(conf, mfrc522_reg_version, &conf->chip_version))) {
        conf->chip_version = MFRC522_REG_VERSION_INVALID;
        return mfrc522_drv_status_ll_err;
    }

    /* Chiptype 9xh stands for MFRC522 */
    if (MFRC522_CONF_CHIP_TYPE != (conf->chip_version & MFRC522_REG_FIELD_MSK_REAL(VERSION_CHIPTYPE))) {
        return mfrc522_drv_status_dev_err;
    }
    return mfrc522_drv_status_ok;
}

mfrc522_drv_status
mfrc522_drv_write_masked(const mfrc522_drv_conf* conf, mfrc522_reg addr, u8 val, u8 mask, u8 pos)
{
    NOT_NULL(conf, mfrc522_drv_status_nullptr);

    mfrc522_drv_status status;
    u8 buff;
    status = mfrc522_drv_read(conf, addr, &buff);
    ERROR_IF_NEQ(status, mfrc522_drv_status_ok);

    buff &= ~(mask << pos);
    buff |= ((val & mask) << pos);

    status = mfrc522_drv_write_byte(conf, addr, buff);
    ERROR_IF_NEQ(status, mfrc522_drv_status_ok);

    return mfrc522_drv_status_ok;
}

mfrc522_drv_status
mfrc522_drv_read_masked(const mfrc522_drv_conf* conf, mfrc522_reg addr, u8* out, u8 mask, u8 pos)
{
    NOT_NULL(conf, mfrc522_drv_status_nullptr);
    NOT_NULL(out, mfrc522_drv_status_nullptr);

    u8 buff;
    mfrc522_drv_status status = mfrc522_drv_read(conf, addr, &buff);
    ERROR_IF_NEQ(status, mfrc522_drv_status_ok);

    buff &= (mask << pos);
    buff >>= pos;
    *out = buff;

    return mfrc522_drv_status_ok;
}

mfrc522_drv_status
mfrc522_drv_read_until(const mfrc522_drv_conf* conf, mfrc522_drv_read_until_conf* ru_conf)
{
    NOT_NULL(conf, mfrc522_drv_status_nullptr);
    NOT_NULL(ru_conf, mfrc522_drv_status_nullptr);

    /* Calculate real retry count */
    u32 rc = get_real_retry_count(ru_conf->retry_cnt);

    /* Handle infinite retry count case */
    u8 rc_decrement_step = (ru_conf->retry_cnt == MFRC522_DRV_RETRY_CNT_INF) ? 0 : 1;

    /* Read register first time */
    mfrc522_drv_status status = mfrc522_drv_read(conf, ru_conf->addr, &ru_conf->payload);
    ERROR_IF_NEQ(status, mfrc522_drv_status_ok);

    while ((ru_conf->exp_payload != (ru_conf->payload & ru_conf->mask))) {
        if (!rc) {
            return mfrc522_drv_status_dev_rtr_err;
        }
        rc -= rc_decrement_step;
        delay(conf, ru_conf->delay); /* Wait for a while and check again */

        status = mfrc522_drv_read(conf, ru_conf->addr, &ru_conf->payload);
        ERROR_IF_NEQ(status, mfrc522_drv_status_ok);
    }
    return mfrc522_drv_status_ok;
}

mfrc522_drv_status
mfrc522_drv_soft_reset(const mfrc522_drv_conf* conf)
{
    NOT_NULL(conf, mfrc522_drv_status_nullptr);

    /* Populate read settings */
    mfrc522_drv_read_until_conf ruc;
    ruc.addr = mfrc522_reg_command;
    ruc.mask = MFRC522_REG_FIELD_MSK_REAL(COMMAND_CMD);
    ruc.exp_payload = mfrc522_reg_cmd_idle;
    ruc.delay = 100; /* Give 100us delay */
    ruc.retry_cnt = MFRC522_DRV_DEF_RETRY_CNT;

    /* Send SoftReset command. Do not care of other bits - they will be set to defaults afterwards */
    mfrc522_drv_status res = mfrc522_drv_write_byte(conf, mfrc522_reg_command, mfrc522_reg_cmd_soft_reset);
    ERROR_IF_NEQ(res, mfrc522_drv_status_ok);

    /* Wait until Idle command is active back */
    res = mfrc522_drv_read_until(conf, &ruc);
    ERROR_IF_NEQ(res, mfrc522_drv_status_ok);

    return mfrc522_drv_status_ok;
}

mfrc522_drv_status
mfrc522_drv_tim_set(mfrc522_drv_tim_conf* tim_conf, u16 period)
{
    NOT_NULL(tim_conf, mfrc522_drv_status_nullptr);
    if (UNLIKELY(0 == period || period > MFRC522_DRV_TIM_MAX_PERIOD)) {
        return mfrc522_drv_status_tim_prd_err;
    }

    /*
     * The formula used to calculate reload value (with TPrescaler == 1694):
     * TReload = 4 * period - 1
     */
    tim_conf->prescaler = TIMER_PRESCALER;
    tim_conf->prescaler_type = mfrc522_drv_tim_psl_even;
    tim_conf->reload_val = (4 * period) - 1;
#if MFRC522_CONF_TIM_RELOAD_FIXED
    ++tim_conf->reload_val;
#endif

    return mfrc522_drv_status_ok;
}

mfrc522_drv_status
mfrc522_drv_tim_start(const mfrc522_drv_conf* conf, const mfrc522_drv_tim_conf* tim_conf)
{
    NOT_NULL(conf, mfrc522_drv_status_nullptr);
    NOT_NULL(tim_conf, mfrc522_drv_status_nullptr);

    /* Write to prescaler Lo and Hi registers */
    u8 prescaler_lo = (u8)(tim_conf->prescaler & 0x00FF);
    u8 prescaler_hi = (u8)((tim_conf->prescaler & 0x0F00) >> 8);
    mfrc522_drv_status status = mfrc522_drv_write_byte(conf, mfrc522_reg_tim_prescaler, prescaler_lo);
    ERROR_IF_NEQ(status, mfrc522_drv_status_ok);
    status = mfrc522_drv_write_masked(conf, mfrc522_reg_tim_mode, prescaler_hi, MFRC522_REG_FIELD(TMODE_TPHI));
    ERROR_IF_NEQ(status, mfrc522_drv_status_ok);

    /* Write to prescaler type register */
    u8 prescaler_type = (tim_conf->prescaler_type == mfrc522_drv_tim_psl_even);
    status = mfrc522_drv_write_masked(conf, mfrc522_reg_demod, prescaler_type, MFRC522_REG_FIELD(DEMOD_TPE));
    ERROR_IF_NEQ(status, mfrc522_drv_status_ok);

    /* Write to reload Lo and Hi registers */
    u8 reload_lo = (u8)(tim_conf->reload_val & 0x00FF);
    u8 reload_hi = (u8)((tim_conf->reload_val & 0xFF00) >> 8);
    status = mfrc522_drv_write_byte(conf, mfrc522_reg_tim_reload_lo, reload_lo);
    ERROR_IF_NEQ(status, mfrc522_drv_status_ok);
    status = mfrc522_drv_write_byte(conf, mfrc522_reg_tim_reload_hi, reload_hi);
    ERROR_IF_NEQ(status, mfrc522_drv_status_ok);

    /* Enable/disable periodicity flag */
    status = mfrc522_drv_write_masked(conf, mfrc522_reg_tim_mode,
                                      tim_conf->periodic, MFRC522_REG_FIELD(TMODE_TAUTO_RESTART));
    ERROR_IF_NEQ(status, mfrc522_drv_status_ok);

    /* Immediately start timer */
    status = mfrc522_drv_write_masked(conf, mfrc522_reg_control, 1, MFRC522_REG_FIELD(CONTROL_TSTART));
    ERROR_IF_NEQ(status, mfrc522_drv_status_ok);

    return mfrc522_drv_status_ok;
}

mfrc522_drv_status
mfrc522_drv_tim_stop(const mfrc522_drv_conf* conf)
{
    NOT_NULL(conf, mfrc522_drv_status_nullptr);

    mfrc522_drv_status status;
    status = mfrc522_drv_write_masked(conf, mfrc522_reg_control, 1, MFRC522_REG_FIELD(CONTROL_TSTOP));
    ERROR_IF_NEQ(status, mfrc522_drv_status_ok);

    return mfrc522_drv_status_ok;
}

mfrc522_drv_status
mfrc522_drv_irq_init(const mfrc522_drv_conf* conf, const mfrc522_drv_irq_conf* irq_conf)
{
    NOT_NULL(conf, mfrc522_drv_status_nullptr);
    NOT_NULL(irq_conf, mfrc522_drv_status_nullptr);

    /* Clear all interrupt flags */
    mfrc522_drv_irq_clr(conf, mfrc522_reg_irq_all);

    /* Update IRQ flags */
    mfrc522_drv_status status = mfrc522_drv_write_masked(conf, mfrc522_reg_com_irq_en,
                                                         irq_conf->irq_signal_inv, MFRC522_REG_FIELD(COMIEN_IRQ_INV));
    ERROR_IF_NEQ(status, mfrc522_drv_status_ok);
    status = mfrc522_drv_write_masked(conf, mfrc522_reg_div_irq_en,
                                      irq_conf->irq_push_pull, MFRC522_REG_FIELD(DIVIEN_IRQ_PUSHPULL));
    ERROR_IF_NEQ(status, mfrc522_drv_status_ok);

    return mfrc522_drv_status_ok;
}

mfrc522_drv_status
mfrc522_drv_irq_clr(const mfrc522_drv_conf* conf, mfrc522_reg_irq irq)
{
    NOT_NULL(conf, mfrc522_drv_status_nullptr);

    mfrc522_drv_status status;

    /* Clear all interrupts if special flag was passed */
    if (mfrc522_reg_irq_all == irq) {
        status = mfrc522_drv_write_byte(conf, mfrc522_reg_com_irq, IRQ_ALL_COM_MASK);
        ERROR_IF_NEQ(status, mfrc522_drv_status_ok);
        status = mfrc522_drv_write_byte(conf, mfrc522_reg_div_irq, IRQ_ALL_DIV_MASK);
        ERROR_IF_NEQ(status, mfrc522_drv_status_ok);
    } else {
        /* Find correct register to clear interrupt flag in */
        u8 reg;
        if (irq & MFRC522_REG_IRQ_DIV) {
            irq &= ~MFRC522_REG_IRQ_DIV;
            reg = mfrc522_reg_div_irq;
        } else {
            reg = mfrc522_reg_com_irq;
        }
        status = mfrc522_drv_write_byte(conf, reg, 1 << irq);
        ERROR_IF_NEQ(status, mfrc522_drv_status_ok);
    }

    return mfrc522_drv_status_ok;
}

mfrc522_drv_status
mfrc522_drv_irq_en(const mfrc522_drv_conf* conf, mfrc522_reg_irq irq, bool enable)
{
    NOT_NULL(conf, mfrc522_drv_status_nullptr);
    ERROR_IF_EQ(irq, mfrc522_reg_irq_all, mfrc522_drv_status_nok);

    /* Find correct register to enable interrupt */
    u8 reg;
    if (irq & MFRC522_REG_IRQ_DIV) {
        irq &= ~MFRC522_REG_IRQ_DIV;
        reg = mfrc522_reg_div_irq_en;
    } else {
        reg = mfrc522_reg_com_irq_en;
    }
    mfrc522_drv_status status = mfrc522_drv_write_masked(conf, reg, enable, 0x01, irq);
    ERROR_IF_NEQ(status, mfrc522_drv_status_ok);

    return mfrc522_drv_status_ok;
}

mfrc522_drv_status
mfrc522_drv_irq_states(const mfrc522_drv_conf* conf, u16* out)
{
    NOT_NULL(conf, mfrc522_drv_status_nullptr);
    NOT_NULL(out, mfrc522_drv_status_nullptr);

    u8 com_irq;
    u8 div_irq;
    mfrc522_drv_status status = mfrc522_drv_read(conf, mfrc522_reg_com_irq, &com_irq);
    ERROR_IF_NEQ(status, mfrc522_drv_status_ok);
    status = mfrc522_drv_read(conf, mfrc522_reg_div_irq, &div_irq);
    ERROR_IF_NEQ(status, mfrc522_drv_status_ok);

    *out = com_irq | (div_irq << 8);

    return mfrc522_drv_status_ok;
}

bool
mfrc522_drv_irq_pending(u16 irq_states, mfrc522_reg_irq irq)
{
    /* If 'mfrc522_reg_irq_all' was passed return an error and exit */
    ERROR_IF_EQ(irq, mfrc522_reg_irq_all, false);

    if (irq & MFRC522_REG_IRQ_DIV) {
        irq &= ~MFRC522_REG_IRQ_DIV;
        irq += 8;
    }
    return (irq_states & (1 << irq)) ? true : false;
}

mfrc522_drv_status
mfrc522_drv_self_test(mfrc522_drv_conf* conf)
{
    NOT_NULL(conf, mfrc522_drv_status_nullptr);

    mfrc522_drv_status status;

    /* Step 1 - Perform a soft reset */
    status = mfrc522_drv_soft_reset(conf);
    ERROR_IF_NEQ(status, mfrc522_drv_status_ok);

    /* Step 2 - Clear the internal buffer by writing 25 bytes of 00h */
    status = mfrc522_drv_fifo_store(conf, 0x00);
    ERROR_IF_NEQ(status, mfrc522_drv_status_ok);
    status = mfrc522_drv_invoke_cmd(conf, mfrc522_reg_cmd_mem);
    ERROR_IF_NEQ(status, mfrc522_drv_status_ok);

    /* Step 3 - Enable the self test by writing 09h to the AutoTestReg register */
    status = mfrc522_drv_write_masked(conf, mfrc522_reg_auto_test, 0x09, MFRC522_REG_FIELD(AUTOTEST_SELFTEST));
    ERROR_IF_NEQ(status, mfrc522_drv_status_ok);

    /* Step 4 - Write 00h to the FIFO buffer */
    status = mfrc522_drv_fifo_store(conf, 0x00);
    ERROR_IF_NEQ(status, mfrc522_drv_status_ok);

    /* Step 5 - Start the self test with the CalcCRC command */
    status = mfrc522_drv_invoke_cmd(conf, mfrc522_reg_cmd_crc);
    ERROR_IF_NEQ(status, mfrc522_drv_status_ok);

    /* Step 6 - Wait until FIFO buffer contains 64 bytes */
    mfrc522_drv_read_until_conf rc;
    rc.addr = mfrc522_reg_fifo_level;
    rc.mask = 0xFF;
    rc.exp_payload = 0x40; /* 64 bytes */
    rc.delay = 100;
    rc.retry_cnt = MFRC522_DRV_DEF_RETRY_CNT;

    status = mfrc522_drv_read_until(conf, &rc);
    ERROR_IF_NEQ(status, mfrc522_drv_status_ok);

    /* Step 7 - Read content from the FIFO buffer and store it inside device's configuration structure */
    u8 buff;
    for (size i = 0; i < 64; ++i) {
        status = mfrc522_drv_fifo_read(conf, &buff);
        ERROR_IF_NEQ(status, mfrc522_drv_status_ok);
        conf->self_test_out[i] = buff;
    }

    /* Step 8 - Compare output bytes with expected ones */
    u8 expected[MFRC522_DRV_SELF_TEST_FIFO_SZ] = {MFRC522_CONF_SELF_TEST_FIFO_OUT};
    if (UNLIKELY(memcmp(expected, conf->self_test_out, MFRC522_DRV_SELF_TEST_FIFO_SZ))) {
        return mfrc522_drv_status_self_test_err;
    }

    return mfrc522_drv_status_ok;
}

mfrc522_drv_status
mfrc522_drv_invoke_cmd(const mfrc522_drv_conf* conf, mfrc522_reg_cmd cmd)
{
    NOT_NULL(conf, mfrc522_drv_status_nullptr);

    /* Write to command register */
    mfrc522_drv_status status;
    status = mfrc522_drv_write_masked(conf, mfrc522_reg_command, cmd, MFRC522_REG_FIELD(COMMAND_CMD));
    ERROR_IF_NEQ(status, mfrc522_drv_status_ok);

    mfrc522_drv_read_until_conf ru_conf;
    ru_conf.addr = mfrc522_reg_command;
    ru_conf.mask = MFRC522_REG_FIELD_MSK_REAL(COMMAND_CMD);
    ru_conf.exp_payload = mfrc522_reg_cmd_idle;
    ru_conf.delay = 5; /* Give some delay */
    ru_conf.retry_cnt = MFRC522_DRV_DEF_RETRY_CNT;

    switch (cmd) {
        /* These commands terminate automatically. Wait until Idle command is active back */
        case mfrc522_reg_cmd_idle:
        case mfrc522_reg_cmd_mem:
        case mfrc522_reg_cmd_rand:
        case mfrc522_reg_cmd_soft_reset:
            status = mfrc522_drv_read_until(conf, &ru_conf);
            ERROR_IF_NEQ(status, mfrc522_drv_status_ok);
        default:
            break;
    }

    return mfrc522_drv_status_ok;
}

mfrc522_drv_status
mfrc522_drv_crc_init(const mfrc522_drv_conf* conf, const mfrc522_drv_crc_conf* crc_conf)
{
    NOT_NULL(conf, mfrc522_drv_status_nullptr);
    NOT_NULL(crc_conf, mfrc522_drv_status_nullptr);

    /* Write to the registers associated with CRC coprocessor */
    mfrc522_drv_status status;
    status = mfrc522_drv_write_masked(conf, mfrc522_reg_mode, crc_conf->preset, MFRC522_REG_FIELD(MODE_CRC_PRESET));
    ERROR_IF_NEQ(status, mfrc522_drv_status_ok);
    status = mfrc522_drv_write_masked(conf, mfrc522_reg_mode,
                                      crc_conf->msb_first, MFRC522_REG_FIELD(MODE_CRC_MSBFIRST));
    ERROR_IF_NEQ(status, mfrc522_drv_status_ok);

    return mfrc522_drv_status_ok;
}

mfrc522_drv_status
mfrc522_drv_crc_compute(const mfrc522_drv_conf* conf, u16* out)
{
    NOT_NULL(conf, mfrc522_drv_status_nullptr);
    NOT_NULL(out, mfrc522_drv_status_nullptr);

    /* Start computing CRC */
    mfrc522_drv_status status;
    status = mfrc522_drv_invoke_cmd(conf, mfrc522_reg_cmd_crc);
    ERROR_IF_NEQ(status, mfrc522_drv_status_ok);

    /* Wait until ready bit is set */
    mfrc522_drv_read_until_conf ru_conf;
    ru_conf.addr = mfrc522_reg_status1;
    ru_conf.exp_payload = 1 << MFRC522_REG_FIELD_POS(STATUS1_CRC_READY);
    ru_conf.mask = MFRC522_REG_FIELD_MSK_REAL(STATUS1_CRC_READY);
    ru_conf.retry_cnt = MFRC522_DRV_DEF_RETRY_CNT;
    ru_conf.delay = 1;
    status = mfrc522_drv_read_until(conf, &ru_conf);
    ERROR_IF_NEQ(status, mfrc522_drv_status_ok);

    /* CRC command does not terminate itself. Thus activate Idle command back */
    status = mfrc522_drv_invoke_cmd(conf, mfrc522_reg_cmd_idle);
    ERROR_IF_NEQ(status, mfrc522_drv_status_ok);

    /* Read computed value and write into the buffer */
    u8 crc_lo;
    u8 crc_hi;
    status = mfrc522_drv_read(conf, mfrc522_reg_crc_result_lsb, &crc_lo);
    ERROR_IF_NEQ(status, mfrc522_drv_status_ok);
    status = mfrc522_drv_read(conf, mfrc522_reg_crc_result_msb, &crc_hi);
    ERROR_IF_NEQ(status, mfrc522_drv_status_ok);
    *out = crc_lo | (crc_hi << 8);

    return mfrc522_drv_status_ok;
}

mfrc522_drv_status
mfrc522_drv_generate_rand(const mfrc522_drv_conf* conf, u8* out, size num_rand)
{
    NOT_NULL(conf, mfrc522_drv_status_nullptr);
    NOT_NULL(out, mfrc522_drv_status_nullptr);
    if (num_rand > MFRC522_DRV_RAND_BYTES) {
        num_rand = MFRC522_DRV_RAND_BYTES;
    }

    mfrc522_drv_status status;
    /* Invoke 'Random' command */
    status = mfrc522_drv_invoke_cmd(conf, mfrc522_reg_cmd_rand);
    ERROR_IF_NEQ(status, mfrc522_drv_status_ok);
    /* Copy bytes from internal buffer into the FIFO buffer */
    status = mfrc522_drv_invoke_cmd(conf, mfrc522_reg_cmd_mem);
    ERROR_IF_NEQ(status, mfrc522_drv_status_ok);

    /* Copy data from the FIFO buffer */
    size rand_bytes_idx[MFRC522_DRV_RAND_BYTES] = {MFRC522_CONF_RAND_BYTE_IDX};
    u8 rand_bytes[MFRC522_DRV_RAND_BYTES];
    u8 buff;
    size current_idx = 0;
    for (size i = 0; i < MFRC522_DRV_RAND_TOTAL; ++i) {
        status = mfrc522_drv_fifo_read(conf, &buff);
        ERROR_IF_NEQ(status, mfrc522_drv_status_ok);
        for (size j = 0; j < MFRC522_DRV_RAND_BYTES; ++j) {
            if (i == rand_bytes_idx[j]) {
                rand_bytes[current_idx++] = buff;
            }
        }
    }

    /* Copy requested number of random bytes into the final buffer */
    memcpy(out, rand_bytes, num_rand);

    return mfrc522_drv_status_ok;
}

bool
mfrc522_drv_check_error(u8 error_reg, mfrc522_reg_err err)
{
    if (mfrc522_reg_err_any == err) {
        return (0 != error_reg);
    }
    return (error_reg & (1 << err)) ? true : false;
}

mfrc522_drv_status
mfrc522_drv_ext_itf_init(const mfrc522_drv_conf* conf, const mfrc522_drv_ext_itf_conf* itf_conf)
{
    NOT_NULL(conf, mfrc522_drv_status_nullptr);
    NOT_NULL(itf_conf, mfrc522_drv_status_nullptr);

    /* Force a 100% ASK modulation */
    mfrc522_drv_status status;
    status = mfrc522_drv_write_masked(conf, mfrc522_reg_tx_ask, 1, MFRC522_REG_FIELD(TXASK_FORCE_ASK));
    ERROR_IF_NEQ(status, mfrc522_drv_status_ok);

    /* Configure TX RF */
    status = mfrc522_drv_write_masked(conf, mfrc522_reg_tx_control, 1, MFRC522_REG_FIELD(TXCONTROL_TX1RFEN));
    ERROR_IF_NEQ(status, mfrc522_drv_status_ok);
    status = mfrc522_drv_write_masked(conf, mfrc522_reg_tx_control, 1, MFRC522_REG_FIELD(TXCONTROL_TX2RFEN));
    ERROR_IF_NEQ(status, mfrc522_drv_status_ok);

    /* Switch on analog part of the receiver */
    status = mfrc522_drv_write_masked(conf, mfrc522_reg_command, 0, MFRC522_REG_FIELD(COMMAND_RCVOFF));
    ERROR_IF_NEQ(status, mfrc522_drv_status_ok);

    return mfrc522_drv_status_ok;
}

mfrc522_drv_status
mfrc522_drv_transceive(const mfrc522_drv_conf* conf, mfrc522_drv_transceive_conf* tr_conf)
{
    NOT_NULL(conf, mfrc522_drv_status_nullptr);
    NOT_NULL(tr_conf, mfrc522_drv_status_nullptr);
    bool cmd_error = (mfrc522_reg_cmd_transceive != tr_conf->command) &&
                     (mfrc522_reg_cmd_authent != tr_conf->command);
    if (UNLIKELY(cmd_error)) {
        return mfrc522_drv_status_nok;
    }

    /* Clear all IRQs */
    mfrc522_drv_status status = mfrc522_drv_irq_clr(conf, mfrc522_reg_irq_all);
    ERROR_IF_NEQ(status, mfrc522_drv_status_ok);

    /* Flush the FIFO buffer and store actual data */
    status = mfrc522_drv_fifo_flush(conf);
    ERROR_IF_NEQ(status, mfrc522_drv_status_ok);
    status = mfrc522_drv_fifo_store_mul(conf, tr_conf->tx_data, tr_conf->tx_data_sz);
    ERROR_IF_NEQ(status, mfrc522_drv_status_ok);

    /* Invoke transceive command */
    status = mfrc522_drv_invoke_cmd(conf, tr_conf->command);
    ERROR_IF_NEQ(status, mfrc522_drv_status_ok);

    /* Start transmission of the data */
    status = mfrc522_drv_write_masked(conf, mfrc522_reg_bit_framing, 1, MFRC522_REG_FIELD(BITFRAMING_START));
    ERROR_IF_NEQ(status, mfrc522_drv_status_ok);

    u16 irq_states;
    bool exit_irq;
    bool error;
    size retry_total_num = get_real_retry_count(MFRC522_DRV_DEF_RETRY_CNT);
    size retries;
    for (retries = 0; retries < retry_total_num; ++retries) {
        status = mfrc522_drv_irq_states(conf, &irq_states);
        ERROR_IF_NEQ(status, mfrc522_drv_status_ok);

        exit_irq = mfrc522_drv_irq_pending(irq_states, get_awaited_irq_num(tr_conf->command));
        error = mfrc522_drv_irq_pending(irq_states, mfrc522_reg_irq_err);
        if (exit_irq || error) {
            break;
        }
        delay(conf, 1); /* Make some delay */
    }

    /* End transmission of the data */
    status = mfrc522_drv_write_masked(conf, mfrc522_reg_bit_framing, 0, MFRC522_REG_FIELD(BITFRAMING_START));
    ERROR_IF_NEQ(status, mfrc522_drv_status_ok);

    /* Enter Idle state */
    status = mfrc522_drv_invoke_cmd(conf, mfrc522_reg_cmd_idle);
    ERROR_IF_NEQ(status, mfrc522_drv_status_ok);

    /* In case when response is missing */
    if (UNLIKELY(retry_total_num == retries)) {
        return mfrc522_drv_status_transceive_timeout;
    }

    /* In case when at least one error bit is present */
    if (UNLIKELY(error)) {
        return mfrc522_drv_status_transceive_err;
    }

    /* Get RX data if desired */
    if (0 != tr_conf->rx_data_sz) {
        /* Get RX data size */
        u8 valid_rx_bytes;
        status = get_valid_rx_bytes(conf, &valid_rx_bytes);
        ERROR_IF_NEQ(status, mfrc522_drv_status_ok);

        /* RX data size error */
        if (UNLIKELY(valid_rx_bytes != tr_conf->rx_data_sz)) {
            return mfrc522_drv_status_transceive_rx_mism;
        }

        /* Get FIFO contents */
        for (size i = 0; i < tr_conf->rx_data_sz; ++i) {
            status = mfrc522_drv_fifo_read(conf, &tr_conf->rx_data[i]);
            ERROR_IF_NEQ(status, mfrc522_drv_status_ok);
        }
    }

    return mfrc522_drv_status_ok;
}

mfrc522_drv_status
mfrc522_drv_reqa(const mfrc522_drv_conf* conf, u16* atqa)
{
    NOT_NULL(conf, mfrc522_drv_status_nullptr);
    NOT_NULL(atqa, mfrc522_drv_status_nullptr);

    /* REQA is a bit oriented frame (7-bit), thus set proper register */
    mfrc522_drv_status status;
    status = mfrc522_drv_write_masked(conf, mfrc522_reg_bit_framing, 0x07, MFRC522_REG_FIELD(BITFRAMING_TX_LASTBITS));
    ERROR_IF_NEQ(status, mfrc522_drv_status_ok);

    /* Handle transmission/reception of the data */
    u8 reqa = mfrc522_picc_cmd_reqa;
    u8 response[2];

    mfrc522_drv_transceive_conf tr_conf;
    tr_conf.tx_data = &reqa;
    tr_conf.tx_data_sz = 1;
    tr_conf.rx_data = &response[0];
    tr_conf.rx_data_sz = 2;
    tr_conf.command = mfrc522_reg_cmd_transceive;
    status = mfrc522_drv_transceive(conf, &tr_conf);

    switch (status) {
        case mfrc522_drv_status_transceive_timeout:
        case mfrc522_drv_status_transceive_err:
        case mfrc522_drv_status_transceive_rx_mism:
            *atqa = MFRC522_PICC_ATQA_INV;
            return status;
        case mfrc522_drv_status_ok:
            /* Nothing to do here. Just exit the switch statement */
            break;
        default: /* Low-level error, etc. */
            return status;
    }

    /* Restore TX last bits config */
    status = mfrc522_drv_write_masked(conf, mfrc522_reg_bit_framing, 0x00, MFRC522_REG_FIELD(BITFRAMING_TX_LASTBITS));
    ERROR_IF_NEQ(status, mfrc522_drv_status_ok);

    /* Verify ATQA */
    status = verify_atqa(conf, &response[0], atqa);
    ERROR_IF_NEQ(status, mfrc522_drv_status_ok);

    return mfrc522_drv_status_ok;
}

mfrc522_drv_status
mfrc522_drv_anticollision(const mfrc522_drv_conf* conf, u8* serial)
{
    NOT_NULL(conf, mfrc522_drv_status_nullptr);
    NOT_NULL(serial, mfrc522_drv_status_nullptr);

    mfrc522_drv_status status;

    /* TX data consist of two bytes */
    u8 tx[2];
    tx[0] = mfrc522_picc_cmd_anticoll_cl1 & 0xFF;
    tx[1] = (mfrc522_picc_cmd_anticoll_cl1 & 0xFF00) >> 8;

    /* Transceive the data */
    mfrc522_drv_transceive_conf tr_conf;
    tr_conf.tx_data = &tx[0];
    tr_conf.tx_data_sz = SIZE_ARRAY(tx);
    tr_conf.rx_data = serial;
    tr_conf.rx_data_sz = 5;
    tr_conf.command = mfrc522_reg_cmd_transceive;
    status = mfrc522_drv_transceive(conf, &tr_conf);

    /* Compute checksum */
    if (mfrc522_drv_status_ok == status) {
        u8 checksum = 0;
        for (size i = 0; i < 4; ++i) {
            checksum ^= serial[i];
        }
        if (UNLIKELY(serial[4] != checksum)) {
            return mfrc522_drv_status_anticoll_chksum_err;
        }
    }

    return status;
}

mfrc522_drv_status
mfrc522_drv_select(const mfrc522_drv_conf* conf, const u8* serial, u8* sak)
{
    NOT_NULL(conf, mfrc522_drv_status_nullptr);
    NOT_NULL(serial, mfrc522_drv_status_nullptr);
    NOT_NULL(sak, mfrc522_drv_status_nullptr);

    /* Build TX data */
    u16 crc;
    u8 tx[9];
    tx[0] = mfrc522_picc_cmd_select_cl1 & 0xFF;
    tx[1] = (mfrc522_picc_cmd_select_cl1 & 0xFF00) >> 8;
    memcpy(&tx[2], serial, 5);
    /* Compute and append CRC */
    mfrc522_drv_status status = mfrc522_drv_fifo_store_mul(conf, &tx[0], 7);
    ERROR_IF_NEQ(status, mfrc522_drv_status_ok);
    status = mfrc522_drv_crc_compute(conf, &crc);
    ERROR_IF_NEQ(status, mfrc522_drv_status_ok);
    tx[7] = crc & 0xFF;
    tx[8] = (crc & 0xFF00) >> 8;

    /* Transceive the data */
    u8 rx[3];
    mfrc522_drv_transceive_conf tr_conf;
    tr_conf.tx_data = &tx[0];
    tr_conf.tx_data_sz = SIZE_ARRAY(tx);
    tr_conf.rx_data = &rx[0];
    tr_conf.rx_data_sz = SIZE_ARRAY(rx);
    tr_conf.command = mfrc522_reg_cmd_transceive;
    status = mfrc522_drv_transceive(conf, &tr_conf);
    ERROR_IF_NEQ(status, mfrc522_drv_status_ok);

    /* Calculate CRC of SAK response */
    status = mfrc522_drv_fifo_store(conf, rx[0]);
    ERROR_IF_NEQ(status, mfrc522_drv_status_ok);
    status = mfrc522_drv_crc_compute(conf, &crc);
    ERROR_IF_NEQ(status, mfrc522_drv_status_ok);

    u16 crc_from_picc = rx[1] | (rx[2] << 8);
    if (UNLIKELY(crc_from_picc != crc)) {
        return mfrc522_drv_status_crc_err;
    }

    *sak = rx[0];
    return mfrc522_drv_status_ok;
}

mfrc522_drv_status
mfrc522_drv_authenticate(const mfrc522_drv_conf* conf, const mfrc522_drv_auth_conf* auth_conf)
{
    NOT_NULL(conf, mfrc522_drv_status_nullptr);
    NOT_NULL(auth_conf, mfrc522_drv_status_nullptr);

    /* Build TX data */
    u8 tx[12];
    tx[0] = auth_conf->key_type;
    tx[1] = mfrc522_picc_block_descriptor(auth_conf->sector, auth_conf->block);
    memcpy(&tx[2], auth_conf->key, 6);
    memcpy(&tx[8], auth_conf->serial, 4);

    /* Transceive the data */
    mfrc522_drv_transceive_conf tr_conf;
    tr_conf.tx_data = &tx[0];
    tr_conf.tx_data_sz = SIZE_ARRAY(tx);
    tr_conf.rx_data = NULL;
    tr_conf.rx_data_sz = 0; /* No data is expected on RX side */
    tr_conf.command = mfrc522_reg_cmd_authent;
    mfrc522_drv_status status = mfrc522_drv_transceive(conf, &tr_conf);
    ERROR_IF_NEQ(status, mfrc522_drv_status_ok);

    /* Check if crypto is enabled */
    u8 crypto;
    status = mfrc522_drv_read_masked(conf, mfrc522_reg_status2, &crypto, MFRC522_REG_FIELD(STATUS2_CRYPTO_ON));
    ERROR_IF_NEQ(status, mfrc522_drv_status_ok);

    return (!crypto) ? mfrc522_drv_status_crypto_err : mfrc522_drv_status_ok;
}

mfrc522_drv_status
mfrc522_drv_halt(const mfrc522_drv_conf* conf)
{
    NOT_NULL(conf, mfrc522_drv_status_nullptr);

    /* TX data */
    u8 tx[4];
    tx[0] = mfrc522_picc_cmd_halt & 0xFF;
    tx[1] = (mfrc522_picc_cmd_halt & 0xFF00) >> 8;
    mfrc522_drv_status status = mfrc522_drv_fifo_store_mul(conf, &tx[0], 2);
    ERROR_IF_NEQ(status, mfrc522_drv_status_ok);
    u16 crc;
    status = mfrc522_drv_crc_compute(conf, &crc);
    ERROR_IF_NEQ(status, mfrc522_drv_status_ok);
    tx[2] = crc & 0xFF;
    tx[3] = (crc & 0xFF00) >> 8;

    /* Transceive the data */
    mfrc522_drv_transceive_conf tr_conf;
    tr_conf.tx_data = &tx[0];
    tr_conf.tx_data_sz = SIZE_ARRAY(tx);
    tr_conf.rx_data = NULL;
    tr_conf.rx_data_sz = 0;
    tr_conf.command = mfrc522_reg_cmd_transceive;
    status = mfrc522_drv_transceive(conf, &tr_conf);
    /* This is intentional! Halt command succeeded when timeout occurs during reception of the data */
    if (UNLIKELY(mfrc522_drv_status_ok == status)) {
        return mfrc522_drv_status_halt_err;
    } else if (mfrc522_drv_status_transceive_timeout != status) {
        return status; /* Just forward the error */
    }

    /* Turn off the crypto unit */
    status = mfrc522_drv_write_masked(conf, mfrc522_reg_status2, 0, MFRC522_REG_FIELD(STATUS2_CRYPTO_ON));
    ERROR_IF_NEQ(status, mfrc522_drv_status_ok);

    return mfrc522_drv_status_ok;
}

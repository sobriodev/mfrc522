#include "mfrc522_drv.h"
#include "mfrc522_conf.h"

/* ------------------------------------------------------------ */
/* ------------------------ Private macros -------------------- */
/* ------------------------------------------------------------ */

/* Try to send single-byte to device with returning a generic error code if necessary */
#define TRY_WRITE_BYTE(...)  \
do { \
    if (UNLIKELY(mfrc522_ll_status_ok != mfrc522_drv_write_byte(__VA_ARGS__))) return mfrc522_drv_status_ll_err; \
} while (0)

/* Try to send multiple bytes to device with returning a generic error code if necessary */
#define TRY_WRITE_MULTI(...)  \
do { \
    if (UNLIKELY(mfrc522_ll_status_ok != mfrc522_drv_write(__VA_ARGS__))) return mfrc522_drv_status_ll_err; \
} while (0)

/* Try to perform masked write with returning a generic error code if necessary */
#define TRY_WRITE_MASKED(...)  \
do { \
    if (UNLIKELY(mfrc522_ll_status_ok != mfrc522_drv_write_masked(__VA_ARGS__))) return mfrc522_drv_status_ll_err; \
} while (0)

/* Try to receive bytes from device with returning a generic error code if necessary */
#define TRY_READ(...) \
do { \
    if (UNLIKELY(mfrc522_ll_status_ok != mfrc522_drv_read(__VA_ARGS__))) return mfrc522_drv_status_ll_err; \
} while (0)

/* Timer prescaler when using millisecond as a time unit */
#define TIMER_PRESCALER 1694

/* Masks to handle all possible interrupts */
#define IRQ_ALL_COM_MASK 0x7F
#define IRQ_ALL_DIV_MASK 0x14

/* ------------------------------------------------------------ */
/* ----------------------- Private functions ------------------ */
/* ------------------------------------------------------------ */

/* Private delay implementation */
static inline void delay(const mfrc522_drv_conf* conf, u32 period)
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
static inline u32 get_real_retry_count(u32 rc)
{
#if !MFRC522_LL_DELAY
    rc *= MFRC522_CONF_RETRY_CNT_MUL;
#endif
    return rc;
}

/* ------------------------------------------------------------ */
/* ----------------------- Public functions ------------------- */
/* ------------------------------------------------------------ */

mfrc522_drv_status mfrc522_drv_init(mfrc522_drv_conf* conf)
{
    ERROR_IF_EQ(conf, NULL, mfrc522_drv_status_nullptr);

    /* In case 'pointer' low-level calls are used check for NULL also */
#if MFRC522_LL_PTR
    ERROR_IF_EQ(conf->ll_send, NULL, mfrc522_drv_status_nullptr);
    ERROR_IF_EQ(conf->ll_recv, NULL, mfrc522_drv_status_nullptr);
#if MFRC522_LL_DELAY
    ERROR_IF_EQ(conf->ll_delay, NULL, mfrc522_drv_status_nullptr);
#endif
#endif

    /* Try to get chip version from a device */
    if (UNLIKELY(mfrc522_ll_status_ok != mfrc522_drv_read(conf, mfrc522_reg_version, &conf->chip_version))) {
        conf->chip_version = MFRC522_REG_VERSION_INVALID;
        return mfrc522_drv_status_ll_err;
    }

    /* Chiptype 9xh stands for MFRC522 */
    if (MFRC522_CONF_CHIP_TYPE != (conf->chip_version & MFRC522_REG_MSK_REAL(VERSION_CHIPTYPE))) {
        return mfrc522_drv_status_dev_err;
    }
    return mfrc522_drv_status_ok;
}

mfrc522_ll_status mfrc522_drv_write_masked(const mfrc522_drv_conf* conf, mfrc522_reg addr, u8 val, u8 mask, u8 pos)
{
    ERROR_IF_EQ(conf, NULL, mfrc522_ll_status_send_err);

    u8 buff;
    ERROR_IF_NEQ(mfrc522_drv_read(conf, addr, &buff), mfrc522_ll_status_ok);
    buff &= ~(mask << pos);
    buff |= ((val & mask) << pos);
    ERROR_IF_NEQ(mfrc522_drv_write_byte(conf, addr, buff), mfrc522_ll_status_ok);
    return mfrc522_ll_status_ok;
}

mfrc522_drv_status mfrc522_drv_read_until(const mfrc522_drv_conf* conf, mfrc522_drv_read_until_conf* ru_conf)
{
    ERROR_IF_EQ(conf, NULL, mfrc522_drv_status_nullptr);
    ERROR_IF_EQ(ru_conf, NULL, mfrc522_drv_status_nullptr);

    /* Calculate real retry count */
    u32 rc = get_real_retry_count(ru_conf->retry_cnt);

    /* Handle infinite retry count case */
    u8 rc_decrement_step = (ru_conf->retry_cnt == MFRC522_DRV_RETRY_CNT_INF) ? 0 : 1;

    /* Read register first time */
    TRY_READ(conf, ru_conf->addr, &ru_conf->payload);

    while ((ru_conf->exp_payload != (ru_conf->payload & ru_conf->mask))) {
        if (!rc) {
            return mfrc522_drv_status_dev_rtr_err;
        }
        rc -= rc_decrement_step;
        delay(conf, ru_conf->delay); /* Wait for a while and check again */
        TRY_READ(conf, ru_conf->addr, &ru_conf->payload);
    }
    return mfrc522_drv_status_ok;
}

mfrc522_drv_status mfrc522_drv_soft_reset(const mfrc522_drv_conf* conf)
{
    ERROR_IF_EQ(conf, NULL, mfrc522_drv_status_nullptr);

    /* Populate read settings */
    mfrc522_drv_read_until_conf ruc;
    ruc.addr = mfrc522_reg_command;
    ruc.mask = MFRC522_REG_MSK_REAL(COMMAND_CMD);
    ruc.exp_payload = mfrc522_reg_cmd_idle;
    ruc.delay = 100; /* Give 100us delay */
    ruc.retry_cnt = MFRC522_DRV_DEF_RETRY_CNT;

    /* Wait for current command to finish */
    mfrc522_drv_status res = mfrc522_drv_read_until(conf, &ruc);
    ERROR_IF_NEQ(res, mfrc522_drv_status_ok);

    /* Send SoftReset command. Do not care of other bits - they will be set to defaults afterwards */
    TRY_WRITE_BYTE(conf, mfrc522_reg_command, mfrc522_reg_cmd_soft_reset);

    /* Wait until Idle command is active back */
    res = mfrc522_drv_read_until(conf, &ruc);
    ERROR_IF_NEQ(res, mfrc522_drv_status_ok);

    return mfrc522_drv_status_ok;
}

mfrc522_drv_status mfrc522_drv_tim_set(mfrc522_drv_tim_conf* tim_conf, u16 period)
{
    ERROR_IF_EQ(tim_conf, NULL, mfrc522_drv_status_nullptr);
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

mfrc522_drv_status mfrc522_drv_tim_start(const mfrc522_drv_conf* conf, const mfrc522_drv_tim_conf* tim_conf)
{
    ERROR_IF_EQ(conf, NULL, mfrc522_drv_status_nullptr);
    ERROR_IF_EQ(tim_conf, NULL, mfrc522_drv_status_nullptr);

    /* Write to prescaler Lo and Hi registers */
    u8 prescaler_lo = (u8)(tim_conf->prescaler & 0x00FF);
    u8 prescaler_hi = (u8)((tim_conf->prescaler & 0x0F00) >> 8);
    TRY_WRITE_BYTE(conf, mfrc522_reg_tim_prescaler, prescaler_lo);
    TRY_WRITE_MASKED(conf, mfrc522_reg_tim_mode, prescaler_hi, MFRC522_REG_FIELD(TMODE_TPHI));

    /* Write to prescaler type register */
    u8 prescaler_type = (tim_conf->prescaler_type == mfrc522_drv_tim_psl_even);
    TRY_WRITE_MASKED(conf, mfrc522_reg_demod, prescaler_type, MFRC522_REG_FIELD(DEMOD_TPE));

    /* Write to reload Lo and Hi registers */
    u8 reload_lo = (u8)(tim_conf->reload_val & 0x00FF);
    u8 reload_hi = (u8)((tim_conf->reload_val & 0xFF00) >> 8);
    TRY_WRITE_BYTE(conf, mfrc522_reg_tim_reload_lo, reload_lo);
    TRY_WRITE_BYTE(conf, mfrc522_reg_tim_reload_hi, reload_hi);

    /* Enable/disable periodicity flag */
    TRY_WRITE_MASKED(conf, mfrc522_reg_tim_mode, tim_conf->periodic, MFRC522_REG_FIELD(TMODE_TAUTO_RESTART));

    /* Immediately start timer */
    TRY_WRITE_MASKED(conf, mfrc522_reg_control_reg, 1, MFRC522_REG_FIELD(CONTROL_TSN));

    return mfrc522_drv_status_ok;
}

mfrc522_drv_status mfrc522_drv_irq_init(const mfrc522_drv_conf* conf, const mfrc522_drv_irq_conf* irq_conf)
{
    ERROR_IF_EQ(conf, NULL, mfrc522_drv_status_nullptr);
    ERROR_IF_EQ(irq_conf, NULL, mfrc522_drv_status_nullptr);

    /* Clear all interrupt flags */
    mfrc522_drv_irq_clr(conf, mfrc522_reg_irq_all);

    /* Update IRQ flags */
    TRY_WRITE_MASKED(conf, mfrc522_reg_com_irq_en, irq_conf->irq_signal_inv, MFRC522_REG_FIELD(COMIEN_IRQ_INV));
    TRY_WRITE_MASKED(conf, mfrc522_reg_div_irq_en, irq_conf->irq_push_pull, MFRC522_REG_FIELD(DIVIEN_IRQ_PUSHPULL));

    return mfrc522_drv_status_ok;
}

mfrc522_drv_status mfrc522_drv_irq_clr(const mfrc522_drv_conf* conf, mfrc522_reg_irq irq)
{
    ERROR_IF_EQ(conf, NULL, mfrc522_drv_status_nullptr);

    /* Clear all interrupts if special flag was passed */
    if (mfrc522_reg_irq_all == irq) {
        TRY_WRITE_BYTE(conf, mfrc522_reg_com_irq, IRQ_ALL_COM_MASK);
        TRY_WRITE_BYTE(conf, mfrc522_reg_div_irq, IRQ_ALL_DIV_MASK);
    } else {
        /* Find correct register to clear interrupt flag in */
        u8 reg;
        if (irq & MFRC522_REG_IRQ_DIV) {
            irq &= ~MFRC522_REG_IRQ_DIV;
            reg = mfrc522_reg_div_irq;
        } else {
            reg = mfrc522_reg_com_irq;
        }
        TRY_WRITE_BYTE(conf, reg, 1 << irq);
    }

    return mfrc522_drv_status_ok;
}

mfrc522_drv_status mfrc522_drv_irq_en(const mfrc522_drv_conf* conf, mfrc522_reg_irq irq, bool enable)
{
    ERROR_IF_EQ(conf, NULL, mfrc522_drv_status_nullptr);
    ERROR_IF_EQ(irq, mfrc522_reg_irq_all, mfrc522_drv_status_nok);

    /* Find correct register to enable interrupt */
    u8 reg;
    if (irq & MFRC522_REG_IRQ_DIV) {
        irq &= ~MFRC522_REG_IRQ_DIV;
        reg = mfrc522_reg_div_irq_en;
    } else {
        reg = mfrc522_reg_com_irq_en;
    }
    TRY_WRITE_MASKED(conf, reg, enable, 0x01, irq);

    return mfrc522_drv_status_ok;
}

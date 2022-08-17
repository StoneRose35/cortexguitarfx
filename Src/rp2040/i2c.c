#include <stdint.h>
#include "hardware/regs/addressmap.h"
#include "hardware/regs/resets.h"
#include "hardware/regs/io_bank0.h"
#include "hardware/regs/pads_bank0.h"
#include "hardware/regs/i2c.h"
#include "hardware/rp2040_registers.h"
#include "i2c.h"


void initI2c(uint8_t slaveAdress)
{
    // unreset i2c
    *RESETS |= (1 << RESETS_RESET_I2C0_LSB);
    *RESETS &= ~(1 << RESETS_RESET_I2C0_LSB);
	while ((*RESETS_DONE & (1 << RESETS_RESET_I2C0_LSB)) == 0);

    // configure pads
    // • pull-up enabled
    // • slew rate limited
    // • schmitt trigger enabled
    *I2C_SCL_PAD_CNTR &= ~((1 << PADS_BANK0_GPIO0_PDE_LSB) | (1 << PADS_BANK0_GPIO0_SLEWFAST_LSB));
    *I2C_SCL_PAD_CNTR |= (1 << PADS_BANK0_GPIO0_PUE_LSB) | (1 << PADS_BANK0_GPIO0_SCHMITT_LSB);

    *I2C_SCL_PAD_CNTR &= ~((1 << PADS_BANK0_GPIO0_PDE_LSB) | (1 << PADS_BANK0_GPIO0_SLEWFAST_LSB));
    *I2C_SCL_PAD_CNTR |= (1 << PADS_BANK0_GPIO0_PUE_LSB) | (1 << PADS_BANK0_GPIO0_SCHMITT_LSB);

    // wire up pins to i2c
    *I2C_SCL_PIN_CNTR = 4;
    *I2C_SDA_PIN_CNTR = 4;

    // disable i2c
    *I2C_ENABLE_IC &= ~(1 << I2C_IC_ENABLE_ENABLE_LSB);

    // configure i2c device as master
    *I2C_IC_CON = (1 << I2C_IC_CON_SPEED_LSB) | 
                  (1 << I2C_IC_CON_IC_SLAVE_DISABLE_LSB) | 
                  (1 << I2C_IC_CON_IC_RESTART_EN_LSB) | 
                  (1 << I2C_IC_CON_MASTER_MODE_LSB);

    // configure clock, 100kHz in standard mode with a peripheral clock of 120 MHz
    /*
        MIN_SCL_HIGHtime = Minimum High Period
        MIN_SCL_HIGHtime =
        4000 ns for 100 kbps,
        600 ns for 400 kbps,
        260 ns for 1000 kbps,
        MIN_SCL_LOWtime = Minimum Low Period
        MIN_SCL_LOWtime =
        4700 ns for 100 kbps,
        1300 ns for 400 kbps,
        500 ns for 1000 kbps,
    */
    *I2C_IC_SS_SCL_HCNT=480;
    *I2C_IC_SS_SCL_LCNT=564;

    // set slave address
    *I2C_IC_TAR = slaveAdress;

    //enable i2c
    *I2C_ENABLE_IC |= (1 << I2C_IC_ENABLE_ENABLE_LSB);
}

uint8_t masterTransmit(uint8_t data,uint8_t lastCmd)
{
    int32_t txAbortSrc;
    // put data
    if (lastCmd !=0)
    {
        *I2C_IC_DATA_CMD = data & (1 << I2C_IC_DATA_CMD_STOP_LSB);
    }
    else
    {
        *I2C_IC_DATA_CMD = data;
    }

    // wait until transmission is done
    while ((*I2C_IC_STATUS & (1 << I2C_IC_STATUS_ACTIVITY_LSB))!=0);

    if ((*I2C_IC_RAW_INTR_STAT & (1 << I2C_IC_RAW_INTR_STAT_TX_ABRT_LSB)) !=0)
    {
        txAbortSrc=*I2C_IC_TX_ABRT_SOURCE;
        if ((txAbortSrc & (1 << I2C_IC_TX_ABRT_SOURCE_ARB_LOST_LSB))!=0)
        {
            return I2C_ERROR_ARBITRATION_LOST;
        }
        if((txAbortSrc & (1 << I2C_IC_TX_ABRT_SOURCE_ABRT_TXDATA_NOACK_LSB))!=0)
        {
            return I2C_ERROR_DATA_NACK;
        }
        if((txAbortSrc & (1 << I2C_IC_TX_ABRT_SOURCE_ABRT_7B_ADDR_NOACK_LSB))!=0)
        {
            return I2C_ERROR_SLAVE_ADDRESS_NACK;
        }
    }

    return 0;
}

uint8_t masterReceive(uint8_t lastCmd)
{
    uint8_t res;
    if (lastCmd !=0)
    {
        *I2C_IC_DATA_CMD = (1 << I2C_IC_DATA_CMD_STOP_LSB) | (1 << I2C_IC_DATA_CMD_CMD_LSB);
    }
    else
    {
        *I2C_IC_DATA_CMD = (1 << I2C_IC_DATA_CMD_CMD_LSB);
    }

    // wait until transmission is done
    while ((*I2C_IC_STATUS & (1 << I2C_IC_STATUS_ACTIVITY_LSB))!=0);

    // read back value
    res = (uint8_t)*I2C_IC_DATA_CMD;
    return res;

}

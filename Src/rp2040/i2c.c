#include <stdint.h>
#include "hardware/regs/addressmap.h"
#include "hardware/regs/resets.h"
#include "hardware/regs/io_bank0.h"
#include "hardware/regs/pads_bank0.h"
#include "hardware/regs/i2c.h"
#include "hardware/rp2040_registers.h"
#include "hardware/regs/m0plus.h"
#include "drivers/i2c.h"
#include "drivers/systick.h"


static inline void read_reg(volatile uint32_t* reg)
{
}


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

    *I2C_SDA_PAD_CNTR &= ~((1 << PADS_BANK0_GPIO0_PDE_LSB) | (1 << PADS_BANK0_GPIO0_SLEWFAST_LSB));
    *I2C_SDA_PAD_CNTR |= (1 << PADS_BANK0_GPIO0_PUE_LSB) |(1 << PADS_BANK0_GPIO0_SCHMITT_LSB);


    // wire up pins to i2c
    *I2C_SCL_PIN_CNTR = 3;
    *I2C_SDA_PIN_CNTR = 3;

    // disable i2c
    *I2C_ENABLE_IC &= ~(1 << I2C_IC_ENABLE_ENABLE_LSB);

    // configure i2c device as master
    *I2C_IC_CON = (1 << I2C_IC_CON_SPEED_LSB) | 
                  (1 << I2C_IC_CON_IC_SLAVE_DISABLE_LSB) | 
                  (1 << I2C_IC_CON_IC_RESTART_EN_LSB) | 
                  (1 << I2C_IC_CON_MASTER_MODE_LSB) |
                  (1 << I2C_IC_CON_TX_EMPTY_CTRL_LSB);

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

    // enable dma signals
    *I2C_IC_DMA_CR |= (1 << I2C_IC_DMA_CR_RDMAE_LSB) | (1 << I2C_IC_DMA_CR_TDMAE_LSB);

    // enable interrupt
    //*NVIC_ISER = (1 << 23);

    // set the threshhold levels to 1
    *I2C_IC_TX_TL = 0;
    *I2C_IC_RX_TL = 0;

    // set slave address
    *I2C_IC_TAR = slaveAdress;


    //enable i2c
    *I2C_ENABLE_IC |= (1 << I2C_IC_ENABLE_ENABLE_LSB);
}

void setTargetAddress(uint8_t address)
{
    // wait until transmission is done
    while ((*I2C_IC_STATUS & (1 << I2C_IC_STATUS_TFE_LSB))==0);
    // disable i2c
    *I2C_ENABLE_IC &= ~(1 << I2C_IC_ENABLE_ENABLE_LSB);

    *I2C_IC_TAR = address;

    *I2C_ENABLE_IC |= (1 << I2C_IC_ENABLE_ENABLE_LSB);
}

uint8_t masterTransmit(uint8_t data,uint8_t lastCmd)
{
    uint32_t txAbortSrc;
    uint8_t retval = 0;

    // return with an error code if an abort occurred due to a previous
    // operation
    /*
    if ((*I2C_IC_RAW_INTR_STAT & (1 << I2C_IC_RAW_INTR_STAT_TX_ABRT_LSB)) !=0)
    {
        txAbortSrc=*I2C_IC_TX_ABRT_SOURCE;
        (void)(*I2C_IC_CLR_TX_ABRT);
        if ((txAbortSrc & (1 << I2C_IC_TX_ABRT_SOURCE_ARB_LOST_LSB))!=0)
        {
            retval = I2C_ERROR_ARBITRATION_LOST;
        }
        else if((txAbortSrc & (1 << I2C_IC_TX_ABRT_SOURCE_ABRT_TXDATA_NOACK_LSB))!=0)
        {
            retval = I2C_ERROR_DATA_NACK;
        }
        else if((txAbortSrc & (1 << I2C_IC_TX_ABRT_SOURCE_ABRT_7B_ADDR_NOACK_LSB))!=0)
        {
            retval = I2C_ERROR_SLAVE_ADDRESS_NACK;
        }
        else
        {
            retval = I2C_GENERIC_TX_ERROR;
        }
        return retval;
    }
    */

    //block as long as fifo is completely full
    while ((*I2C_IC_TXFLR)>15);


    // put data
    if (lastCmd !=0)
    {
        *I2C_IC_DATA_CMD = data | (1 << I2C_IC_DATA_CMD_STOP_LSB);
    }
    else
    {
        *I2C_IC_DATA_CMD = data;
    }
    while((*I2C_IC_RAW_INTR_STAT & (1 << I2C_IC_RAW_INTR_STAT_TX_EMPTY_LSB))==0); // wait until address and read/write is transmitted

    txAbortSrc=*I2C_IC_TX_ABRT_SOURCE;
    if (txAbortSrc != 0)
    {
        (void)(*I2C_IC_CLR_TX_ABRT);
    }
    if (lastCmd == 1 || txAbortSrc != 0)
    {
        while((*I2C_IC_RAW_INTR_STAT & (1 << I2C_IC_RAW_INTR_STAT_STOP_DET_LSB))==0);
        (void)(*I2C_IC_CLR_STOP_DET);
    }
        
    if (txAbortSrc !=0)
    {
        if ((txAbortSrc & (1 << I2C_IC_TX_ABRT_SOURCE_ARB_LOST_LSB))!=0)
        {
            retval = I2C_ERROR_ARBITRATION_LOST;
        }
        else if((txAbortSrc & (1 << I2C_IC_TX_ABRT_SOURCE_ABRT_TXDATA_NOACK_LSB))!=0)
        {
            retval = I2C_ERROR_DATA_NACK;
        }
        else if((txAbortSrc & (1 << I2C_IC_TX_ABRT_SOURCE_ABRT_7B_ADDR_NOACK_LSB))!=0)
        {
            retval = I2C_ERROR_SLAVE_ADDRESS_NACK;
        }
        else
        {
            retval = I2C_GENERIC_TX_ERROR;
        }
    }
 

    return retval;
}

uint8_t masterReceive(uint8_t lastCmd)
{
    uint8_t res;
    volatile uint32_t systickStart, currentSystick;

    volatile uint8_t rxlvl;
    (void)*I2C_IC_CLR_INTR;

    if (lastCmd !=0)
    {
        *I2C_IC_DATA_CMD = (1 << I2C_IC_DATA_CMD_STOP_LSB) | (1 << I2C_IC_DATA_CMD_CMD_LSB);
    }
    else
    {
        *I2C_IC_DATA_CMD = (1 << I2C_IC_DATA_CMD_CMD_LSB);
    }

    // wait until byte is received
    rxlvl = *I2C_IC_RXFLR;
    systickStart = getTickValue();
    currentSystick = getTickValue();
    while (rxlvl==0 && currentSystick - systickStart < 2)
    {
        rxlvl = *I2C_IC_RXFLR;
        (void)*I2C_IC_CLR_TX_ABRT;
        currentSystick=getTickValue();
    }

    // read back value
    if (rxlvl > 0)
    {
        res = *I2C_IC_DATA_CMD_BYTE;
        return res;
    }
    else
    {
        return 0xFF;
    }
}

void startMasterReceive(uint8_t lastCmd)
{
    if (lastCmd !=0)
    {
        *I2C_IC_DATA_CMD = (1 << I2C_IC_DATA_CMD_STOP_LSB) | (1 << I2C_IC_DATA_CMD_CMD_LSB);
    }
    else
    {
        *I2C_IC_DATA_CMD = (1 << I2C_IC_DATA_CMD_CMD_LSB);
    }
}

uint8_t getTargetAddress()
{
    return (uint8_t)*I2C_IC_TAR;
}

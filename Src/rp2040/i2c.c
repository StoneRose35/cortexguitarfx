#include <stdint.h>
#include "hardware/regs/addressmap.h"
#include "hardware/regs/resets.h"
#include "hardware/regs/io_bank0.h"
#include "hardware/regs/pads_bank0.h"
#include "hardware/regs/i2c.h"
#include "hardware/rp2040_registers.h"
#include "hardware/regs/m0plus.h"
#include "i2c.h"


static inline void read_reg(volatile uint32_t* reg)
{}

static volatile void(*irqHandler)(uint8_t data,uint8_t address)=0;

void isr_i2c0_irq23()
{
    if(irqHandler!=0)
    {
        irqHandler(*I2C_IC_DATA_CMD & 0xFF,*I2C_IC_TAR &0xFF);
    }
    read_reg(I2C_IC_CLR_INTR);
}


void registerI2C0IRQ(void(*handler)(uint8_t data,uint8_t address))
{
    irqHandler = handler;
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
    // block as long as fifo is not empty
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

    if ((*I2C_IC_RAW_INTR_STAT & (1 << I2C_IC_RAW_INTR_STAT_TX_ABRT_LSB)) !=0)
    {
        txAbortSrc=*I2C_IC_TX_ABRT_SOURCE;
        read_reg(I2C_IC_CLR_TX_ABRT);
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
        else
        {
            return I2C_GENERIC_TX_ERROR;
        }
    }

    return 0;
}

uint8_t masterReceive(uint8_t lastCmd)
{
    uint8_t res;
    volatile uint8_t abort, rxlvl;
     read_reg(I2C_IC_CLR_INTR);
    // block as long as active
    while ((*I2C_IC_STATUS & (1 << I2C_IC_STATUS_ACTIVITY_LSB))!=0);

    if (lastCmd !=0)
    {
        *I2C_IC_DATA_CMD = (1 << I2C_IC_DATA_CMD_STOP_LSB) | (1 << I2C_IC_DATA_CMD_CMD_LSB);
    }
    else
    {
        *I2C_IC_DATA_CMD = (1 << I2C_IC_DATA_CMD_CMD_LSB);
    }

    // wait until byte is received
    abort = *I2C_IC_CLR_TX_ABRT;
    rxlvl = *I2C_IC_RXFLR;
    while (rxlvl==0 && abort == 0)
    {
        rxlvl = *I2C_IC_RXFLR;
        abort = *I2C_IC_CLR_TX_ABRT;
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

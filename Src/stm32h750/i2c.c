#include "i2c.h"
#include "stm32h750/stm32h750xx.h"
#include "stm32h750/stm32h750_cfg_pins.h"

static volatile uint8_t slave_address;
static volatile uint8_t firstCommand;

static volatile uint8_t sendBfr[4];
static volatile uint8_t nSend=0;

uint8_t I2CsendMultiple();

// I2C2
static void config_i2c_pin(uint8_t pinnr)
{
    GPIO_TypeDef *gpio;
    uint32_t port;
    uint32_t regbfr;
    port = pinnr >> 4;
    RCC->AHB4ENR |= (1 << port);
    gpio=(GPIO_TypeDef*)(GPIOA_BASE + port*0x400);
    regbfr = gpio->MODER;
    regbfr &= ~(3 << ((pinnr & 0xF)<<1));
    regbfr |= (2 << ((pinnr & 0xF)<<1));
    gpio->MODER=regbfr;
    regbfr = gpio->OTYPER;
    regbfr |= 1 << (pinnr & 0xF);
    gpio->OTYPER = regbfr;
    regbfr = gpio->PUPDR;
    regbfr &= ~(3 << ((pinnr & 0xF)<<1));
    gpio->PUPDR = regbfr;
    regbfr = gpio->AFR[(pinnr & 0xF)>>3];
    regbfr &= ~(0xF << ((pinnr & 0x7) << 2));
    regbfr |= 4 << ((pinnr & 0x7) << 2);
    gpio->AFR[(pinnr & 0xF)>>3] = regbfr; 
}

void dummy_read(volatile uint32_t reg)
{}

void initI2c(uint8_t slaveAdress)
{

    RCC->APB1LENR |= (1 << I2C_ENR);
    

    // taken from the libDaisy code
    I2C_BLOCK->TIMINGR = (0x6 << I2C_TIMINGR_PRESC_Pos) 
                    | (0x9 << I2C_TIMINGR_SCLDEL_Pos)
                    | (0 << I2C_TIMINGR_SDADEL_Pos)
                    | (0x43 << I2C_TIMINGR_SCLH_Pos)
                    | (0x5F << I2C_TIMINGR_SCLL_Pos);
    config_i2c_pin(I2C_SCL);
    config_i2c_pin(I2C_SDA);

    I2C_BLOCK->CR1 |= (1 << I2C_CR1_PE_Pos);
    slave_address = slaveAdress;
    firstCommand = 1;
}

uint8_t masterTransmit(uint8_t data,uint8_t lastCmd)
{
    sendBfr[nSend++] = data;
    if (lastCmd==1)
    {
        I2CsendMultiple();
    }
    return 0;
}

uint8_t I2CsendMultiple()
{
    uint32_t regbfr;
    uint16_t sCnt=0;
    while ((I2C_BLOCK->ISR & (1 << I2C_ISR_TXE_Pos))==0);
    
    regbfr = I2C_BLOCK->CR2;
    regbfr &= ~((I2C_CR2_SADD_Msk) | (1 << I2C_CR2_RD_WRN_Pos) | (0xFF << I2C_CR2_NBYTES_Pos));
    regbfr |= (slave_address << (I2C_CR2_SADD_Pos+1)) | (1 << I2C_CR2_START_Pos) | (nSend << I2C_CR2_NBYTES_Pos) | (1 << I2C_CR2_AUTOEND_Pos);
    I2C_BLOCK->CR2 = regbfr;

    while (sCnt < nSend)
    {
        I2C_BLOCK->TXDR = sendBfr[sCnt];
        sCnt++;
        while ((I2C_BLOCK->ISR & I2C_ISR_TXE)==0);
    }
    nSend=0;

    return 0;
}

uint8_t masterReceive(uint8_t lastCmd)
{
    uint8_t res;
    uint32_t regbfr;
    while ((I2C_BLOCK->ISR & (1 << I2C_ISR_BUSY_Pos))!=0);
    if (firstCommand==1)
    {
        regbfr = I2C_BLOCK->CR2;
        regbfr &= ~((I2C_CR2_SADD_Msk) | (1 << I2C_CR2_RD_WRN_Pos));
        regbfr |= (slave_address << I2C_CR2_SADD_Pos) | (1 << I2C_CR2_START_Pos) | (0xFF << I2C_CR2_NBYTES_Pos) 
                  | (1 << I2C_CR2_RD_WRN_Pos);
        I2C_BLOCK->CR2 |= regbfr;
        firstCommand=0;
    }
    if (lastCmd != 0)
    {
        firstCommand = 1;
    }
    while ((I2C_BLOCK->ISR & (1 << I2C_ISR_RXNE_Pos))==0);
    res = (uint8_t)I2C_BLOCK->RXDR;
    return res;
}

void setTargetAddress(uint8_t address)
{
    slave_address = address;
}
uint8_t getTargetAddress()
{
    return slave_address;
}
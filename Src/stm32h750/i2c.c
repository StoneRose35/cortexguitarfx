#include "i2c.h"
#include "stm32h750/stm32h750xx.h"
#include "stm32h750/stm32h750_cfg_pins.h"

static volatile uint8_t slave_address;
static volatile uint8_t firstCommand;

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

    RCC->APB1LENR |= (1 << RCC_APB1LENR_I2C2EN_Pos);
    
    // clock frequency of 120MHZ ,divided by (37+38)*16
    I2C2->TIMINGR |= (0xF << I2C_TIMINGR_PRESC_Pos) | ((37-1) << I2C_TIMINGR_SCLH_Pos) | 
    ((38-1) << I2C_TIMINGR_SCLL_Pos) | (4 << I2C_TIMINGR_SDADEL_Pos) | (10 << I2C_TIMINGR_SCLDEL_Pos);

    config_i2c_pin(I2C_SCL);
    config_i2c_pin(I2C_SDA);

    I2C2->CR1 |= (1 << I2C_CR1_PE_Pos);
    slave_address = slaveAdress;
    firstCommand = 1;
}

uint8_t masterTransmit(uint8_t data,uint8_t lastCmd)
{
    uint32_t regbfr;
    while ((I2C2->ISR & (1 << I2C_ISR_BUSY_Pos))!=0);
    if (firstCommand==1)
    {
        regbfr = I2C2->CR2;
        regbfr &= ~((I2C_CR2_SADD_Msk) | (1 << I2C_CR2_RD_WRN_Pos));
        regbfr |= (slave_address << I2C_CR2_SADD_Pos) | (1 << I2C_CR2_START_Pos) | (0xFF << I2C_CR2_NBYTES_Pos);
        I2C1->CR2 |= regbfr;
        firstCommand=0;
    }
    I2C2->TXDR = data;
    while ((I2C2->ISR & I2C_ISR_TXE)==0);

    if (lastCmd==1)
    {
        I2C2->CR2 |= (1 << I2C_CR2_STOP_Pos);
        firstCommand=1;
    }
    return 0;
}

uint8_t masterReceive(uint8_t lastCmd)
{
    uint8_t res;
    uint32_t regbfr;
    while ((I2C2->ISR & (1 << I2C_ISR_BUSY_Pos))!=0);
    if (firstCommand==1)
    {
        regbfr = I2C2->CR2;
        regbfr &= ~((I2C_CR2_SADD_Msk) | (1 << I2C_CR2_RD_WRN_Pos));
        regbfr |= (slave_address << I2C_CR2_SADD_Pos) | (1 << I2C_CR2_START_Pos) | (0xFF << I2C_CR2_NBYTES_Pos) 
                  | (1 << I2C_CR2_RD_WRN_Pos);
        I2C1->CR2 |= regbfr;
        firstCommand=0;
    }
    if (lastCmd != 0)
    {
        firstCommand = 1;
    }
    while ((I2C2->ISR & (1 << I2C_ISR_RXNE_Pos))==0);
    res = (uint8_t)I2C2->RXDR;
    return res;
}
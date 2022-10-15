#include "i2c.h"
#include "stm32f446/stm32f446xx.h"
#include "stm32f446/stm32f446_cfg_pins.h"

static volatile uint8_t slave_address;

static void config_i2c_pin(uint8_t pinnr)
{
    GPIO_TypeDef *gpio;
    uint32_t port;
    uint32_t regbfr;
    port = pinnr >> 4;
    RCC->AHB1ENR |= (1 << port);
    gpio=(GPIO_TypeDef*)(GPIOA_BASE + port*0x400);
    regbfr = gpio->MODER;
    regbfr &= ~(3 << ((pinnr & 0xF)<<1));
    regbfr |= (2 << ((pinnr & 0xF)<<1));
    gpio->MODER=regbfr;
    regbfr = gpio->PUPDR;
    regbfr &= ~(3 << ((pinnr & 0xF)<<1));
    regbfr |= (1  << ((pinnr & 0xF)<<1));
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
    uint32_t regbfr;
    RCC->APB1ENR |= (1 << RCC_APB1ENR_I2C1EN_Pos);
    // clock frequency of 45MHZ
    regbfr = I2C1->CR2;
    regbfr &= ~(0x3F); 
    regbfr |= (45);
    I2C1->CR2 = regbfr;

    // rise time of 44: 1000ns/(1/45MHZ)-1
    regbfr = I2C1->TRISE;
    regbfr &= ~(0x3F);
    regbfr |= (44);
    I2C1->TRISE = regbfr;

    // CCR: 5000ns/22.2ns = 225
    regbfr = I2C1->CCR;
    regbfr &= ~(0xFFF);
    regbfr |= (225);
    I2C1->CCR = regbfr;

    config_i2c_pin(I2C_SCL);
    config_i2c_pin(I2C_SDA);

    I2C1->CR1 |= (1 << I2C_CR1_PE_Pos);
    slave_address = slaveAdress;
    
}

uint8_t masterTransmit(uint8_t data,uint8_t lastCmd)
{
    if ((I2C1->SR2 & I2C_SR2_MSL)==0)
    {
        I2C1->CR1 |= (1 << I2C_CR1_START_Pos);
        while ((I2C1->SR1 & I2C_SR1_SB)==0);
        I2C1->DR = (slave_address << 1) | 0;
        while ((I2C1->SR1 & I2C_SR1_ADDR)==0);
        dummy_read(I2C1->SR2);
    }
    I2C1->DR = data;
    while ((I2C1->SR1 & I2C_SR1_TXE)==0);

    if (lastCmd==1)
    {
        I2C1->CR1 |= (1 << I2C_CR1_STOP_Pos);
        while ((I2C1->SR2 & I2C_SR2_MSL)!=0);
    }
    return 0;
}

uint8_t masterReceive(uint8_t lastCmd)
{
    uint8_t res;
    if ((I2C1->SR2 & I2C_SR2_MSL)==0)
    {
        I2C1->CR1 |= (1 << I2C_CR1_START_Pos);
        while ((I2C1->SR1 & I2C_SR1_SB)==0);
        I2C1->DR = (slave_address << 1) | 1;
        while ((I2C1->SR1 & I2C_SR1_ADDR)==0);
        dummy_read(I2C1->SR2);
    }
    while ((I2C1->SR1 & I2C_SR1_RXNE_Pos)==0);
    res = (uint8_t)I2C1->DR;
    return res;
}
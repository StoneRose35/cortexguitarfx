#include "stdint.h" 
#include "stm32h750/stm32h750xx.h"
#include "gpio.h"

void initGpio()
{

}

void setAsOutput(uint32_t pinNr)
{
    GPIO_TypeDef * gpio;
    uint32_t port;
    uint32_t reg;
    port = pinNr >> 4;
    RCC->AHB4ENR |= (1 << port);
    gpio=(GPIO_TypeDef*)(GPIOA_BASE + port*0x400);
    reg = gpio->MODER;
    reg &= ~(3 << ((pinNr & 0xF)<<1));
    reg &= (1 << ((pinNr & 0xF)<<1));
    gpio->MODER = reg;
    gpio->PUPDR &= ~(3 << ((pinNr & 0xF)<< 1));
}

void setAsInput(uint32_t pinNr,uint8_t PullState)
{
    GPIO_TypeDef * gpio;
    uint32_t port;
    uint32_t reg;
    port = pinNr >> 4;
    RCC->AHB4ENR |= (1 << port);
    gpio=(GPIO_TypeDef*)(GPIOA_BASE + port*0x400);
    reg = gpio->MODER;
    reg &= ~(3 << ((pinNr & 0xF)<<1));
    gpio->MODER = reg;
    reg = gpio->PUPDR;
    reg  &= ~(3 << ((pinNr & 0xF)<< 1));
    reg |= (PullState << ((pinNr & 0xF)<< 1));
    gpio->PUPDR = reg;
}

void setHigh(uint32_t pinNr)
{
    GPIO_TypeDef * gpio;
    uint32_t port;
    port = pinNr >> 4;
    gpio=(GPIO_TypeDef*)(GPIOA_BASE + port*0x400);
    gpio->BSRR = (1 << ((pinNr & 0xF)+15));
}

void setLow(uint32_t pinNr)
{
    GPIO_TypeDef * gpio;
    uint32_t port;
    port = pinNr >> 4;
    gpio=(GPIO_TypeDef*)(GPIOA_BASE + port*0x400);
    gpio->BSRR = (1 << (pinNr & 0xF));
}

uint8_t readPin(uint32_t pinNr)
{
    GPIO_TypeDef * gpio;
    uint32_t port;
    uint32_t reg;
    port = pinNr >> 4;
    gpio=(GPIO_TypeDef*)(GPIOA_BASE + port*0x400); 
    reg = (gpio->IDR & (1 << (pinNr & 0xF)));
    if (reg != 0)
    {
        return 1;
    }
    else
    {
        return 0;
    }
}

void setPin(uint32_t pinNr,uint8_t value)
{
    if (value != 0)
    {
        setHigh(pinNr);
    }
    else
    {
        setLow(pinNr);
    }
}
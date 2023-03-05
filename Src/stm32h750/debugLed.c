#include "debugLed.h"
#include "stm32h750/stm32h750xx.h"

#define DEBUG_LED (1*16+0)
void initDebugLed()
{
    
    GPIO_TypeDef *gpio;
    uint32_t port;
    uint32_t regbfr;
    port = DEBUG_LED >> 4;
    RCC->AHB1ENR |= (1 << port);
    gpio=(GPIO_TypeDef*)(GPIOA_BASE + port*0x400);
    regbfr = gpio->MODER;
    regbfr &= ~(3 << ((DEBUG_LED & 0xF)<<1));
    regbfr |= (1 << ((DEBUG_LED & 0xF)<<1));
    gpio->MODER=regbfr;
    gpio->PUPDR &= ~(3 << ((DEBUG_LED & 0xF)<<1));
}

void DebugLedOn()
{
    GPIO_TypeDef *gpio;
    uint32_t port;
    port = DEBUG_LED >> 4;
    gpio=(GPIO_TypeDef*)(GPIOA_BASE + port*0x400);
    gpio->BSRR = (1 << (DEBUG_LED & 0xF));
}
void DebugLedOff()
{
    GPIO_TypeDef *gpio;
    uint32_t port;
    port = DEBUG_LED >> 4;
    gpio=(GPIO_TypeDef*)(GPIOA_BASE + port*0x400);
    gpio->BSRR = (1 << ((DEBUG_LED & 0xF) + 16));
}
void DebugLedToggle()
{
    GPIO_TypeDef *gpio;
    uint32_t port;
    port = DEBUG_LED >> 4;
    gpio=(GPIO_TypeDef*)(GPIOA_BASE + port*0x400);
    gpio->ODR ^= (1 << (DEBUG_LED & 0xF));
}
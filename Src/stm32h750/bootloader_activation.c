#include "stdint.h"
#include "stm32h750/stm32h750xx.h"
#include "bootloader_activation.h"

void jumpToBootloader(void)
{
    uint32_t cfgr;
    // disable interrupts
    for (uint8_t c=0;c<8;c++)
    {
        NVIC->ICER[c] = 0xFFFFFFFF;
    }
    // clear pending interrupts
    for (uint8_t c=0;c<8;c++)
    {
        NVIC->ICPR[c]= 0xFFFFFFFF;
    }

    // disable all peripheral clocks
    RCC->AHB1ENR = 0;
    RCC->AHB2ENR = 0;
    RCC->AHB3ENR = 0;
    RCC->AHB4ENR = 0;
    RCC->APB1HENR = 0;
    RCC->APB1LENR = 0;
    RCC->APB2ENR = 0;   
    //RCC->APB2LPENR = 0; 
    RCC->APB3ENR = 0; 
    //RCC->APB3LPENR = 0; 
    RCC->APB4ENR = 0; 
    //RCC->APB4LPENR = 0; 

    // switch clock back to internal, disable pll's
    // switch to internal oscillator
    cfgr = RCC->CFGR;
    cfgr &= ~(3 << RCC_CFGR_SW_Pos);
    cfgr |= (2 << RCC_CFGR_SW_Pos);
    RCC->CFGR &= ~(7 << RCC_CFGR_SW_Pos);
    while ((RCC->CFGR & RCC_CFGR_SWS_Msk) != 0);

    // switch off all PLL's
    RCC->CR &= ~((1 << RCC_CR_PLL1ON_Pos) | (1 << RCC_CR_PLL2ON_Pos) | (1 << RCC_CR_PLL3ON_Pos));

    // jump to system memory
    __set_MSP(BOOTLOADER_START_ADDRESS);

    ((void (*)(void))(*((uint32_t *) (BOOTLOADER_START_ADDRESS + 4))))();
}
#include "stm32h750/stm32h750xx.h"
#include "systemClock.h"

void setupClock()
{
    uint32_t cfgr;
    // switch on external oscillator
    RCC->CR &= ~(1 << RCC_CR_HSEBYP_Pos);
    RCC->CR |= (1 << RCC_CR_HSEON_Pos);
    while ((RCC->CR & RCC_CR_HSERDY)==0);


    // enable sysconfig
    RCC->APB4ENR |= (1 << RCC_APB4ENR_SYSCFGEN_Pos);

    // switch to external oscillator
    cfgr = RCC->CFGR;
    cfgr &= ~(3 << RCC_CFGR_SW_Pos);
    cfgr |= (2 << RCC_CFGR_SW_Pos);
    RCC->CFGR = cfgr;
    while ((RCC->CFGR & (RCC_CFGR_SWS_HSE)) == 0);

    
    PWR->D3CR |= (3 << PWR_D3CR_VOS_Pos); // set scale 1 to d3 domain power
    while((PWR->D3CR & (1 << PWR_D3CR_VOSRDY_Pos)) == 0); // wait until ready
    SYSCFG->PWRCR |= (1 << SYSCFG_PWRCR_ODEN_Pos); // enable overdrive, we want to build stuff for rock&metal guitar after all :-)
    while ((PWR->D3CR& (1 << PWR_D3CR_VOSRDY_Pos)) == 0); // wait till ready

    // set pll source to HSE (2), set divider m1 (clock input for pll) to 4
    RCC->PLLCKSELR = (2 << RCC_PLLCKSELR_PLLSRC_Pos) | (4 << RCC_PLLCKSELR_DIVM1_Pos);


    // configure and enable pll for 480 MHz cpu clock
    RCC->PLL1DIVR = (240 << RCC_PLL1DIVR_N1_Pos) |
                    (2 << RCC_PLL1DIVR_P1_Pos) |
                    (5 << RCC_PLL1DIVR_Q1_Pos) |
                    (2 << RCC_PLL1DIVR_R1_Pos);
    RCC->PLLCFGR |= (RCC_PLLCFGR_PLL1RGE_2 << RCC_PLLCFGR_PLL1RGE_Pos);

    RCC->CR |= (1 << RCC_CR_PLL1ON_Pos);

    // wait for pll to be ready
    while ((RCC->CR & RCC_CR_PLL1RDY) == 0);

    // set dividers for the various buses
    // D1 domain
    RCC->D1CFGR = 0x8 | (4 << RCC_D1CFGR_D1PPRE_Pos); // HPRE = 2, D1PPRE=2
    // D2 Domain
    RCC->D2CFGR = (4 << RCC_D2CFGR_D2PPRE2_Pos) | (4 << RCC_D2CFGR_D2PPRE1_Pos); // D2PPRRE1 =2, D2PPRE2=2
    // D3 Domain
    RCC->D3CFGR = (4 << RCC_D3CFGR_D3PPRE_Pos);

    // divide by 16 to get 1MHz RTC
    RCC->CFGR |= (16 << RCC_CFGR_RTCPRE_Pos);

    // adapt flash latency
    FLASH->ACR |= 4;

    // switch to pll
    cfgr = RCC->CFGR;
    cfgr &= ~(7 << RCC_CFGR_SW_Pos);
    cfgr |= (3 << RCC_CFGR_SW_Pos);
    RCC->CFGR = cfgr;
    while ((RCC->CFGR & (RCC_CFGR_SWS_PLL1)) != RCC_CFGR_SWS_PLL1);


    // switch on additional pll for
    // ...


}
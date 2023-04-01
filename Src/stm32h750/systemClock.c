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

    
    PWR->CR3 = (1 << PWR_CR3_SCUEN_Pos)| (1 << PWR_CR3_LDOEN_Pos);
    while((PWR->CSR1 & (1 << PWR_CSR1_ACTVOSRDY_Pos)) == 0);
    cfgr = PWR->D3CR;
    cfgr &= ~(3 << PWR_D3CR_VOS_Pos);
    cfgr |= (3 << PWR_D3CR_VOS_Pos); // set scale 1 to d3 domain power
    PWR->D3CR = cfgr;
    //while((PWR->D3CR & (1 << PWR_D3CR_VOSRDY_Pos)) == 0); // wait until ready
    __asm("nop");
    SYSCFG->PWRCR |= (1 << SYSCFG_PWRCR_ODEN_Pos); // enable overdrive, we want to build stuff for rock&metal guitar after all :-)
    while ((PWR->D3CR& (1 << PWR_D3CR_VOSRDY_Pos)) == 0); // wait till ready

    // set pll source to HSE (2), set divider m1 (clock input for pll) to 4
    // m2 to 1 and m3 to 6
    RCC->PLLCKSELR = (2 << RCC_PLLCKSELR_PLLSRC_Pos) 
                    | (4 << RCC_PLLCKSELR_DIVM1_Pos) 
                    | (1 << RCC_PLLCKSELR_DIVM2_Pos) 
                    | (5 << RCC_PLLCKSELR_DIVM3_Pos);


    // configure and enable pll for 480 MHz cpu clock
    RCC->PLL1DIVR = ((240-1) << RCC_PLL1DIVR_N1_Pos) |
                    ((2-1) << RCC_PLL1DIVR_P1_Pos) |
                    ((5-1) << RCC_PLL1DIVR_Q1_Pos) |
                    ((2-1) << RCC_PLL1DIVR_R1_Pos);
    RCC->PLLCFGR |= (2 << RCC_PLLCFGR_PLL1RGE_Pos) | (1 << RCC_PLLCFGR_PLL2FRACEN_Pos) | 
                    (2 << RCC_PLLCFGR_PLL2RGE_Pos) | (1 << RCC_PLLCFGR_PLL3RGE_Pos);

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
    // FMC and SAI1
    RCC->PLL2DIVR = ((12-1) << RCC_PLL2DIVR_N2_Pos) | 
                    ((8-1) << RCC_PLL2DIVR_P2_Pos) |
                    ((2-1) << RCC_PLL2DIVR_Q2_Pos) |
                    ((1-1) << RCC_PLL2DIVR_R2_Pos);
    RCC->PLL2FRACR = 4096 << RCC_PLL2FRACR_FRACN2_Pos;

// residual: 0.000000MHz, m3: 5, n3: 192, f_vco: 614, f_pll: 25, p3: 25, mckdiv: 2.0
    RCC->PLL3DIVR = ((192-1) << RCC_PLL3DIVR_N3_Pos) | 
                    ((25-1) << RCC_PLL3DIVR_P3_Pos) |
                    ((4-1) << RCC_PLL3DIVR_Q3_Pos) |
                    ((32-1) << RCC_PLL3DIVR_R3_Pos);
    RCC->CR |= (1 << RCC_CR_PLL2ON_Pos) | (1 << RCC_CR_PLL3ON_Pos);
    
    while(((RCC->CR & (1 << RCC_CR_PLL2RDY_Pos))==0) || ((RCC->CR & (1 << RCC_CR_PLL3RDY_Pos))==0));

    // select pll2, r divider from the FMC
    RCC->D1CCIPR = (2 << RCC_D1CCIPR_FMCSEL_Pos);

    // select pll3 for the sai 1
    RCC->D2CCIP1R = (2 << RCC_D2CCIP1R_SAI1SEL_Pos);


}
#include "timer.h"
#include "stm32h750/stm32h750xx.h"
#include "systick.h"

// timer 5 as a 1MHz/us timer

void initTimer()
{
    RCC->APB1LENR &= ~(1 << RCC_APB1LENR_TIM5EN_Pos);
    RCC->APB1LENR |= (1 << RCC_APB1LENR_TIM5EN_Pos);    
    waitSysticks(1);
    TIM5->PSC=240-1;
    TIM5->ARR = 0xFFFFFFFF;
    TIM5->CNT = 0xFFFFFFF0;
    TIM5->EGR=(1 << TIM_EGR_UG_Pos);
    if ((TIM5->SR & (1 << TIM_SR_UIF_Pos)) != 0)
    {
        TIM5->SR &= ~(1 << TIM_SR_UIF_Pos);
    }
    TIM5->CR1=(1 << TIM_CR1_CEN_Pos);   

}

void resetTime()
{
    TIM5->CNT=0;
}

uint32_t getTimeLW()
{
    return TIM5->CNT;
}

void waitUs(uint32_t us)
{
    uint32_t t0=TIM5->CNT;
    uint32_t t1=TIM5->CNT;
    while (t1-t0 < us)
    {
        if (t1 < t0)
        {
            us -= (0xFFFFFFFF - t0);
            t0=0;
        }
        t1 = TIM5->CNT;
    }
}
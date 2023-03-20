#include "timer.h"
#include "stm32h750/stm32h750xx.h"

// timer 5 as a 1MHz/us timer

void initTimer()
{
    RCC->APB1LENR &= ~(1 << RCC_APB1LENR_TIM5EN_Pos);
    RCC->APB1LENR |= (1 << RCC_APB1LENR_TIM5EN_Pos);
    TIM5->PSC=240-1;
    TIM5->CNT=0xFFFFFFFF;
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
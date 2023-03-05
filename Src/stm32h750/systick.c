
#include "systick.h"
#include "system.h"
#include "stm32h750/stm32h750xx.h"
volatile static uint32_t ticks=0;
#define AHB_CLOCK 180000000

void SysTick_Handler()
{
    ticks++;
}

uint32_t getTickValue()
{
    return ticks;
}

void initSystickTimer()
{

    SysTick->LOAD = (AHB_CLOCK>>3)/100;
    SysTick->VAL = (AHB_CLOCK>>3)/100;
    SysTick->CTRL = (1 << SysTick_CTRL_ENABLE_Pos) | (1 << SysTick_CTRL_TICKINT_Pos);

}


void stopSystickTimer()
{
    SysTick->LOAD = (AHB_CLOCK>>3)/100;
    SysTick->VAL = (AHB_CLOCK>>3)/100;
    SysTick->CTRL &= ~(1 << SysTick_CTRL_ENABLE_Pos);
}

void waitSysticks(uint32_t dticks)
{
    uint32_t t0;
    uint32_t tlim;
    t0 = getTickValue();
    tlim = t0+dticks;
    while (getTickValue() <= tlim);
}

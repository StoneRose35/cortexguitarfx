
#include "systick.h"
#include "system.h"
#include "stm32h750/stm32h750xx.h"
#include "memoryRegions.h"

volatile static uint32_t ticks=0;
volatile int32_t encoderValOldSpeedMesurement=0;
extern volatile uint32_t encoderSpeed;
#define AHB_CLOCK 480000000

__ITCM_CODE_FLASH
void SysTick_Handler()
{
    uint32_t currentCnt = TIM3->CNT;
    ticks++;
    if (currentCnt > encoderValOldSpeedMesurement)
    {
        encoderSpeed = currentCnt - encoderValOldSpeedMesurement;
    }
    else
    {
        encoderSpeed = encoderValOldSpeedMesurement - currentCnt;
    }
    encoderValOldSpeedMesurement = currentCnt;
}

__ITCM_CODE_FLASH
uint32_t getTickValue()
{
    return ticks;
}

/**
 * generates a Systick every 10 ms
 */
void initSystickTimer()
{
    if (SysTick->CTRL & (1 << SysTick_CTRL_ENABLE_Pos))
    {
        return;
    }
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

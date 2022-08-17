#include "stm32f446/stm32f446xx.h"
#include "core.h"

#define __SOFT_FP__
void enableFpu()
{
	SCB->CPACR = 0xF << 20;
}
#include "stm32h750/stm32h750xx.h"
#include "core.h"

#define __SOFT_FP__
void enableFpu()
{
	SCB->CPACR = 0xF << 20;
}
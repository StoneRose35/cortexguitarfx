#include "stm32h750/stm32h750xx.h"
#include "core.h"

#define __SOFT_FP__
void enableFpu()
{
	SCB->CPACR |= ((3UL << 10*2) |             /* set CP10 Full Access               */
                 (3UL << 11*2)  );           /* set CP11 Full Access               */
}
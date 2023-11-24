#ifndef _SYSTICK_
#define _SYSTICK_

#include "systemChoice.h"
#include <stdint.h>
#ifdef HARDWARE

/**
 * @brief returns the number of ticks passed since the systick-time was been switched on
 * divide this number by 100 to get the runtime in seconds. The tick value is stored in a 32bit unsigned integer
 * so an overflow is (theoretically) expected every 497.102 days
 * 
 * @return the tick value
 */
uint32_t getTickValue();
void initSystickTimer();
void stopSystickTimer();
void waitSysticks(uint32_t);

#endif

#endif
#ifndef _TIMER_H_
#define _TIMER_H_
#include <stdint.h>



/**
 * @brief a us-timebase, i.e. a time which increases by one every microsecond
 * 
 */

/**
 * @brief initialize and start the time
 * 
 */
void initTimer();

/**
 * @brief reset the timer value to zeros
 * 
 */
void resetTime();

/**
 * @brief Get the lower word of a potentially 64bit timer value
 * 
 * @return uint32_t 
 */
uint32_t getTimeLW();


void waitUs(uint32_t us);
#endif
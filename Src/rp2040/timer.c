#include "stdint.h"
#include "timer.h"
#include "hardware/regs/addressmap.h"
#include "hardware/regs/timer.h"
#include "hardware/regs/watchdog.h"
#include "hardware/rp2040_registers.h"

void initTimer()
{
    // de-reset timer block
    *RESETS |= (1 << RESETS_RESET_TIMER_LSB);
    *RESETS &= ~(1 << RESETS_RESET_TIMER_LSB);
    while ((*RESETS_DONE & (1 << RESETS_RESET_TIMER_LSB)) == 0);

    *WATCHDOG_TICK = (1 << WATCHDOG_TICK_ENABLE_LSB) | (12 << WATCHDOG_TICK_CYCLES_LSB);
    *TIMER_TIMELW = 0;
    *TIMER_TIMEHW = 0;
}

void resetTime()
{
    *TIMER_TIMELW = 0;
    *TIMER_TIMEHW = 0;
}

uint32_t getTimeLW()
{
    uint32_t timelw;
    timelw = *TIMER_TIMELR;
    (void)*TIMER_TIMEHR;
    return timelw;
}
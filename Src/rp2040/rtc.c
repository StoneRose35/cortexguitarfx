#include "datetimeClock.h"
#include <stdint.h>
#ifdef RP2040_FEATHER
#include "hardware/regs/addressmap.h"
#include "hardware/regs/rtc.h"
#include "hardware/regs/resets.h"
#include "hardware/regs/clocks.h"
#include "system.h"
#include "stringFunctions.h"

#define RTC_CLKDIV_M1 ((volatile uint32_t*)(RTC_BASE + RTC_CLKDIV_M1_OFFSET))
#define RTC_SETUP_0 ((volatile uint32_t*)(RTC_BASE + RTC_SETUP_0_OFFSET))
#define RTC_SETUP_1 ((volatile uint32_t*)(RTC_BASE + RTC_SETUP_1_OFFSET))
#define RTC_CTRL ((volatile uint32_t*)(RTC_BASE + RTC_CTRL_OFFSET))
#define RTC_RTC_1 ((volatile uint32_t*)(RTC_BASE + RTC_RTC_1_OFFSET))
#define RTC_RTC_0 ((volatile uint32_t*)(RTC_BASE + RTC_RTC_0_OFFSET))
#define RESETS ((volatile uint32_t*)(RESETS_BASE + RESETS_RESET_OFFSET))
#define RESETS_DONE ((volatile uint32_t*)(RESETS_BASE + RESETS_RESET_DONE_OFFSET))
#define CLK_RTC_CTRL ((volatile uint32_t*)(CLOCKS_BASE + CLOCKS_CLK_RTC_CTRL_OFFSET))
#define CLK_RTC_DIV ((volatile uint32_t*)(CLOCKS_BASE + CLOCKS_CLK_RTC_DIV_OFFSET))
#define RTC_PRESCALE_DIV 1000
#endif

uint8_t initDatetimeClock()
{
    // enable rtc clk
    *CLK_RTC_CTRL = (3 << CLOCKS_CLK_RTC_CTRL_AUXSRC_LSB) | (1 << CLOCKS_CLK_RTC_CTRL_ENABLE_LSB);

    // divide 
    *CLK_RTC_DIV = ((F_XOSC/RTC_PRESCALE_DIV) << CLOCKS_CLK_RTC_DIV_INT_LSB);

    // enable the component
    *RESETS |= (1 << RESETS_RESET_RTC_LSB);
    *RESETS &= ~(1 << RESETS_RESET_RTC_LSB);
	while ((*RESETS_DONE & (1 << RESETS_RESET_RTC_LSB)) == 0);

    // divide by 1000
    *RTC_CLKDIV_M1 = RTC_PRESCALE_DIV-1;

    // set 1.1.1970 00:00:00 as the initial date and time
    *RTC_SETUP_0= (1970 << RTC_SETUP_0_YEAR_LSB) | (1 << RTC_SETUP_0_MONTH_LSB) | (1 << RTC_SETUP_0_DAY_LSB);
    *RTC_SETUP_1=(4 << RTC_SETUP_1_DOTW_LSB);

    // load date and time value 
    *RTC_CTRL = RTC_CTRL_LOAD_BITS;

    // enable the rtc
    *RTC_CTRL = (1 << RTC_CTRL_RTC_ENABLE_LSB);

    // only return when the clock is running
    while ((*RTC_CTRL & (1 << RTC_CTRL_RTC_ACTIVE_LSB))==0);
    return 0;

}

uint8_t setHour(uint8_t h)
{
    if (h < 24)
    {
        *RTC_CTRL=0;
        while ((*RTC_CTRL & (1 << RTC_CTRL_RTC_ACTIVE_LSB))>0);
        uint32_t rval=*RTC_SETUP_1;
        rval &= ~(RTC_SETUP_1_HOUR_BITS);
        rval |= (h << RTC_SETUP_1_HOUR_LSB);
        *RTC_SETUP_1=rval;
        *RTC_CTRL = RTC_CTRL_LOAD_BITS;
        *RTC_CTRL = RTC_CTRL_RTC_ENABLE_BITS;
        while ((*RTC_CTRL & (1 << RTC_CTRL_RTC_ACTIVE_LSB))==0);
        return 0;
    }
    return 1;
}

uint8_t setMinute(uint8_t m)
{
    if (m < 60)
    {
        *RTC_CTRL=0;
        while ((*RTC_CTRL & (1 << RTC_CTRL_RTC_ACTIVE_LSB))>0);
        uint32_t rval=*RTC_SETUP_1;
        rval &= ~(RTC_SETUP_1_MIN_BITS);
        rval |= (m << RTC_SETUP_1_MIN_LSB);
        *RTC_SETUP_1=rval;
        *RTC_CTRL = RTC_CTRL_LOAD_BITS;
        *RTC_CTRL = RTC_CTRL_RTC_ENABLE_BITS;
        while ((*RTC_CTRL & (1 << RTC_CTRL_RTC_ACTIVE_LSB))==0);
        return 0;
    }
    return 1;
}

uint8_t setSecond(uint8_t s)
{
    if (s < 60)
    {
        *RTC_CTRL=0;
        while ((*RTC_CTRL & (1 << RTC_CTRL_RTC_ACTIVE_LSB))>0);
        uint32_t rval=*RTC_SETUP_1;
        rval &= ~(RTC_SETUP_1_SEC_BITS);
        rval |= (s << RTC_SETUP_1_SEC_LSB);
        *RTC_SETUP_1=rval;
        *RTC_CTRL = RTC_CTRL_LOAD_BITS;
        *RTC_CTRL = RTC_CTRL_RTC_ENABLE_BITS;
        while ((*RTC_CTRL & (1 << RTC_CTRL_RTC_ACTIVE_LSB))==0);
        return 0;
    }
    return 1;
}

uint8_t setYear(uint16_t y)
{
    *RTC_CTRL=0;
    while ((*RTC_CTRL & (1 << RTC_CTRL_RTC_ACTIVE_LSB))>0);
    uint32_t rval=*RTC_SETUP_0;
    rval &= ~(RTC_SETUP_0_YEAR_BITS);
    rval |= (y << RTC_SETUP_0_YEAR_LSB);
    *RTC_SETUP_0=rval;
    *RTC_CTRL = RTC_CTRL_LOAD_BITS;
    *RTC_CTRL = RTC_CTRL_RTC_ENABLE_BITS;
    while ((*RTC_CTRL & (1 << RTC_CTRL_RTC_ACTIVE_LSB))==0);
    return 0;
}

uint8_t setMonth(uint8_t m)
{
    if(m<13 && m>0)
    {
        *RTC_CTRL=0;
        while ((*RTC_CTRL & (1 << RTC_CTRL_RTC_ACTIVE_LSB))>0);
        uint32_t rval=*RTC_SETUP_0;
        rval &= ~(RTC_SETUP_0_MONTH_BITS);
        rval |= (m << RTC_SETUP_0_MONTH_LSB);
        *RTC_SETUP_0=rval;
        *RTC_CTRL = RTC_CTRL_LOAD_BITS;
        *RTC_CTRL = RTC_CTRL_RTC_ENABLE_BITS;
        while ((*RTC_CTRL & (1 << RTC_CTRL_RTC_ACTIVE_LSB))==0);
        return 0;
    }
    return 1;
}

uint8_t setDay(uint8_t d)
{
    if(d>0 && d<32)
    {
        *RTC_CTRL=0;
        while ((*RTC_CTRL & (1 << RTC_CTRL_RTC_ACTIVE_LSB))>0);
        uint32_t rval=*RTC_SETUP_0;
        rval &= ~(RTC_SETUP_0_DAY_BITS);
        rval |= (d << RTC_SETUP_0_DAY_LSB);
        *RTC_SETUP_0=rval;
        *RTC_CTRL = RTC_CTRL_LOAD_BITS;
        *RTC_CTRL = RTC_CTRL_RTC_ENABLE_BITS;
        while ((*RTC_CTRL & (1 << RTC_CTRL_RTC_ACTIVE_LSB))==0);
        return 0;
    }
    return 1;
}

uint8_t getHour()
{
    return ((*RTC_RTC_0 & RTC_RTC_0_HOUR_BITS) >> RTC_RTC_0_HOUR_LSB) & 0xFF;
}

uint8_t getMinute()
{
    return ((*RTC_RTC_0 & RTC_RTC_0_MIN_BITS) >> RTC_RTC_0_MIN_LSB) & 0xFF;
}

uint8_t getSecond()
{
    return ((*RTC_RTC_0 & RTC_RTC_0_SEC_BITS) >> RTC_RTC_0_SEC_LSB) & 0xFF;
}

uint16_t getYear()
{
    return ((*RTC_RTC_1 & RTC_RTC_1_YEAR_BITS) >> RTC_RTC_1_YEAR_LSB) & 0xFFFF;
}

uint8_t getMonth()
{
    return ((*RTC_RTC_1 & RTC_RTC_1_MONTH_BITS) >> RTC_RTC_1_MONTH_LSB) & 0xFF;
}

uint8_t getDay()
{
    return ((*RTC_RTC_1 & RTC_RTC_1_DAY_BITS) >> RTC_RTC_1_DAY_LSB) & 0xFF;
}

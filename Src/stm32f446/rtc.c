#include "stm32f446/stm32f446xx.h"
#include "datetimeClock.h"

uint8_t initDatetimeClock()
{
    RCC->APB1ENR |= (1 << RCC_APB1ENR_PWREN_Pos);
    PWR->CR |= (1 << PWR_CR_DBP_Pos);
    RCC->BDCR = (3 << RCC_BDCR_RTCSEL_Pos) | (1 << RCC_BDCR_RTCEN_Pos); // set hse divided by the prescaler and enable the rtc
    // disable write-protection
    //1. Write ‘0xCA’ into the RTC_WPR register.
    //2. Write ‘0x53’ into the RTC_WPR register.
    RTC->WPR = 0xCA;
    RTC->WPR = 0x53;

    RTC->ISR |= (1 << RTC_ISR_INIT_Pos);
    while ((RTC->ISR & RTC_ISR_INITF)==0);

    RTC->DR = (2 << RTC_DR_YT_Pos) | (2 << RTC_DR_YU_Pos) | (1 << RTC_DR_MU_Pos) | (1 << RTC_DR_DU) | (6 << RTC_DR_WDU_Pos);

    RTC->ISR &= ~(1 << RTC_ISR_INIT_Pos);

    return 0;
}

uint8_t setHour(uint8_t h)
{
    uint8_t ht, hu;
    uint32_t regbfr;
    ht = h/10;
    hu =h - ht;

    RTC->ISR |= (1 << RTC_ISR_INIT_Pos);
    while ((RTC->ISR & RTC_ISR_INITF)==0);
    regbfr = RTC->TR;
    regbfr &= ~((RTC_TR_HT_Msk) | (RTC_TR_HU_Msk));
    regbfr |= (ht << RTC_TR_HT_Pos) | (hu << RTC_TR_HU_Pos);
    RTC->TR = regbfr;
    RTC->ISR &= ~(1 << RTC_ISR_INIT_Pos);
    return 0;
}

uint8_t setMinute(uint8_t m)
{
    uint8_t mt, mu;
    uint32_t regbfr;
    mt = m/10;
    mu =m - mt;

    RTC->ISR |= (1 << RTC_ISR_INIT_Pos);
    while ((RTC->ISR & RTC_ISR_INITF)==0);
    regbfr = RTC->TR;
    regbfr &= ~((RTC_TR_MNT_Msk) | (RTC_TR_MNU_Msk));
    regbfr |= (mt << RTC_TR_MNT_Pos) | (mu << RTC_TR_MNU_Pos);
    RTC->TR = regbfr;
    RTC->ISR &= ~(1 << RTC_ISR_INIT_Pos);
    return 0;
}

uint8_t setSecond(uint8_t s)
{
    uint8_t st, su;
    uint32_t regbfr;
    st = s/10;
    su =s - st;

    RTC->ISR |= (1 << RTC_ISR_INIT_Pos);
    while ((RTC->ISR & RTC_ISR_INITF)==0);
    regbfr = RTC->TR;
    regbfr &= ~((RTC_TR_ST_Msk) | (RTC_TR_SU_Msk));
    regbfr |= (st << RTC_TR_ST_Pos) | (su << RTC_TR_SU_Pos);
    RTC->TR = regbfr;
    RTC->ISR &= ~(1 << RTC_ISR_INIT_Pos);
    return 0;
}

uint8_t setYear(uint16_t y)
{
    uint8_t yt, yu;
    uint16_t yb;
    uint32_t regbfr;
    if (y < 2000)
    {
        return 1;
    }
    yb = y - 2000;
    yt = yb/10;
    yu =yb - yt;

    RTC->ISR |= (1 << RTC_ISR_INIT_Pos);
    while ((RTC->ISR & RTC_ISR_INITF)==0);
    regbfr = RTC->DR;
    regbfr &= ~((RTC_DR_YT_Msk) | (RTC_DR_YU_Msk));
    regbfr |= (yt << RTC_DR_YT_Pos) | (yu << RTC_DR_YU_Pos);
    RTC->DR = regbfr;
    RTC->ISR &= ~(1 << RTC_ISR_INIT_Pos);

    return 0;
}

uint8_t setMonth(uint8_t m)
{
    uint8_t mt, mu;
    uint32_t regbfr;
    mt = m/10;
    mu =m - mt;

    RTC->ISR |= (1 << RTC_ISR_INIT_Pos);
    while ((RTC->ISR & RTC_ISR_INITF)==0);
    regbfr = RTC->DR;
    regbfr &= ~((RTC_DR_MT_Msk) | (RTC_DR_MU_Msk));
    regbfr |= (mt << RTC_DR_MT_Pos) | (mu << RTC_DR_MU_Pos);
    RTC->DR = regbfr;
    RTC->ISR &= ~(1 << RTC_ISR_INIT_Pos);

    return 0;
}

uint8_t setDay(uint8_t d)
{
    uint8_t dt, du;
    uint32_t regbfr;
    dt = d/10;
    du =d - dt;

    RTC->ISR |= (1 << RTC_ISR_INIT_Pos);
    while ((RTC->ISR & RTC_ISR_INITF)==0);
    regbfr = RTC->DR;
    regbfr &= ~((RTC_DR_DT_Msk) | (RTC_DR_DU_Msk));
    regbfr |= (dt << RTC_DR_MT_Pos) | (du << RTC_DR_MU_Pos);
    RTC->DR = regbfr;
    RTC->ISR &= ~(1 << RTC_ISR_INIT_Pos);

    return 0;
}

uint8_t getHour()
{
    uint8_t t, u;
    t = (RTC->TR & RTC_TR_HT_Msk) >> RTC_TR_HT_Pos;
    u = (RTC->TR & RTC_TR_HU_Msk) >> RTC_TR_HU_Pos;
    return 10*t+u;    
}

uint8_t getMinute()
{
    uint8_t t, u;
    t = (RTC->TR & RTC_TR_MNT_Msk) >> RTC_TR_MNT_Pos;
    u = (RTC->TR & RTC_TR_MNU_Msk) >> RTC_TR_MNU_Pos;
    return 10*t+u; 
}

uint8_t getSecond()
{
    uint8_t t, u;
    t = (RTC->TR & RTC_TR_ST_Msk) >> RTC_TR_ST_Pos;
    u = (RTC->TR & RTC_TR_SU_Msk) >> RTC_TR_SU_Pos;
    return 10*t+u; 
}

uint16_t getYear()
{
    uint8_t t, u;
    t = (RTC->DR & RTC_DR_YT_Msk) >> RTC_DR_YT_Pos;
    u = (RTC->DR & RTC_DR_YU_Msk) >> RTC_DR_YU_Pos;
    return 10*t+u+2000; 
}
uint8_t getMonth()
{
    uint8_t t, u;
    t = (RTC->DR & RTC_DR_MT_Msk) >> RTC_DR_MT_Pos;
    u = (RTC->DR & RTC_DR_MU_Msk) >> RTC_DR_MU_Pos;
    return 10*t+u; 
}
uint8_t getDay()
{
    uint8_t t, u;
    t = (RTC->DR & RTC_DR_DT_Msk) >> RTC_DR_DT_Pos;
    u = (RTC->DR & RTC_DR_DU_Msk) >> RTC_DR_DU_Pos;
    return 10*t+u; 
}
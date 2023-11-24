#ifndef _DATETIME_CLOCK_H_
#define _DATETIME_CLOCK_H_
#include <stdint.h>


uint8_t initDatetimeClock();

uint8_t setHour(uint8_t h);
uint8_t setMinute(uint8_t m);
uint8_t setSecond(uint8_t s);

uint8_t setYear(uint16_t y);
uint8_t setMonth(uint8_t m);
uint8_t setDay(uint8_t d);

uint8_t getHour();
uint8_t getMinute();
uint8_t getSecond();

uint16_t getYear();
uint8_t getMonth();
uint8_t getDay();

#endif
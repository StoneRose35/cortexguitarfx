#ifndef _LED_BLINK_H_
#define _LED_BLINK_H_
#include "stdint.h"

#define LED_BLINK_STATE_RUNNING 1
#define LED_BLINK_STATE_STOPPED 0
typedef struct 
{
    uint8_t state;
    uint8_t repetitionsCnt;
    uint16_t sysTicksCnt;
    uint16_t sysTicksOn;
    uint16_t sysTicksOff;
    uint8_t nRepetitions;
} LedBlinkType;


void processLedBlinkProgram(LedBlinkType*program);

void resetLedBlinkProgram(LedBlinkType*program);

#endif
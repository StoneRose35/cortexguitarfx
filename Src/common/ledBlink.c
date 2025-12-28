#include "ledBlink.h"
#include "drivers/gpio.h"
#include "hardware/rp2040_registers.h"
#include "globalConfig.h"
// called each systick
void processLedBlinkProgram(LedBlinkType*program)
{
    if (program->state != LED_BLINK_STATE_RUNNING)
    {
        setPin(CLIPPING_LED_INPUT,CLIPPING_LED_POLARITY); // off
        return;
    }
    if (program->sysTicksCnt == 0)
    {
        setPin(CLIPPING_LED_INPUT,CLIPPING_LED_POLARITY ^ 1); // on
        program->sysTicksCnt++;
        return;
    }

    
    if (program->sysTicksCnt > program->sysTicksOn)
    {
        setPin(CLIPPING_LED_INPUT,CLIPPING_LED_POLARITY); // off
    }
    else if (program->sysTicksCnt > program->sysTicksOn + program->sysTicksOff)
    {
        program->repetitionsCnt++;
        if (program->nRepetitions == 0xFF || program->repetitionsCnt < program->nRepetitions)
        {
            setPin(CLIPPING_LED_INPUT,CLIPPING_LED_POLARITY ^ 1); // on
        }
        else
        {
            program->state = LED_BLINK_STATE_STOPPED;
        }
    }
    program->sysTicksCnt++;
}

void resetLedBlinkProgram(LedBlinkType*program)
{
    program->repetitionsCnt = 0;
    program->sysTicksCnt = 0;
}
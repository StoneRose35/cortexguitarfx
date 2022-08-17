#include "neopixelDriver.h"

volatile uint8_t estate;
void setEngineState(uint8_t flag)
{
    estate = flag;
}

uint8_t getEngineState()
{
    return estate;
}
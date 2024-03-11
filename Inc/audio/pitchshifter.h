#ifndef _PITCHSHIFTER_H_
#define _PITCHSHIFTER_H_

#include "stdint.h"

typedef struct 
{
    int16_t delayLength1, delayLength2;
    uint16_t currentDelayPosition;
    int16_t delayIncrement; // fixed point decimal 1=1/4, decimal point after bis position 1, position 0 being lsb
    uint16_t buffersizePowerTwo;
    uint16_t buffersize;
} PitchshifterDataType;

#define PITSHIFTER_BUFFER_SIZE_TWOS_POWER 11
#define PITCHSHIFTER_BUFFER_SIZE (1<<PITSHIFTER_BUFFER_SIZE_TWOS_POWER)
float pitchShifterProcessSample(float sampleIn,PitchshifterDataType*data);
void initPitchshifter(PitchshifterDataType*data);
#endif
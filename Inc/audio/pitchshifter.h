#ifndef _PITCHSHIFTER_H_
#define _PITCHSHIFTER_H_

#include "stdint.h"

typedef struct 
{
    uint16_t delayIndex1, delayIndex2,delayIndex3;
    uint16_t currentDelayPosition;
    uint16_t delayIncrement; // fixed point decimal 1=1/4, decimal point after bis position 1, position 0 being lsb
} PitchshifterDataType;

#define PITCHSHIFTER_BUFFER_SIZE 512
#define PITSHIFTER_BUFFER_SIZE_TWOS_POWER 9
int16_t pitchShifterProcessSample(int16_t sampleIn,PitchshifterDataType*data);
void initPitchshifter(PitchshifterDataType*data);
#endif
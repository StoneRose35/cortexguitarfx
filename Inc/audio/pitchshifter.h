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

typedef struct 
{
    int16_t delayPointer1,delayPointer2;
    uint16_t currentDelayPosition;
    int16_t delayIncrement; // fixed point decimal 1=1/4, decimal point after bis position 1, position 0 being lsb
    uint16_t buffersizePowerTwo;
    uint16_t buffersize;
    uint16_t crossFadeWidth;
    uint8_t crossFadeWidthPwr2;
} Pitchshifter2DataType;

#define PITSHIFTER_BUFFER_SIZE_TWOS_POWER 11
#define PITCHSHIFTER_BUFFER_SIZE (1<<PITSHIFTER_BUFFER_SIZE_TWOS_POWER)
__attribute__ ((section (".ramfunc"))) int16_t pitchShifterProcessSample(int16_t sampleIn,PitchshifterDataType*data,volatile uint32_t*audioStatePtr);
void initPitchshifter(PitchshifterDataType*data);
__attribute__ ((section (".ramfunc"))) int16_t pitchShifter2ProcessSample(int16_t sampleIn,Pitchshifter2DataType*data,volatile uint32_t*audioStatePtr);
void initPitchshifter2(Pitchshifter2DataType*data);
#endif
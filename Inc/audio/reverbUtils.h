#ifndef _REVERB_UTILS_C_
#define _REVERB_UTILS_C_
#include "stdint.h"

typedef struct 
{
    int16_t coefficient;
    uint16_t delayPtr;
    uint16_t delayInSamples;
    int16_t oldValues;
    uint16_t bufferSize; 
    int16_t * delayLineIn;
    int16_t * delayLineOut;
} AllpassType;

typedef struct 
{
    int16_t * delayPointers[4];
    int16_t delayTimes[4];
    int16_t delayPointer;
    uint16_t diffusorSize;
} HadamardDiffuserType;

int16_t  allpassProcessSample(int16_t sampleIn,AllpassType*allpass,volatile uint32_t*);
void hadamardDiffuserProcessArray(int32_t * channels,HadamardDiffuserType*data,volatile uint32_t * audioStatePtr);

#endif
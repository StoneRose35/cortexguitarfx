#ifndef _REVERB_UTILS_C_
#define _REVERB_UTILS_C_
#include "stdint.h"

typedef struct 
{
    float coefficient;
    uint16_t delayPtr;
    uint16_t delayInSamples;
    float oldValues;
    uint16_t bufferSize; 
    float * delayLineIn;
    float * delayLineOut;
} AllpassType;

typedef struct 
{
    float * delayPointers[4];
    int16_t delayTimes[4];
    int16_t delayPointer;
    uint16_t diffusorSize;
} HadamardDiffuserType;

float  allpassProcessSample(float sampleIn,AllpassType*allpass,volatile uint32_t*);
void hadamardDiffuserProcessArray(float * channels,HadamardDiffuserType*data,volatile uint32_t * audioStatePtr);

#endif
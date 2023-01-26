#ifndef _REVERB_H_
#define _REVERB_H_
#include "stdint.h"

#ifndef FLOAT_AUDIO

typedef struct 
{
    int16_t coefficient;
    uint16_t delayPtr;
    uint16_t delayInSamples;
    int16_t oldValues;
    int16_t * delayLine;
} AllpassType;

typedef struct 
{
    AllpassType allpasses[4];
    uint16_t delayPointer;
    int16_t feedbackValues[4];
    int16_t delayPointers[4];
    int16_t mix;

} ReverbType;

int16_t reverbProcessSample(int16_t sampleIn,ReverbType*reverbData);
void initReverb(ReverbType*reverbData,int16_t);
void setReverbTime(int16_t reverbTime,ReverbType*reverbData);
int16_t  allpassProcessSample(int16_t sampleIn,AllpassType*allpass);
#else

typedef struct 
{
    float coefficient;
    uint16_t delayPtr;
    uint16_t delayInSamples;
    float oldValues;
    float * delayLine;
} AllpassType;

typedef struct 
{
    AllpassType allpasses[4];
    uint16_t delayPointer;
    float feedbackValues[4];
    float mix;

} ReverbType;

float reverbProcessSample(float sampleIn,ReverbType*reverbData);
void initReverb(ReverbType*reverbData,float);
void setReverbTime(float reverbTime,ReverbType*reverbData);

#endif


#endif
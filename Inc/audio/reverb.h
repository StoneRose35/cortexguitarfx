#ifndef _REVERB_H_
#define _REVERB_H_
#include "stdint.h"

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
    float delayTime;
    float mix;

} ReverbType;

float reverbProcessSample(float sampleIn,ReverbType*reverbData);
void initReverb(ReverbType*reverbData);
void setReverbTime(float reverbTime,ReverbType*reverbData);


#endif
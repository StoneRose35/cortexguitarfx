#ifndef _REVERB_H_
#define _REVERB_H_
#include "stdint.h"
#include "audio/reverbUtils.h"
#ifndef FLOAT_AUDIO

typedef struct 
{
    AllpassType allpasses[4];
    uint16_t delayPointer;
    int16_t feedbackValues[4];
    int16_t * delayPointers[4];
    int16_t mix;
    uint8_t paramNr;
} ReverbType;

int16_t reverbProcessSample(int16_t sampleIn,ReverbType*reverbData);
void initReverb(ReverbType*reverbData,int16_t);
void setReverbTime(int16_t reverbTime,ReverbType*reverbData);
const char * getReverbParameterSetName(ReverbType*reverbData);
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
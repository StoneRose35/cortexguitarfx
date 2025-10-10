#ifndef _DELAY_H_
#define _DELAY_H_
#include <stdint.h>

#include "audiotools.h"

typedef struct 
{
    float * delayLine; 
    uint32_t delayLinePtr;
    int32_t delayInSamples;
    float feedback; 
    uint32_t delayBufferLength;
    float mix;
    AudioProcessorFunc feedbackFunction;
    void * feebackData;
} DelayDataType;
 
void initDelay(DelayDataType*data,float * memoryPointer,uint32_t bufferLength);

float delayLineProcessSample(float sampleIn,DelayDataType*data);

// simply returnes the sample delayed 
float getDelayedSample(DelayDataType*data);

// adds a sample to the delay line
void addSampleToDelayline(float sampleIn,DelayDataType*data);

float delayLineWetProcessSample(float sampleIn,DelayDataType*data);



#endif
#ifndef _DELAY_H_
#define _DELAY_H_
#include <stdint.h>
#define DELAY_LINE_LENGTH (65536+32768)
#include "audiotools.h"

typedef struct 
{
    int16_t * delayLine; 
    uint32_t delayLinePtr;
    int32_t delayInSamples;
    int16_t feedback; 
    uint32_t delayBufferLength;
    int16_t mix;
    AudioProcessorFunc feedbackFunction;
    void * feebackData;
} DelayDataType;
 
void initDelay(DelayDataType*data,int16_t * memoryPointer,uint32_t bufferLength);

__attribute__ ((section (".ramfunc"))) int16_t delayLineProcessSample(int16_t sampleIn,DelayDataType*data);

__attribute__ ((section (".ramfunc"))) int16_t delayLineWetProcessSample(int16_t sampleIn,DelayDataType*data);

// simply returnes the sample delayed 
__attribute__ ((section (".ramfunc"))) int16_t getDelayedSample(DelayDataType*data);

// adds a sample to the delay line
__attribute__ ((section (".ramfunc"))) void addSampleToDelayline(int16_t sampleIn,DelayDataType*data);


#endif
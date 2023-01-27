#ifndef _DELAY_H_
#define _DELAY_H_
#include <stdint.h>
#define DELAY_LINE_LENGTH 65536

typedef int16_t(*feedbackProcessor)(int16_t,void*);

typedef struct 
{
    int16_t * delayLine; 
    uint32_t delayLinePtr;
    int32_t delayInSamples;
    int16_t feedback; 
    uint16_t delayBufferLength;
    int16_t mix;
    feedbackProcessor feedbackFunction;
    void * feebackData;
} DelayDataType;
 
void initDelay(DelayDataType*data,int16_t * memoryPointer,uint32_t bufferLength);

int16_t delayLineProcessSample(int16_t sampleIn,DelayDataType*data);

// simply returnes the sample delayed 
int16_t getDelayedSample(DelayDataType*data);

// adds a sample to the delay line
void addSampleToDelayline(int16_t sampleIn,DelayDataType*data);

int16_t * getDelayMemoryPointer();


#endif
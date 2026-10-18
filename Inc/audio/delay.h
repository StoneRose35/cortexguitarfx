#ifndef _DELAY_H_
#define _DELAY_H_
#include <stdint.h>
#include "preprocessorHelpers.h"
#include "memoryRegions.h"
#define MAX_DELAY_SINGLEBUFFER 65536



#if IS_NOT_POWER_OF_2(MAX_DELAY_SINGLEBUFFER)
#error maximum delay of a single buffer must be a power of two
#endif
#include "audiotools.h"

typedef struct 
{
    float * delayLine; 
    uint32_t delayLinePtr;
    int32_t delayInSamples;
    float feedback; 
    uint32_t delayBufferLength;
    float mix;
    float gainIn;
    AudioProcessorFunc feedbackFunction;
    void * feebackData;
    uint8_t frozen;

} DelayDataType;
 __QSPI_CODE
void initDelay(DelayDataType*data,float * memoryPointer,uint32_t bufferLength);

__ITCM_CODE
float delayLineProcessSample(float sampleIn,DelayDataType*data);

// simply returnes the sample delayed 
__ITCM_CODE
float getDelayedSample(DelayDataType*data);

// adds a sample to the delay line
__ITCM_CODE
void addSampleToDelayline(float sampleIn,DelayDataType*data);

__ITCM_CODE
float delayLineWetProcessSample(float sampleIn,DelayDataType*data);



#endif
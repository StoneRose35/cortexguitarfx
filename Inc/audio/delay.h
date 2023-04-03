#ifndef _DELAY_H_
#define _DELAY_H_
#include <stdint.h>
#define DELAY_LINE_LENGTH (0x800000UL)

#ifndef FLOAT_AUDIO
typedef int16_t(*feedbackProcessor)(int16_t,void*);

typedef struct 
{
    int16_t delayLine[DELAY_LINE_LENGTH]; 
    uint32_t delayLinePtr;
    int32_t delayInSamples;
    int16_t feedback; 
    int16_t mix;
    feedbackProcessor feedbackFunction;
    void * feebackData;
} DelayDataType;
 
int16_t delayLineProcessSample(int16_t sampleIn,DelayDataType*data);
#else
typedef float(*feedbackProcessor)(float,void*);

typedef struct 
{
    float * delayLine; 
    uint32_t delayLinePtr;
    int32_t delayInSamples;
    float feedback; 
    float mix;
    feedbackProcessor feedbackFunction;
    void * feebackData;
} DelayDataType;
 
float delayLineProcessSample(float sampleIn,DelayDataType*data);
float * getDelayLine();
void emptyDelayLine(float*delayLine);

#endif
void initDelay(DelayDataType*data);



DelayDataType * getDelayData();


#endif
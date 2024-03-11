#ifndef _FIR_FILTER_H_
#define _FIR_FILTER_H_
#include <stdint.h>
typedef struct 
{
    const float coefficients[64];
    float delayBuffer[64];
    uint8_t filterLength;
    uint8_t delayPointer;
} FirFilterType;

void initfirFilter(FirFilterType*data);
void addSample(float sampleIn,FirFilterType*data);
float processFirstHalf(FirFilterType*data); 
float processSecondHalf(FirFilterType*data);

float firFilterProcessSample(float sampleIn,FirFilterType*data);
void firFilterReset(FirFilterType*data);

#endif
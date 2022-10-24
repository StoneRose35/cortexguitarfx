#ifndef _FIR_FILTER_H_
#define _FIR_FILTER_H_
#include <stdint.h>
#ifndef FLOAT_AUDIO
typedef struct 
{
    const int16_t coefficients[64];
    int16_t delayBuffer[64];
    uint8_t filterLength;
    uint8_t delayPointer;
} FirFilterType;

void initfirFilter(FirFilterType*data);
void addSample(int16_t sampleIn,FirFilterType*data);
int16_t processFirstHalf(FirFilterType*data);
int16_t processSecondHalf(FirFilterType*data);

int16_t firFilterProcessSample(int16_t sampleIn,FirFilterType*data);
#else
typedef struct 
{
    const float coefficients[64];
    float delayBuffer[64];
    uint8_t filterLength;
    uint8_t delayPointer;
} FirFilterType;

float firFilterProcessSample(float sampleIn,FirFilterType*data);
void addSample(float sampleIn,FirFilterType*data);
float processFirstHalf(FirFilterType*data);
float processSecondHalf(FirFilterType*data);
#endif
#endif
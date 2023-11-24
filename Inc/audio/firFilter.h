#ifndef _FIR_FILTER_H_
#define _FIR_FILTER_H_
#include <stdint.h>
typedef struct 
{
    const int16_t coefficients[64];
    int16_t delayBuffer[64];
    uint8_t filterLength;
    uint8_t delayPointer;
} FirFilterType;

void initfirFilter(FirFilterType*data);
void addSample(int16_t sampleIn,FirFilterType*data);
__attribute__ ((section (".ramfunc"))) int32_t processFirstHalf(FirFilterType*data); 
__attribute__ ((section (".ramfunc"))) int32_t processSecondHalf(FirFilterType*data);

int16_t firFilterProcessSample(int16_t sampleIn,FirFilterType*data);
void firFilterReset(FirFilterType*data);

#endif
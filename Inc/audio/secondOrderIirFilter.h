#ifndef _SECOND_ORDER_IIR_FILTER_H_
#define _SECOND_ORDER_IIR_FILTER_H_
#include <stdint.h>
typedef struct 
{
    int32_t w[3];
    const int32_t coeffA[2];
    const int32_t coeffB[3];
    uint8_t bitRes;
} SecondOrderIirFilterType;

void initSecondOrderIirFilter(SecondOrderIirFilterType* data);

int16_t secondOrderIirFilterProcessSample(int16_t sampleIn,SecondOrderIirFilterType*data);
int16_t secondOrderIirFilterProcessSampleHiRes(int16_t sampleIn,SecondOrderIirFilterType*data);
#endif
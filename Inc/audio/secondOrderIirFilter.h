#ifndef _SECOND_ORDER_IIR_FILTER_H_
#define _SECOND_ORDER_IIR_FILTER_H_
#include <stdint.h>
typedef struct 
{
    int16_t x1,x2,y1,y2;
    int32_t acc;
    const int32_t coeffA[2];
    const int32_t coeffB[3];
} SecondOrderIirFilterType;

void initSecondOrderIirFilter(SecondOrderIirFilterType* data);

int16_t secondOrderIirFilterProcessSample(int16_t sampleIn,SecondOrderIirFilterType*data);
void secondOrderIirFilterReset(SecondOrderIirFilterType*data);
#endif
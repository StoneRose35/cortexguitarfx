#ifndef _SECOND_ORDER_IIR_FILTER_H_
#define _SECOND_ORDER_IIR_FILTER_H_
#include <stdint.h>
typedef struct 
{
    float x1,x2,y1,y2;
    float acc;
    const float coeffA[2];
    const float coeffB[3];
} SecondOrderIirFilterType;

void initSecondOrderIirFilter(SecondOrderIirFilterType* data);

float secondOrderIirFilterProcessSample(float sampleIn,SecondOrderIirFilterType*data);
void secondOrderIirFilterReset(SecondOrderIirFilterType*data);
#endif
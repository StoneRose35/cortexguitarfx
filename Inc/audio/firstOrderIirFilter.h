#ifndef _FIRST_ORDER_IIR_FILTER_H_
#define _FIRST_ORDER_IIR_FILTER_H_
#include <stdint.h>

typedef struct 
{
    int16_t oldVal;
    int16_t oldXVal;
    int16_t alpha;
} FirstOrderIirType;

int16_t firstOrderIirLowpassProcessSample(int16_t sampleIn,FirstOrderIirType*data);
int16_t firstOrderIirHighpassProcessSample(int16_t sampleIn,FirstOrderIirType*data);

#endif
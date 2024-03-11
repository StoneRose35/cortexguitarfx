#ifndef _THREEBANDEQ_H_
#define _THREEBANDEQ_H_
#include "stdint.h"
#include "secondOrderIirFilter.h"
typedef struct 
{
    SecondOrderIirFilterType * lowShelf;
    SecondOrderIirFilterType * midBand;
    SecondOrderIirFilterType * highShelf;

    float lowFactor; // ranging from 0 to 8
    float midFactor; 
    float highFactor; 
} ThreeBandEQType;

void initThreeBandEq(ThreeBandEQType*data);
float threeBandEqProcessSample(float sampleIn,ThreeBandEQType*data);
void threeBandEqReset(ThreeBandEQType*data);
#endif
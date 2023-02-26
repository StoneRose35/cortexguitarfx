#ifndef _THREEBANDEQ_H_
#define _THREEBANDEQ_H_
#include "stdint.h"
#include "secondOrderIirFilter.h"
typedef struct 
{
    SecondOrderIirFilterType * lowShelf;
    SecondOrderIirFilterType * midBand;
    SecondOrderIirFilterType * highShelf;

    int16_t lowFactor; // as q12, thus ranging from 8 to 0 in the usual q15 range
    int16_t midFactor; 
    int16_t highFactor; 
} ThreeBandEQType;

void initThreeBandEq(ThreeBandEQType*data);
int16_t threeBandEqProcessSample(int16_t sampleIn,ThreeBandEQType*data);

#endif
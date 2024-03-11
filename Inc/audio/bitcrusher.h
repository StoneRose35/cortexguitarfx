#ifndef _BITCRUSHER_H_
#define _BITCRUSHER_H_
#include <stdint.h>
typedef struct 
{
    uint32_t bitmask;
} BitCrusherDataType;


void initBitcrusher(BitCrusherDataType*data);

void setBitMask(uint8_t resolution,BitCrusherDataType*data);

float bitCrusherProcessSample(float sampleIn,BitCrusherDataType*data);

#endif
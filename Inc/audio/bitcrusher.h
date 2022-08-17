#ifndef _BITCRUSHER_H_
#define _BITCRUSHER_H_
#include <stdint.h>
typedef struct 
{
    uint16_t bitmask;
} BitCrusherDataType;


void initBitcrusher(BitCrusherDataType*data);

void setBitMask(uint8_t resolution,BitCrusherDataType*data);

int16_t bitCrusherProcessSample(int16_t sampleIn,BitCrusherDataType*data);

#endif
#ifndef _BITCRUSHER_H_
#define _BITCRUSHER_H_
#include <stdint.h>
#ifndef FLOAT_AUDIO
typedef struct 
{
    uint16_t bitmask;
} BitCrusherDataType;


void initBitcrusher(BitCrusherDataType*data);

void setBitMask(uint8_t resolution,BitCrusherDataType*data);

int16_t bitCrusherProcessSample(int16_t sampleIn,BitCrusherDataType*data);
#else
typedef struct 
{
    uint32_t bitmask;
} BitCrusherDataType;


void initBitcrusher(BitCrusherDataType*data);

void setBitMask(uint8_t resolution,BitCrusherDataType*data);

float bitCrusherProcessSample(float sampleIn,BitCrusherDataType*data);
#endif
#endif
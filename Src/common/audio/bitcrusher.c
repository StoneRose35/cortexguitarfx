#include "audio/bitcrusher.h"

#ifndef FLOAT_AUDIO

void initBitcrusher(BitCrusherDataType*data)
{
    data->bitmask= ~(0x7FFF);
}

void setBitMask(uint8_t resolution,BitCrusherDataType*data)
{
    data->bitmask=0;
    for (uint8_t c=0;c<(16-resolution);c++)
    {
        data->bitmask <<=1;
        data->bitmask += 1;
    }
    data->bitmask = ~(data->bitmask);
}

int16_t bitCrusherProcessSample(int16_t sampleIn,BitCrusherDataType*data)
{
    return (int16_t)(((uint16_t)sampleIn) & data->bitmask);
}

#else

void initBitcrusher(BitCrusherDataType*data)
{
    data->bitmask= ~(0x7FFFFF);
}

void setBitMask(uint8_t resolution,BitCrusherDataType*data)
{
    data->bitmask=0;
    for (uint8_t c=0;c<(24-resolution);c++)
    {
        data->bitmask <<=1;
        data->bitmask += 1;
    }
    data->bitmask = ~(data->bitmask);
}

__attribute__ ((section (".qspi_code")))
float bitCrusherProcessSample(float sampleIn,BitCrusherDataType*data)
{
    int32_t isample;
    isample=(float)(sampleIn*8388608.0f);
    return ((float)(int32_t)(((uint32_t)isample) & data->bitmask))/8388608.0f;
}

 #endif

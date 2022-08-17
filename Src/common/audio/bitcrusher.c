#include "audio/bitcrusher.h"


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

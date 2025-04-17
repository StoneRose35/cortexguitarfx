#include "audio/bitcrusher.h"
#include "memoryRegions.h"

__QSPI_CODE
void initBitcrusher(BitCrusherDataType*data)
{
    data->bitmask= ~(0x7FFFFF);
}

__ITCM_CODE
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

__ITCM_CODE
float bitCrusherProcessSample(float sampleIn,BitCrusherDataType*data)
{
    int32_t isample;
    isample=(float)(sampleIn*8388608.0f);
    return ((float)(int32_t)(((uint32_t)isample) & data->bitmask))/8388608.0f;
}


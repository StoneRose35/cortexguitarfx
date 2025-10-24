#include "audio/bitcrusher.h"
#include "memoryRegions.h"

__QSPI_CODE
void initBitcrusher(BitCrusherDataType*data)
{
    data->bitmask= 0xFFFFFF;
    data->halfvalue = 0;
}

__QSPI_CODE
void setBitMask(uint8_t resolution,BitCrusherDataType*data)
{
    data->bitmask=0;
    for (uint8_t c=0;c<(24-resolution);c++)
    {
        data->bitmask <<=1;
        data->bitmask += 1;
    }
    data->halfvalue = data->bitmask >> 1;
    data->bitmask = ~(data->bitmask) | 0xFF800000;
    
}

__ITCM_CODE
float bitCrusherProcessSample(float sampleIn,BitCrusherDataType*data)
{
    int32_t isample;
    isample=(int32_t)(sampleIn*8388608.0f);
    return ((float)(int32_t)(((uint32_t)isample + data->halfvalue) & data->bitmask))/8388608.0f;
}


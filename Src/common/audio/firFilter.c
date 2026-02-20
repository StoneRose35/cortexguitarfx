#include "audio/firFilter.h"
#include "stm32h750/helpers.h"
#include "memoryRegions.h"

__ITCM_CODE
void addSample(float sampleIn,FirFilterType*data)
{
    data->delayPointer--;
    data->delayPointer &= (uint8_t)(data->filterLength-1);
    *(data->delayBuffer + data->delayPointer)=sampleIn;
}

__ITCM_CODE
float firFilterProcessSample(float sampleIn,FirFilterType*data)
{
    addSample(sampleIn,data);
    float res;
    res = convolve(data->coefficients,data->delayBuffer,data->delayPointer);
    return res;
}

__QSPI_CODE
void initfirFilter(FirFilterType*data)
{
    data->filterLength=64;
    data->delayPointer=0;
    for(uint8_t c=0;c<data->filterLength;c++)
    {
        data->delayBuffer[c]=0.0f;
    }
}

__ITCM_CODE
void firFilterReset(FirFilterType*data)
{
    data->filterLength=64;
    data->delayPointer=0;
    for(uint8_t c=0;c<data->filterLength;c++)
    {
        data->delayBuffer[c]=0.0f;
    }
}



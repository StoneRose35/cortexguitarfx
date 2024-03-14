#include "audio/firFilter.h"
#include "stm32h750/helpers.h"


__attribute__((section (".qspi_code")))
void addSample(float sampleIn,FirFilterType*data)
{
    data->delayPointer--;
    data->delayPointer &= (uint8_t)(data->filterLength-1);
    *(data->delayBuffer + data->delayPointer)=sampleIn;
}

float processFirstHalf(FirFilterType*data)
{
    //convolve(data->coefficients,data->delayBuffer,data->delayPointer);
    return 0.0f;
}

float processSecondHalf(FirFilterType*data)
{
    //convolve(data->coefficients,data->delayBuffer,(data->delayPointer + (data->filterLength >> 1)) & (data->filterLength-1));
    return 0.0f;
}

__attribute__ ((section (".qspi_code")))
float firFilterProcessSample(float sampleIn,FirFilterType*data)
{
    addSample(sampleIn,data);
    float res;
    res = convolve(data->coefficients,data->delayBuffer,data->delayPointer);
    return res;
}

__attribute__((section (".qspi_code")))
void initfirFilter(FirFilterType*data)
{
    data->filterLength=64;
    data->delayPointer=0;
    for(uint8_t c=0;c<data->filterLength;c++)
    {
        data->delayBuffer[c]=0.0f;
    }
}

__attribute__((section (".qspi_code")))
void firFilterReset(FirFilterType*data)
{
    data->filterLength=64;
    data->delayPointer=0;
    for(uint8_t c=0;c<data->filterLength;c++)
    {
        data->delayBuffer[c]=0.0f;
    }
}



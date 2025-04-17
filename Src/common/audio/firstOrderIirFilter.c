#include "memoryRegions.h"
#include "audio/firstOrderIirFilter.h"


__ITCM_CODE
float firstOrderIirLowpassProcessSample(float sampleIn,FirstOrderIirType*data)
{
    data->oldVal = sampleIn + data->alpha*(data->oldVal - sampleIn);
    return data->oldVal;
}

__ITCM_CODE
float firstOrderIirDualCoeffLPProcessSample(float sampleIn,FirstOrderIirDualCoeffLPType*data)
{
    if (sampleIn > data->oldVal)
    {
        data->oldVal = sampleIn + data->alphaRising*(data->oldVal - sampleIn);
        return data->oldVal;
    }
    else
    {
        data->oldVal = sampleIn + data->alphaFalling*(data->oldVal - sampleIn);
        return data->oldVal;
    }
}

__ITCM_CODE
float firstOrderIirHighpassProcessSample(float sampleIn,FirstOrderIirType*data)
{
    data->oldVal = (1.0f + data->alpha)/2.0f*(sampleIn - data->oldXVal) + data->alpha*data->oldVal;
    data->oldXVal = sampleIn;
    return data->oldVal;
}

__QSPI_CODE
void firstOrderIirReset(FirstOrderIirType*data)
{
    data->oldVal=0.0f;
    data->oldXVal=0.0f;
}

__QSPI_CODE
void firstOrderIirDualCoeffLPReset(FirstOrderIirDualCoeffLPType*data)
{
    data->oldVal=0.0f;
    data->oldXVal=0.0f;
}


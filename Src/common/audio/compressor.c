#include "stdint.h"
#include "audio/compressor.h"
#include "stdio.h"
#include "fastExpLog.h"
#include "ln.h"
#include "memoryRegions.h"

__ITCM_CODE
float applyGain(float sample,float avgVolume,CompressorDataType*comp)
{
    float logAvg;
    float sampleInterm=0.0f;
    float gainFactor;

    logAvg=toDb(avgVolume);
    if (logAvg < comp->gainFunction.threshhold)
    {
        sampleInterm =sample;
    }
    else if (comp->gainFunction.gainReduction > 16.0f)
    {
        gainFactor = toLin(comp->gainFunction.threshhold);
        if (avgVolume > 0.000001f)
        {
            gainFactor /= avgVolume;
            sampleInterm = sample*gainFactor;
        }
    }
    else
    {
        gainFactor = toLin(comp->gainFunction.threshhold + (logAvg-comp->gainFunction.threshhold)/comp->gainFunction.gainReduction);
        if (avgVolume > 0.000001f)
        {
            gainFactor /=avgVolume;
            sampleInterm = sample*gainFactor;
        }
    }
    return sampleInterm;
}

__ITCM_CODE
float getMaxGain(CompressorDataType*comp)
{
    if (comp->gainFunction.gainReduction > 16.0f)
    {
        return toLin(comp->gainFunction.threshhold);
    }
    else
    {
        return toLin(comp->gainFunction.threshhold + ((-comp->gainFunction.threshhold)/(comp->gainFunction.gainReduction)));
    }
}

__ITCM_CODE
float compressorProcessSample(float sampleIn,CompressorDataType*data)
{
    float absSample;
    float sampleOut;

    sampleOut = applyGain(sampleIn,data->currentAvg,data);
    if(sampleOut < 0.0f)
    {
        absSample = -sampleOut;
    }
    else
    {
        absSample = sampleOut;
    }
    data->currentAvg = firstOrderIirDualCoeffLPProcessSample(absSample,&data->avgLowpass);
    return sampleOut;
}

__QSPI_CODE
void compressorReset(CompressorDataType*data)
{
    firstOrderIirDualCoeffLPReset(&data->avgLowpass);
    data->currentAvg = 0.0f;
}

#include "stdint.h"
#include "audio/compressor.h"
#include "stdio.h"
#include "fastExpLog.h"


__attribute__((section (".qspi_code")))
float applyGain2(float sample,float avgVolume,CompressorDataType*comp)
{
    float logAvg, expAvg;
    float sampleInterm;
    float gainFactor;

    logAvg=avgVolume;
    if (logAvg < comp->gainFunction.threshhold)
    {
        sampleInterm =sample;
    }
    else if (comp->gainFunction.gainReduction > 16.0f)
    {
        expAvg = fastexp(avgVolume);
        gainFactor = fastexp(comp->gainFunction.threshhold);
        if (expAvg > 0.000001f)
        {
            gainFactor /= expAvg;
            sampleInterm = sample*gainFactor;
        }
    }
    else
    {
        expAvg = fastexp(avgVolume);
        gainFactor = fastexp(comp->gainFunction.threshhold + (logAvg-comp->gainFunction.threshhold)/comp->gainFunction.gainReduction);
        if (avgVolume > 0.000001f)
        {
            gainFactor /=expAvg;
            sampleInterm = sample*gainFactor;
        }
    }
    return sampleInterm;
}

float compressor2ProcessSample(float sampleIn,CompressorDataType*data)
{
    float absSample;
    float sampleOut;
    float intermAvg;

    sampleOut = applyGain2(sampleIn,data->currentAvg,data);
    
    if(sampleOut < 0.0f)
    {
        absSample = -sampleOut;
    }
    else
    {
        absSample = sampleOut;
    }
    absSample = fastlog(absSample);
    intermAvg = firstOrderIirDualCoeffLPProcessSample(absSample,&data->avgLowpass);
    data->currentAvg = intermAvg; 
    return sampleOut;
}
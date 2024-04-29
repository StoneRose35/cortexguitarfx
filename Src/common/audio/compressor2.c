#include "stdint.h"
#include "audio/compressor.h"
#include "stdio.h"
#include "fastExpLog.h"
#include "ln.h"


__attribute__((section (".qspi_code")))
float applyGain2(float sample,float avgVolume,CompressorDataType*comp)
{
    float logAvg, expAvg;
    float sampleInterm=0.0f;
    float gainFactor;

    logAvg=avgVolume;
    if (logAvg < comp->gainFunction.threshhold)
    {
        sampleInterm =sample;
    }
    else if (comp->gainFunction.gainReduction > 16.0f)
    {
        expAvg = toLin(avgVolume);
        gainFactor = toLin(comp->gainFunction.threshhold);
        if (expAvg > 0.000001f)
        {
            gainFactor /= expAvg;
            sampleInterm = sample*gainFactor;
        }
    }
    else
    {
        expAvg = toLin(avgVolume);
        gainFactor = toLin(comp->gainFunction.threshhold + (logAvg-comp->gainFunction.threshhold)/comp->gainFunction.gainReduction);
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
    absSample = toDb(absSample);
    intermAvg = firstOrderIirDualCoeffLPProcessSample(absSample,&data->avgLowpass);
    data->currentAvg = intermAvg; 
    return sampleOut;
}
#include "stdint.h"
#include "audio/compressor.h"
#include "romfunc.h"
#include "fastExpLog.h"
#include "audio/firstOrderIirFilter.h"

int16_t applyGain2(int16_t sample,int16_t avgVolume,CompressorDataType*comp)
{
    int32_t gainFactor;
    int16_t sampleOut;
    int32_t sampleInterm=0;
    int16_t logAvg;

    logAvg = avgVolume;//fastlog(avgVolume);
    if (logAvg < comp->gainFunction.threshhold) // below threshhold, amplification 1
    {
        sampleInterm=sample;
    }
    else if (comp->gainFunction.gainReduction > 4)
    {
        gainFactor = fastexp(comp->gainFunction.threshhold)*32767;
        gainFactor /=fastexp(avgVolume);
        if (avgVolume != 0)
        {
            sampleInterm = (sample*gainFactor) >> 15;  // sampleInterm/avgVolume;
        }
    }
    else
    {
        gainFactor =  fastexp(comp->gainFunction.threshhold + ((logAvg-comp->gainFunction.threshhold) >> (comp->gainFunction.gainReduction)))*32767;
        gainFactor /=fastexp(avgVolume);
        if (avgVolume != 0)
        {
            sampleInterm = (sample*gainFactor) >> 15;
        }
    }
    sampleOut=(int16_t)sampleInterm;
    return sampleOut;
}




int16_t compressor2ProcessSample(int16_t sampleIn,CompressorDataType*data)
{
    int16_t absSample;
    int16_t sampleOut;
    int32_t intermAvg;

    sampleOut = applyGain2(sampleIn,data->currentAvg,data);
    
    if(sampleOut < 0)
    {
        absSample = -sampleOut;
    }
    else
    {
        absSample = sampleOut;
    }
    /*if(sampleIn < 0)
    {
        absSample = -sampleIn;
    }
    else
    {
        absSample = sampleIn;
    }*/
    absSample = fastlog(absSample);
    intermAvg = firstOrderIirDualCoeffLPProcessSample(absSample,&data->avgLowpass);
    data->currentAvg = (int16_t)intermAvg; 
    return sampleOut;
}

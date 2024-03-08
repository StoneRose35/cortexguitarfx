#include "stdint.h"
#include "audio/compressor.h"
#include "romfunc.h"
#include "stdio.h"



__attribute__((section (".qspi_code")))
float applyGain(float sample,float avgVolume,CompressorDataType*comp)
{
    float sample_reduced;
    float sampleOut;
    if (avgVolume < comp->gainFunction.threshhold)
    {
        sample_reduced = avgVolume;
        sampleOut=sample;
    }
    else if (comp->gainFunction.gainReduction > 16.0f)
    {
        sample_reduced = comp->gainFunction.threshhold;
        sampleOut=sample*sample_reduced;
        if (avgVolume > 0.000001f)
        {
            sampleOut = sampleOut/avgVolume;
        }
    }
    else
    {
        sample_reduced =  comp->gainFunction.threshhold + (avgVolume-comp->gainFunction.threshhold)/comp->gainFunction.gainReduction;
        sampleOut=sample*sample_reduced;
        if (avgVolume > 0.000001f)
        {
            sampleOut = sampleOut = avgVolume;
        }
    }

    return sampleOut;
}

void setAttack(int32_t attackInUs,CompressorDataType*data)
{
    float attackFloat;
    attackFloat = (float)(attackInUs);
    data->attack= 20.833f/attackFloat;
}

void setRelease(int32_t releaseInUs,CompressorDataType*data)
{
    float releaseFloat;
    releaseFloat = (float)releaseInUs;
    data->release= 20.833f/releaseFloat;
}

__attribute__((section (".qspi_code")))
float compressorProcessSample(float sampleIn,CompressorDataType*data)
{
    float absSample;
    float delta;
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
    delta = absSample - data->currentAvg;
    if (delta < 0.0f)
    {
        if(-delta > data->release)
        {
            data->currentAvg -= data->release;
        }
        else
        {
            data->currentAvg += delta;
        }
        if (data->currentAvg < 0)
        {
            data->currentAvg=0.0f;
        }
    }
    else
    {
        if(delta > data->attack)
        {
            data->currentAvg += data->attack;
        }
        else
        {
            data->currentAvg += delta;
        }
        if (data->currentAvg > 1.0f) // overflow in this case
        {
            data->currentAvg=1.0f;
        }
    }
    return sampleOut;
}

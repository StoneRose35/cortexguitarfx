#include "stdint.h"
#include "audio/compressor.h"
#include "romfunc.h"
#include "stdio.h"

#ifndef FLOAT_AUDIO
int16_t applyGain(int16_t sample,int16_t avgVolume,CompressorDataType*comp)
{
    int16_t sample_reduced;
    int16_t sampleOut;
    int32_t sampleInterm;
    if (avgVolume < comp->gainFunction.threshhold)
    {
        sample_reduced = avgVolume;
        sampleInterm=sample;
    }
    else if (comp->gainFunction.gainReduction > 4)
    {
        sample_reduced =  comp->gainFunction.threshhold;
        sampleInterm=sample*sample_reduced;
        if (avgVolume != 0)
        {
            sampleInterm = sampleInterm/avgVolume;
        }
    }
    else
    {
        sample_reduced =  comp->gainFunction.threshhold + ((avgVolume-comp->gainFunction.threshhold) >> (comp->gainFunction.gainReduction));
        sampleInterm=sample*sample_reduced;
        if (avgVolume != 0)
        {
            sampleInterm = sampleInterm/avgVolume;
        }
    }


    sampleOut=(int16_t)sampleInterm;

    return sampleOut;
}

void setAttack(int32_t attackInUs,CompressorDataType*data)
{
    float attackFloat, samplesFloat;
    attackFloat = int2float(attackInUs);
    samplesFloat = 32767.0f*20.833f/attackFloat;
    data->attack= (int16_t)float2int(samplesFloat);
}

void setRelease(int32_t releaseInUs,CompressorDataType*data)
{
    float releaseFloat, samplesFloat;
    releaseFloat = int2float(releaseInUs);
    samplesFloat = 32767.0f*20.833f/releaseFloat;
    data->release= (int16_t)float2int(samplesFloat);
}

int16_t compressorProcessSample(int16_t sampleIn,CompressorDataType*data)
{
    int16_t absSample;
    int16_t delta;
    int16_t sampleOut;

    sampleOut = applyGain(sampleIn,data->currentAvg,data);
    if(sampleOut < 0)
    {
        absSample = -sampleOut;
    }
    else
    {
        absSample = sampleOut;
    }
    delta = absSample - data->currentAvg;
    if (delta < 0)
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
            data->currentAvg=0;
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
        if (data->currentAvg < 0) // overflow in this case
        {
            data->currentAvg=(1 << 15)-1;
        }
    }
    return sampleOut;
}

#else

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
        sampleOut=sample*comp->gainFunction.threshhold/avgVolume;
    }
    else
    {
        sample_reduced =  comp->gainFunction.threshhold + (avgVolume-comp->gainFunction.threshhold)/comp->gainFunction.gainReduction;
        sampleOut=sample*sample_reduced/avgVolume;
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

#endif
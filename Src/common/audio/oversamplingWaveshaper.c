#include "audio/oversamplingWaveshaper.h"
#include "audio/secondOrderIirFilter.h"
#include <stdint.h>
#include "sai.h"

// oversampling factor as power of two
#define OVERSAMPLING_FACTOR 2 


__attribute__((section (".qspi_code")))
void initOversamplingWaveshaper(OversamplingWaveshaperDataType*data)
{
    data->oldValue = 0;
    initWaveShaper(&data->waveshaper,&waveShaperDefaultOverdrive);
}

//uint16_t oversampledBuffer[AUDIO_BUFFER_SIZE*2*(1 << OVERSAMPLING_FACTOR)];

__attribute__((section (".qspi_code")))
void  applyOversamplingDistortion(float*data,OversamplingWaveshaperDataType* waveshaper)
{
    float oversample;
    for (uint16_t c=0;c<AUDIO_BUFFER_SIZE*2*(1 << OVERSAMPLING_FACTOR);c++)
    {
        if ((c&OVERSAMPLING_FACTOR)!=0)
        {
            oversample=data[c >> OVERSAMPLING_FACTOR];
        }
        else
        {
            oversample=0;
        }

        if ((c&OVERSAMPLING_FACTOR)!=0)
        {
            data[c >> OVERSAMPLING_FACTOR] = oversample;
        }
    }
}

__attribute__ ((section (".qspi_code")))
float  OversamplingDistortionProcessSample(float sample,OversamplingWaveshaperDataType* waveshaper)
{
    float osVal1, osVal2, osVal3, osVal4;
    float  outVal;
    float diff = sample - waveshaper->oldValue;
    osVal1 = waveshaper->oldValue + diff*0.25f; // old + 0.25*(new-old)
    osVal2 = waveshaper->oldValue + diff*0.5f; // old +0.5*(new-old)
    osVal3 = sample - diff*0.25f; // old + 0.75*(new-old)
    osVal4 = sample; // new
    waveshaper->oldValue=sample;
    osVal1 = waveShaperProcessSample(osVal1,&waveshaper->waveshaper);
    osVal2 = waveShaperProcessSample(osVal2,&waveshaper->waveshaper);
    osVal3 = waveShaperProcessSample(osVal3,&waveshaper->waveshaper);
    osVal4 = waveShaperProcessSample(osVal4,&waveshaper->waveshaper);

    outVal = (osVal1+osVal2+osVal3+osVal4)*0.25f;
    //osVal4 = secondOrderIirFilterProcessSample(osVal4,&waveshaper->oversamplingFilter);
    return outVal;
}

__attribute__ ((section (".qspi_code")))
void oversamplingWaveshaperReset(OversamplingWaveshaperDataType*data)
{
    data->oldValue=0.0f;
}

#include "audio/delay.h"
#include "audio/audiotools.h"
#include "memoryRegions.h"



__QSPI_CODE
void initDelay(DelayDataType*data,float * memoryPointer,uint32_t bufferLength)
{
    data->delayLine = memoryPointer;
    data->delayBufferLength = bufferLength;
    for (uint32_t c=0;c<bufferLength;c++)
    {
        data->delayLine[c]=0.0f;
    }
    data->delayLinePtr=0;
}

__ITCM_CODE
float delayLineProcessSample(float sampleIn,DelayDataType*data)
{
    uint32_t delayIdx;
    float sampleOut;
    float sampleFedBack;
    volatile uint32_t * audioStatePtr = getAudioStatePtr();
    delayIdx = (data->delayLinePtr - data->delayInSamples) & (data->delayBufferLength -1);

    sampleOut = *(data->delayLine +delayIdx)*data->mix + sampleIn*(1.0f - data->mix);
    sampleFedBack = *(data->delayLine +delayIdx);
    if (data->feedbackFunction != 0)
    {
        sampleFedBack = data->feedbackFunction(sampleFedBack,data->feebackData,audioStatePtr);
    }
    sampleFedBack=data->feedback*sampleFedBack;

    *(data->delayLine + data->delayLinePtr) = sampleIn + sampleFedBack;
    data->delayLinePtr++;
    data->delayLinePtr &= (data->delayBufferLength -1UL);
    return sampleOut;
}

__ITCM_CODE
float delayLineWetProcessSample(float sampleIn,DelayDataType*data)
{
    uint32_t delayIdx;
    float sampleOut;
    float sampleFedBack;
    volatile uint32_t * audioStatePtr = getAudioStatePtr();
    delayIdx = (data->delayLinePtr - data->delayInSamples) & (data->delayBufferLength -1);

    sampleOut = *(data->delayLine +delayIdx);
    sampleFedBack = *(data->delayLine +delayIdx); //sampleOut;

    if (data->feedbackFunction != 0)
    {
        sampleFedBack = data->feedbackFunction(sampleFedBack,data->feebackData,audioStatePtr);
    }
    sampleFedBack *= data->feedback;

    *(data->delayLine + data->delayLinePtr) = sampleIn + sampleFedBack;
    data->delayLinePtr++;
    data->delayLinePtr &= (data->delayBufferLength -1);
    return sampleOut;
}

__ITCM_CODE
float getDelayedSample(DelayDataType*data)
{
    uint32_t delayIdx;
    float sampleOut;

    delayIdx = (data->delayLinePtr - data->delayInSamples) & (data->delayBufferLength -1);
    sampleOut = *(data->delayLine +delayIdx);

    return sampleOut;
}



__ITCM_CODE
void addSampleToDelayline(float sampleIn,DelayDataType*data)
{
    *(data->delayLine + data->delayLinePtr) = sampleIn;
    data->delayLinePtr++;
    data->delayLinePtr &= (data->delayBufferLength -1);
}


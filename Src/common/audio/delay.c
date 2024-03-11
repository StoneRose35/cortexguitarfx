#include "audio/delay.h"
#include "audio/audiotools.h"

__attribute__ ((section (".sdram_bss")))
float delayLineSdram[DELAY_LINE_LENGTH];

DelayDataType singletonDelay;

__attribute__((section (".qspi_code")))
DelayDataType * getDelayData()
{
    return &singletonDelay;
}


__attribute__((section (".qspi_code")))
void initDelay(DelayDataType*data,float * memoryPointer,uint32_t bufferLength)
{
    data->delayLine = (float*)memoryPointer;
    data->delayBufferLength = bufferLength;
    for (uint32_t c=0;c<bufferLength;c++)
    {
        data->delayLine[c]=0.0f;
    }
    data->delayInSamples=1;
    data->delayLinePtr=0;
    data->feedback=0.0f;
    data->mix=0.0f;
    data->feedbackFunction=0;
    data->feebackData=0;
}

__attribute__((section (".qspi_code")))
float delayLineProcessSample(float sampleIn,DelayDataType*data)
{
    uint32_t delayIdx;
    float sampleOut;
    float sampleFedBack;
    volatile uint32_t * audioStatePtr = getAudioStatePtr();
    delayIdx = (data->delayLinePtr - data->delayInSamples) & (data->delayBufferLength -1);

    sampleOut = *(data->delayLine +delayIdx)*data->mix + sampleIn*(1.0f - data->mix);
    sampleFedBack = sampleOut;
    if (data->feedbackFunction != 0)
    {
        sampleFedBack = data->feedbackFunction(sampleFedBack,data->feebackData);
    }
    sampleFedBack=data->feedback*sampleFedBack;
    sampleFedBack=clip(sampleFedBack,audioStatePtr);

    *(data->delayLine + data->delayLinePtr) = (sampleIn + sampleFedBack)*0.5f;
    data->delayLinePtr++;
    data->delayLinePtr &= (data->delayBufferLength -1UL);
    return sampleOut;
}

__attribute__((section (".qspi_code")))
float getDelayedSample(DelayDataType*data)
{
    uint32_t delayIdx;
    float sampleOut;

    delayIdx = (data->delayLinePtr - data->delayInSamples) & (data->delayBufferLength -1);
    sampleOut = *(data->delayLine +delayIdx);

    return sampleOut;
}

__attribute__((section (".qspi_code")))
float * getDelayMemoryPointer()
{
    return (float*)singletonDelay.delayLine;
}

__attribute__((section (".qspi_code")))
void addSampleToDelayline(float sampleIn,DelayDataType*data)
{
    *(data->delayLine + data->delayLinePtr) = sampleIn;
    data->delayLinePtr++;
    data->delayLinePtr &= (data->delayBufferLength -1);
}

__attribute__((section (".qspi_code")))
void clearDelayLine()
{
    for (uint32_t c=0;c<DELAY_LINE_LENGTH;c++)
    {
        delayLineSdram[c]=0.0f;
    }
}


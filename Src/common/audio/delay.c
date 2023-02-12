#include "audio/delay.h"
#include "audio/audiotools.h"
 
 int16_t delayMemory[DELAY_LINE_LENGTH];

 DelayDataType singletonDelay;

int16_t * getDelayMemoryPointer()
{
    return delayMemory;
}


void initDelay(DelayDataType*data,int16_t *  memoryPointer,uint32_t bufferLength)
{
    data->delayLine = memoryPointer;
    data->delayBufferLength=bufferLength;
    for (uint32_t c=0;c<bufferLength;c++)
    {
        data->delayLine[c]=0;
    }
    data->delayInSamples=1;
    data->delayLinePtr=0;
    data->feedback=0;
    data->mix=0;
    data->feedbackFunction=0;
    data->feebackData=0;
}
int16_t delayLineProcessSample(int16_t sampleIn,DelayDataType*data)
{
    uint32_t delayIdx;
    int16_t sampleOut;
    int32_t sampleFedBack;
    volatile uint32_t * audioStatePtr = getAudioStatePtr();
    delayIdx = (data->delayLinePtr - data->delayInSamples) & (data->delayBufferLength -1);

    sampleOut = ((*(data->delayLine +delayIdx)*data->mix) >> 15) + ((sampleIn*(32767 - data->mix)) >> 15);
    sampleFedBack = *(data->delayLine +delayIdx); //sampleOut;

    if (data->feedbackFunction != 0)
    {
        sampleFedBack = (int32_t)data->feedbackFunction((int16_t)sampleFedBack,data->feebackData);
    }
    sampleFedBack = ((data->feedback*sampleFedBack) >> 14);

    sampleFedBack = clip(sampleFedBack,audioStatePtr);
    *(data->delayLine + data->delayLinePtr) = (sampleIn>>1) + (((int16_t)sampleFedBack)>>1);
    data->delayLinePtr++;
    data->delayLinePtr &= (data->delayBufferLength -1);
    return sampleOut;
}

int16_t getDelayedSample(DelayDataType*data)
{
    uint32_t delayIdx;
    int16_t sampleOut;

    delayIdx = (data->delayLinePtr - data->delayInSamples) & (data->delayBufferLength -1);
    sampleOut = *(data->delayLine +delayIdx);

    return sampleOut;
}

void addSampleToDelayline(int16_t sampleIn,DelayDataType*data)
{
    *(data->delayLine + data->delayLinePtr) = sampleIn;
    data->delayLinePtr++;
    data->delayLinePtr &= (data->delayBufferLength -1);
}
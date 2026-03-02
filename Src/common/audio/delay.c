#include "audio/delay.h"
#include "audio/audiotools.h"
#include "memoryRegions.h"




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


float delayLineProcessSample(float sampleIn,DelayDataType*data)
{
    uint32_t delayIdx;
    float sampleOut;
    float sampleFedBack;


    delayIdx = (data->delayLinePtr - data->delayInSamples) & (data->delayBufferLength -1);

    sampleOut = data->mix*(*(data->delayLine +delayIdx) - sampleIn) + sampleIn;//  *(data->delayLine +delayIdx)*data->mix + sampleIn*(1.0f - data->mix);
    if (!data->frozen)
    {
        sampleFedBack = *(data->delayLine +delayIdx);
        if (data->feedbackFunction != 0)
        {
            sampleFedBack = data->feedbackFunction(sampleFedBack,data->feebackData);
        }
        sampleFedBack=data->feedback*sampleFedBack;
        *(data->delayLine + data->delayLinePtr) = sampleIn*data->gainIn + sampleFedBack;
    }
    else // frozen: 100% feedback, no more input
    {
        *(data->delayLine + data->delayLinePtr) = *(data->delayLine +delayIdx);
    }
    data->delayLinePtr++;
    data->delayLinePtr &= (data->delayBufferLength -1UL);


    return sampleOut;
}


float delayLineWetProcessSample(float sampleIn,DelayDataType*data)
{
    uint32_t delayIdx;
    float sampleOut;
    float sampleFedBack;
    delayIdx = (data->delayLinePtr - data->delayInSamples) & (data->delayBufferLength -1);

    sampleOut = *(data->delayLine +delayIdx);
    if (!data->frozen)
    {
        sampleFedBack = *(data->delayLine +delayIdx); //sampleOut;

        if (data->feedbackFunction != 0)
        {
            sampleFedBack = data->feedbackFunction(sampleFedBack,data->feebackData);
        }
        sampleFedBack *= data->feedback;

        *(data->delayLine + data->delayLinePtr) = sampleIn*data->gainIn + sampleFedBack;
    }
    else
    {
        *(data->delayLine + data->delayLinePtr) = *(data->delayLine +delayIdx);
    }
    data->delayLinePtr++;
    data->delayLinePtr &= (data->delayBufferLength -1);
    return sampleOut;
}



float getDelayedSample(DelayDataType*data)
{
    uint32_t delayIdx;
    float sampleOut;

    delayIdx = (data->delayLinePtr - data->delayInSamples) & (data->delayBufferLength -1);
    sampleOut = *(data->delayLine +delayIdx);

    return sampleOut;
}




void addSampleToDelayline(float sampleIn,DelayDataType*data)
{
    *(data->delayLine + data->delayLinePtr) = sampleIn;
    data->delayLinePtr++;
    data->delayLinePtr &= (data->delayBufferLength -1);
}


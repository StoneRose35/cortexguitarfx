#include "audio/delay.h"

 
DelayDataType singletonDelay;

DelayDataType * getDelayData()
{
    return &singletonDelay;
}


void initDelay(DelayDataType*data)
{
    for (uint32_t c=0;c<DELAY_LINE_LENGTH;c++)
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
float delayLineProcessSample(float sampleIn,DelayDataType*data)
{
    uint32_t delayIdx;
    float sampleOut;
    float sampleFedBack;
    delayIdx = (data->delayLinePtr - data->delayInSamples) & (DELAY_LINE_LENGTH -1);

    sampleOut = *(data->delayLine +delayIdx)*data->mix + sampleIn*(1.0f - data->mix);
    sampleFedBack = sampleOut;
    if (data->feedbackFunction != 0)
    {
        sampleFedBack = data->feedbackFunction(sampleFedBack,data->feebackData);
    }
    if (sampleFedBack > 8388607.0f)
    {
        sampleFedBack = 8388607.0f;
    }
    else if (sampleFedBack < -8388608.0f)
    {
        sampleFedBack = -8388608.0f;
    }
    *(data->delayLine + data->delayLinePtr) = sampleFedBack;
    data->delayLinePtr++;
    data->delayLinePtr &= (DELAY_LINE_LENGTH -1);
    return sampleOut;
}
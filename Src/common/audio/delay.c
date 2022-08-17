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
    delayIdx = (data->delayLinePtr - data->delayInSamples) & (DELAY_LINE_LENGTH -1);

    sampleOut = ((*(data->delayLine +delayIdx)*data->mix) >> 15) + ((sampleIn*(32767 - data->mix)) >> 15);
    sampleFedBack = sampleOut;
    if (data->feedbackFunction != 0)
    {
        sampleFedBack = (int32_t)data->feedbackFunction((int16_t)sampleFedBack,data->feebackData);
    }
    sampleFedBack = ((data->feedback*sampleFedBack) >> 15);
    if (sampleFedBack > 32767)
    {
        sampleFedBack = 32767;
    }
    else if (sampleFedBack < -32767)
    {
        sampleFedBack = -32767;
    }
    *(data->delayLine + data->delayLinePtr) = (int16_t)sampleFedBack;
    data->delayLinePtr++;
    data->delayLinePtr &= (DELAY_LINE_LENGTH -1);
    return sampleOut;
}
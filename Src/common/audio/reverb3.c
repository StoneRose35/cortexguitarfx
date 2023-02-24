#include "audio/reverb3.h"
#include "audio/reverbUtils.h"

const int16_t diffuserTimes[4]={293, 353, 419, 487};
void initReverb3(Reverb3Type*data)
{
    int16_t * delayMemoryPtr = getDelayMemoryPointer();
    uint8_t cnt=0;
    for(uint8_t c=0;c<4;c++)
    {
        for(uint8_t q=0;q < 4;q++)
        {
            data->diffusers[c].delayPointers[q] = delayMemoryPtr + (cnt++)*0x1FF;
            data->diffusers[c].delayTimes[q] = diffuserTimes[q];
        }
        data->diffusers[c].delayPointer=0;
    }
    data->delay.delayBufferLength = 0xFFF;
    data->delay.delayInSamples = 0xFFD;
    data->delay.delayLine = delayMemoryPtr + cnt*0x1FF;
    data->delay.feebackData = 0;
    data->delay.feedback = 1 << 14;
    data->delay.feedbackFunction = 0;
    data->delay.mix = 1 << 15;
    for (uint32_t c=0;c<((cnt + 0xFFF)>>1);c++)
    {
        *(((uint32_t*)delayMemoryPtr)+c)=0;
    }
}


int16_t reverb3processSample(int16_t sampleIn,Reverb3Type*data)
{
    int16_t sampleOut;
    int16_t diffuserChannels[4];
    diffuserChannels[0]=sampleIn;
    diffuserChannels[1]=sampleIn;
    diffuserChannels[2]=sampleIn;
    diffuserChannels[3]=sampleIn;
    hadamardDiffuserProcessArray(diffuserChannels,&data->diffusers[0]);
    hadamardDiffuserProcessArray(diffuserChannels,&data->diffusers[1]);
    hadamardDiffuserProcessArray(diffuserChannels,&data->diffusers[2]);
    hadamardDiffuserProcessArray(diffuserChannels,&data->diffusers[3]);
    sampleOut = diffuserChannels[0] + 
                diffuserChannels[1] +
                diffuserChannels[2] +
                diffuserChannels[3];
    sampleOut = delayLineProcessSample(sampleOut,&data->delay);
    return sampleOut;
}
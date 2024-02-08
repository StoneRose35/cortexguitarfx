
#include "audio/pitchshifter.h"
#include "audio/delay.h"
#include "audio/audiotools.h"
int16_t pitchShifterProcessSample(int16_t sampleIn,PitchshifterDataType*data)
{
    int16_t * delayMemoryPointer = getDelayMemoryPointer();
    int32_t sampleOut=0;
    int16_t deltaIndex;
    int16_t envelopeVal;
    volatile  uint32_t* audioStatePtr=getAudioStatePtr();
    deltaIndex = (data->currentDelayPosition - (data->delayLength1>>2)) &(data->buffersize-1);
    if (data->delayLength1 <(data->buffersize<<1))
    {
        envelopeVal = data->delayLength1;
    }
    else
    {
        envelopeVal = (data->buffersize<<2) - data->delayLength1;
    }
    sampleOut += (*(delayMemoryPointer + deltaIndex)*envelopeVal)>>(data->buffersizePowerTwo+1);

    deltaIndex = (data->currentDelayPosition - (data->delayLength2>>2))&(data->buffersize-1);
    if (data->delayLength2 <(data->buffersize<<1))
    {
        envelopeVal = data->delayLength2;
    }
    else
    {
        envelopeVal = (data->buffersize<<2) - data->delayLength2;
    }
    sampleOut += (*(delayMemoryPointer + deltaIndex)*envelopeVal)>>(data->buffersizePowerTwo+1);
    data->currentDelayPosition++;
    data->currentDelayPosition &= (data->buffersize-1);
    data->delayLength1 += data->delayIncrement;
    if ((data->delayLength1>>2) < 0)
    {
        data->delayLength1 = ((data->buffersize<<2) - 1);
    }
    else if ((data->delayLength1>>2) > (data->buffersize - 1))
    {
        data->delayLength1 = 0;
    }

    data->delayLength2 += data->delayIncrement;
    if ((data->delayLength2>>2) < 0)
    {
        data->delayLength2 = ((data->buffersize<<2) - 1);
    }
    else if ((data->delayLength2>>2) > (data->buffersize - 1))
    {
        data->delayLength2 = 0;
    }

    *(delayMemoryPointer + data->currentDelayPosition) = sampleIn;
    return clip(sampleOut,audioStatePtr);
}

void initPitchshifter(PitchshifterDataType*data)
{
    int16_t * delayMemoryPointer = getDelayMemoryPointer();
    data->buffersize = 1 << data->buffersizePowerTwo;
    for (uint16_t c=0;c<data->buffersize;c++)
    {
        *(delayMemoryPointer + c) = 0;
    }

    data->delayLength1 = 0; 
    data->delayLength2 = (data->buffersize<<2)/2;
}
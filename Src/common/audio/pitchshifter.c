
#include "audio/pitchshifter.h"
#include "audio/delay.h"
#include "audio/audiotools.h"
int16_t pitchShifterProcessSample(int16_t sampleIn,PitchshifterDataType*data)
{
    int16_t * delayMemoryPointer = getDelayMemoryPointer();
    int32_t sampleOut=0;
    int16_t deltaIndex;
    volatile  uint32_t* audioStatePtr=getAudioStatePtr();
    deltaIndex = (data->currentDelayPosition - data->delayIndex1) &((1<<PITSHIFTER_BUFFER_SIZE_TWOS_POWER)-1);
    sampleOut += (*(delayMemoryPointer + deltaIndex)*(PITCHSHIFTER_BUFFER_SIZE - deltaIndex))>>PITSHIFTER_BUFFER_SIZE_TWOS_POWER;
    deltaIndex = (data->currentDelayPosition - data->delayIndex2)&((1<<PITSHIFTER_BUFFER_SIZE_TWOS_POWER)-1);
    sampleOut += (*(delayMemoryPointer + data->delayIndex2)*(PITCHSHIFTER_BUFFER_SIZE - deltaIndex))>>PITSHIFTER_BUFFER_SIZE_TWOS_POWER;
    deltaIndex = (data->currentDelayPosition - data->delayIndex3)&((1<<PITSHIFTER_BUFFER_SIZE_TWOS_POWER)-1);
    sampleOut += (*(delayMemoryPointer + data->delayIndex3)*(PITCHSHIFTER_BUFFER_SIZE - deltaIndex))>>PITSHIFTER_BUFFER_SIZE_TWOS_POWER;

    data->currentDelayPosition++;
    data->currentDelayPosition &= ((1 <<PITSHIFTER_BUFFER_SIZE_TWOS_POWER)-1);
    data->delayIndex1 += data->delayIncrement >> 2;
    data->delayIndex1 &= ((1 <<PITSHIFTER_BUFFER_SIZE_TWOS_POWER)-1);
    deltaIndex = data->currentDelayPosition - data->delayIndex1;
    data->delayIndex2 += data->delayIncrement >> 2;
    data->delayIndex2 &= ((1 <<PITSHIFTER_BUFFER_SIZE_TWOS_POWER)-1);
    data->delayIndex3 += data->delayIncrement >> 2;
    data->delayIndex3 &= ((1 <<PITSHIFTER_BUFFER_SIZE_TWOS_POWER)-1);
    *(delayMemoryPointer + data->currentDelayPosition) = sampleIn;
    return clip(sampleOut,audioStatePtr);
}

void initPitchshifter(PitchshifterDataType*data)
{
    int16_t deltaIndex;
    int16_t * delayMemoryPointer = getDelayMemoryPointer();

    for (uint16_t c=0;c<PITCHSHIFTER_BUFFER_SIZE;c++)
    {
        *(delayMemoryPointer + c) = 0;
    }
    deltaIndex = data->currentDelayPosition - PITCHSHIFTER_BUFFER_SIZE/4;
    if (deltaIndex < 0)
    {
        deltaIndex += PITCHSHIFTER_BUFFER_SIZE;
    }
    data->delayIndex1 = deltaIndex;
    deltaIndex = data->currentDelayPosition - PITCHSHIFTER_BUFFER_SIZE/2;
    if (deltaIndex < 0)
    {
        deltaIndex += PITCHSHIFTER_BUFFER_SIZE;
    }
    data->delayIndex2 = deltaIndex;
    deltaIndex = data->currentDelayPosition - 3*PITCHSHIFTER_BUFFER_SIZE/4;
    if (deltaIndex < 0)
    {
        deltaIndex += PITCHSHIFTER_BUFFER_SIZE;
    }
    data->delayIndex3 = deltaIndex;
}
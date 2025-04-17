
#include "audio/pitchshifter.h"
#include "audio/delay.h"
#include "audio/audiotools.h"
#include "memoryRegions.h"

__ITCM_CODE
float pitchShifterProcessSample(float sampleIn,PitchshifterDataType*data)
{
    float sampleOut=0;
    int16_t deltaIndex;
    float envelopeVal;
    deltaIndex = (data->currentDelayPosition - (data->delayLength1>>2)) &(data->buffersize-1);
    if (data->delayLength1 <(data->buffersize<<1))
    {
        envelopeVal = (float)data->delayLength1/(float)((data->buffersize<<1)-1);
    }
    else
    {
        envelopeVal = ((data->buffersize<<2) - data->delayLength1)/(float)((data->buffersize<<1)-1);
    }
    sampleOut += (*(data->delayBufferPtr + deltaIndex)*envelopeVal);

    deltaIndex = (data->currentDelayPosition - (data->delayLength2>>2))&(data->buffersize-1);
    if (data->delayLength2 <(data->buffersize<<1))
    {
        envelopeVal = data->delayLength2/(float)((data->buffersize<<1)-1);
    }
    else
    {
        envelopeVal = ((data->buffersize<<2) - data->delayLength2)/(float)((data->buffersize<<1)-1);
    }
    sampleOut += (*(data->delayBufferPtr + deltaIndex)*envelopeVal);
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

    *(data->delayBufferPtr + data->currentDelayPosition) = sampleIn;
    return sampleOut;
}

__QSPI_CODE
void initPitchshifter(PitchshifterDataType*data,float * delayMemoryPointer)
{
    data->delayBufferPtr = delayMemoryPointer;
    data->buffersize = 1 << data->buffersizePowerTwo;
    for (uint16_t c=0;c<data->buffersize;c++)
    {
        *(delayMemoryPointer + c) = 0;
    }

    data->delayLength1 = 0; 
    data->delayLength2 = (data->buffersize<<2)/2;
}
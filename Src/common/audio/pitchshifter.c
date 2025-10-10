
#include "audio/pitchshifter.h"
#include "audio/delay.h"
#include "audio/audiotools.h"
#include "memoryRegions.h"
#include "pipicofx/delayMemoryHandler.h"

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

__ITCM_CODE
float pitchShifter2ProcessSample(float sampleIn,Pitchshifter2DataType * data)
{
    float sampleOut;
    int16_t deltaIndex;
    int16_t delayPointerTemp=0;

    if (data->delayIncrement > 4)
    {
        // compute relative index positions
        if (data->currentDelayPosition >= (data->delayPointer1 >> 2))
        {
            deltaIndex = (data->currentDelayPosition - (data->delayPointer1 >> 2));
        }
        else
        {
            deltaIndex = (data->currentDelayPosition - (data->delayPointer1 >> 2) + data->buffersize);
        }

        // compute output sample
        if (deltaIndex <= data->crossFadeWidth)
        {
            delayPointerTemp = (data->delayPointer1 + (data->crossFadeWidth << 2)) & ((data->buffersize << 2) - 1);
            sampleOut = ((deltaIndex*data->delayMemoryPtr[data->delayPointer1>>2])/(float)data->crossFadeWidth) +
            ((data->crossFadeWidth - deltaIndex)*data->delayMemoryPtr[delayPointerTemp>>2]/data->crossFadeWidth);
            // (deltaIndex*samples[delayPointer1>>2] >> crossFadeWidthPwr2) + ((crossFadeWidth - 1 - deltaIndex)*samples[delayPointer2>>2] >> crossFadeWidthPwr2)
        }
        else
        {
            sampleOut = data->delayMemoryPtr[data->delayPointer1 >> 2]; //((delayMemoryPointer[data->delayPointer1 >> 2]*(4-(data->delayPointer1&3)))>>2) + (delayMemoryPointer[(data->delayPointer1 >> 2) + 1]*(data->delayPointer1&3)) >> 2;
        }

        // compute new index values, swap index1 with index2 if index1 has surpassed currentDelayPosition
        uint8_t idx1BehindIdx2 = (data->delayPointer1>>2) > data->currentDelayPosition; // slower behind faster?
        int16_t currentDelayPosition1New = (data->currentDelayPosition + 1) & (data->buffersize - 1);
        int16_t delayPointer1New = (data->delayPointer1 + data->delayIncrement) & ((data->buffersize<<2) - 1);
        uint8_t idx1Jumped = currentDelayPosition1New < data->currentDelayPosition; // slower jumped?
        uint8_t idx2Jumped = delayPointer1New < data->delayPointer1; // faster jumped?
        uint8_t idx1BehindIdx2New = (delayPointer1New>>2) > currentDelayPosition1New; // slower behind faster after increment of both?

        if (((idx2Jumped) && (idx1BehindIdx2 == idx1BehindIdx2New)) || // faster index wrapped around and surpassed at the same time
            (!idx1BehindIdx2 && idx1BehindIdx2New && !idx1Jumped)) // slower index now is behind faster one, slower index didn't wrap around
        {
            // faster index surpassed slower one, jump to pointer 2
            data->delayPointer1 = delayPointerTemp;
        }

        *(data->delayMemoryPtr + data->currentDelayPosition) = sampleIn;

        // increment pointers
        data->currentDelayPosition++;
        data->currentDelayPosition &= (data->buffersize-1);
        data->delayPointer1 += data->delayIncrement;
        data->delayPointer1 &= ((data->buffersize<<2)-1);

        return sampleOut;
    }
    else
    {
        // compute relative index positions
        if ((data->delayPointer1 >> 2) >=data->currentDelayPosition)
        {
            deltaIndex = ((data->delayPointer1 >> 2)-data->currentDelayPosition);
        }
        else
        {
            deltaIndex = ((data->delayPointer1 >> 2)-data->currentDelayPosition + data->buffersize);
        }

        // compute output sample
        if (deltaIndex <= data->crossFadeWidth)
        {
            delayPointerTemp = (data->delayPointer1 - (data->crossFadeWidth << 2)) & ((data->buffersize << 2) - 1);
            sampleOut = (deltaIndex*data->delayMemoryPtr[data->delayPointer1>>2])/(float)data->crossFadeWidth +
            ((data->crossFadeWidth - deltaIndex)*data->delayMemoryPtr[delayPointerTemp>>2]/(float)data->crossFadeWidth);
            // (deltaIndex*samples[delayPointer1>>2] >> crossFadeWidthPwr2) + ((crossFadeWidth - 1 - deltaIndex)*samples[delayPointer2>>2] >> crossFadeWidthPwr2)
        }
        else
        {
            sampleOut = data->delayMemoryPtr[data->delayPointer1 >> 2];
        }


        // compute new index values, swap index1 with index2 if index1 has surpassed currentDelayPosition
        uint8_t idx1BehindIdx2 = data->currentDelayPosition > (data->delayPointer1>>2);  // slower behind faster?
        int16_t currentDelayPosition1New = (data->currentDelayPosition + 1) & (data->buffersize - 1);
        int16_t delayPointer1New = (data->delayPointer1 + data->delayIncrement) & ((data->buffersize<<2) - 1);
        uint8_t idx1Jumped = delayPointer1New < data->delayPointer1; // slower jumped?
        uint8_t idx2Jumped = currentDelayPosition1New < data->currentDelayPosition; // faster jumped?
        uint8_t idx1BehindIdx2New = currentDelayPosition1New > (delayPointer1New>>2); // slower behind faster after increment of both?

        if (((idx2Jumped) && (idx1BehindIdx2 == idx1BehindIdx2New)) || // faster index wrapped around and surpassed at the same time
            (!idx1BehindIdx2 && idx1BehindIdx2New && !idx1Jumped)) // slower index now is behind faster one, slower index didn't wrap around
        {
            // faster index surpassed slower one, jump to pointer 2
            data->delayPointer1 = delayPointerTemp;
        }

        *(data->delayMemoryPtr + data->currentDelayPosition) = sampleIn;

        // increment pointers
        data->currentDelayPosition++;
        data->currentDelayPosition &= (data->buffersize-1);
        data->delayPointer1 += data->delayIncrement;
        data->delayPointer1 &= ((data->buffersize<<2)-1);

        return sampleOut;
    }
}

__QSPI_CODE
void initPitchshifter(PitchshifterDataType*data)
{
    data->buffersize = 1 << data->buffersizePowerTwo;
    data->delayBufferPtr = mallocDelayMemory(data->buffersize<<2);
    for (uint16_t c=0;c<data->buffersize;c++)
    {
        *(data->delayBufferPtr + c) = 0.0f;
    }

    data->delayLength1 = 0; 
    data->delayLength2 = (data->buffersize<<2)/2;
}

__QSPI_CODE
void initPitchshifter2(Pitchshifter2DataType*data)
{
    data->buffersize = 1 << data->buffersizePowerTwo;
    data->delayMemoryPtr = mallocDelayMemory(data->buffersize<<2);
    for (uint16_t c=0;c<data->buffersize;c++)
    {
        *(data->delayMemoryPtr + c) = 0;
    }
    data->delayPointer1 = 0;
    data->crossFadeWidth = 1 << data->crossFadeWidthPwr2;
}

__QSPI_CODE
void deinitPitchshifter(PitchshifterDataType*data)
{
    freeDelayMemory(data->delayBufferPtr);
}

__QSPI_CODE
void deinitPitchshifter2(Pitchshifter2DataType*data)
{
    freeDelayMemory(data->delayMemoryPtr);
}

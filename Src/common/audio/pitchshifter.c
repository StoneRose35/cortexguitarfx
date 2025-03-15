
#include "audio/pitchshifter.h"
#include "audio/delay.h"
#include "audio/audiotools.h"
int16_t pitchShifterProcessSample(int16_t sampleIn,PitchshifterDataType*data,volatile uint32_t*audioStatePtr)
{
    int16_t * delayMemoryPointer = getDelayMemoryPointer();
    int32_t sampleOut=0;
    int16_t deltaIndex;
    int16_t envelopeVal;

    if (data->currentDelayPosition >= (data->delayLength1 >> 2))
    {
        deltaIndex = (data->currentDelayPosition - (data->delayLength1 >> 2));
    }
    else
    {
        deltaIndex = (data->buffersize - data->currentDelayPosition + (data->delayLength1 >> 2));
    }



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

int16_t pitchShifter2ProcessSample(int16_t sampleIn,Pitchshifter2DataType * data,volatile uint32_t * audioStatePtr)
{
    int16_t * delayMemoryPointer = getDelayMemoryPointer();
    int16_t sampleOut;
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
            sampleOut = clip((deltaIndex*delayMemoryPointer[data->delayPointer1>>2] >> data->crossFadeWidthPwr2) +
            ((data->crossFadeWidth - deltaIndex)*delayMemoryPointer[delayPointerTemp>>2] >> data->crossFadeWidthPwr2),audioStatePtr);
            // (deltaIndex*samples[delayPointer1>>2] >> crossFadeWidthPwr2) + ((crossFadeWidth - 1 - deltaIndex)*samples[delayPointer2>>2] >> crossFadeWidthPwr2)
        }
        else
        {
            sampleOut = delayMemoryPointer[data->delayPointer1 >> 2]; //((delayMemoryPointer[data->delayPointer1 >> 2]*(4-(data->delayPointer1&3)))>>2) + (delayMemoryPointer[(data->delayPointer1 >> 2) + 1]*(data->delayPointer1&3)) >> 2;
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

        *(delayMemoryPointer + data->currentDelayPosition) = sampleIn;

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
            sampleOut = clip(((deltaIndex*delayMemoryPointer[data->delayPointer1>>2]) >> data->crossFadeWidthPwr2) +
            ((data->crossFadeWidth - deltaIndex)*delayMemoryPointer[delayPointerTemp>>2] >> data->crossFadeWidthPwr2),audioStatePtr);
            // (deltaIndex*samples[delayPointer1>>2] >> crossFadeWidthPwr2) + ((crossFadeWidth - 1 - deltaIndex)*samples[delayPointer2>>2] >> crossFadeWidthPwr2)
        }
        else
        {
            sampleOut = delayMemoryPointer[data->delayPointer1 >> 2];
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

        *(delayMemoryPointer + data->currentDelayPosition) = sampleIn;

        // increment pointers
        data->currentDelayPosition++;
        data->currentDelayPosition &= (data->buffersize-1);
        data->delayPointer1 += data->delayIncrement;
        data->delayPointer1 &= ((data->buffersize<<2)-1);

        return sampleOut;
    }
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

void initPitchshifter2(Pitchshifter2DataType*data)
{
    int16_t * delayMemoryPointer = getDelayMemoryPointer();
    data->buffersize = 1 << data->buffersizePowerTwo;
    for (uint16_t c=0;c<data->buffersize;c++)
    {
        *(delayMemoryPointer + c) = 0;
    }
    data->delayPointer1 = 0;
    data->crossFadeWidth = 1 << data->crossFadeWidthPwr2;
}
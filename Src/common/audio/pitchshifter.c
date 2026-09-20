
#include "audio/pitchshifter.h"
#include "audio/delay.h"
#include "audio/audiotools.h"
#include "memoryRegions.h"
#include "pipicofx/delayMemoryHandler.h"

#ifdef PS3_DBG_PRINT
#include "stdio.h"
#endif

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

__ITCM_CODE
float ps3SummedDifferenceAbs(float * data,uint32_t idxa, uint32_t idxb)
{
	float res=0.0f;
    float diff;
	for (uint16_t c = 0;c<PS3_ANALYSIS_WINDOW_SIZE;c++)
	{
        #ifdef PS3_DBG_PRINT
        //printf("val a: %f, val b: %f, summed diff: %f\r\n",*(data+idxa),*(data+idxb),res);
        #endif
        diff = *(data+idxa) - *(data+idxb);
        if (diff > 0.0f)
        {
            res += diff;
        }
        else
        {
            res -= diff;
        }
		idxa--;
		idxa &= (PS3_MAX_BUFFER_SIZE-1);
		idxb--;
		idxb &= (PS3_MAX_BUFFER_SIZE-1);
	}
	return res;
}

__ITCM_CODE
uint32_t ps3DistanceFromWritePointer(Pitchshifter3DataType * cb)
{
	if (cb->pointerIncrement < (1 << PS3_READ_POINTER_FRACT)) // slower than the write pointer
	{	
		return ((cb->writePointer - (cb->readPointer>>PS3_READ_POINTER_FRACT)) & (PS3_MAX_BUFFER_SIZE - 1));
	}
	else
	{
		return (((cb->readPointer>>PS3_READ_POINTER_FRACT) - cb->writePointer) & (PS3_MAX_BUFFER_SIZE - 1)); 
	}
}

__ITCM_CODE
uint16_t ps3PushZeroCrossing(uint32_t zcIdx,uint16_t minDist, Pitchshifter3DataType*cb)
{
    uint32_t oldVal = cb->zeroCrossings[cb->zeroCrossingPtr];
    if (oldVal == 0xFFFFFFFF || zcIdx - oldVal > minDist)
    {
        cb->zeroCrossingPtr++;
    	cb->zeroCrossingPtr &= (PS3_ZERO_CROSSINGS_SIZE - 1);
    	cb->zeroCrossings[cb->zeroCrossingPtr] = zcIdx;
        return 1;
    }
    return 0;
}

__ITCM_CODE
void ps3IncrementPointer(Pitchshifter3DataType*cb)
{
	cb->writePointer++;
	cb->writePointer &= (PS3_MAX_BUFFER_SIZE - 1);
	cb->readPointer += cb->pointerIncrement;
	cb->readPointer &= ((PS3_MAX_BUFFER_SIZE << PS3_READ_POINTER_FRACT) - 1);
}

__ITCM_CODE
float pitchShifter3ProcessSample(float newSample,Pitchshifter3DataType*cb)
{
	uint8_t zeroCrossed = 0;

    uint8_t jumpCheckDone = 0;
    uint32_t candidatesFound = 0;
    float summedDifferences[PS3_ZERO_CROSSINGS_SIZE];
    uint16_t zcp;
    uint16_t oldDistance = 0xFFFF;
    uint16_t distance;
    uint32_t minIdx=0;
    float currentMin = 99999999.9f;
    uint32_t readPtrNext;
    uint32_t currentDistance;
    uint32_t zeroCrossingDataIndex;
    float res;
    //uint8_t exception=0; 
    float newSampleLowpassed;
	float currentSample = cb->delayMemoryPtr[(cb->writePointer-1) & (PS3_MAX_BUFFER_SIZE-1)];
	if ((currentSample > 0.0f && newSampleLowpassed < 0.0f) || (currentSample < 0.0f && newSampleLowpassed > 0.0f))
	{
		cb->zeroCrossingsAddedSinceLastJump += ps3PushZeroCrossing(cb->writePointer,ZERO_PUSH_MIN_DIST,cb);
	}
	readPtrNext = (((cb->readPointer + cb->pointerIncrement) >> PS3_READ_POINTER_FRACT) & (PS3_MAX_BUFFER_SIZE -1));
	if ((cb->delayMemoryPtr[cb->readPointer>>PS3_READ_POINTER_FRACT] > 0.0f && cb->delayMemoryPtr[readPtrNext] < 0.0f) || (cb->delayMemoryPtr[cb->readPointer>>PS3_READ_POINTER_FRACT] < 0.0f && cb->delayMemoryPtr[readPtrNext] > 0.0f))
	{
		zeroCrossed = 1;
	}
    cb->delayMemoryPtr[cb->writePointer] = newSample;

	currentDistance = ps3DistanceFromWritePointer(cb);
	if (cb->pointerIncrement < (1 << PS3_READ_POINTER_FRACT) && currentDistance > PS3_DISTANCE_THRESHOLD && zeroCrossed) // read pointer moving slower and away from the write pointer, zero cross on read anticipated
	{
        zcp = cb->zeroCrossingPtr;
		// check the newest zero crossings written into the buffer for the best candidate according to the summed difference and move the pointer to the best value
        candidatesFound = 0;
        #ifdef PS3_DBG_PRINT
        printf("----------------check pointer jump ----------------\r\n");
        #endif
		while (jumpCheckDone == 0)
		{
			zeroCrossingDataIndex = cb->zeroCrossings[zcp];
            distance =  ((zeroCrossingDataIndex - (cb->readPointer >> PS3_READ_POINTER_FRACT)) & (PS3_MAX_BUFFER_SIZE - 1)) ;
			if (oldDistance > distance && cb->zeroCrossings[zcp] != 0xFFFFFFFF && candidatesFound < (PS3_MIN_ZC_CANDIDATES + 4) && distance > PS3_ANALYSIS_WINDOW_SIZE)
			{
                #ifdef PS3_DBG_PRINT
                printf("computing summed differences for zero crossing at %u\r\n",zeroCrossingDataIndex);
                #endif
				summedDifferences[candidatesFound++] = ps3SummedDifferenceAbs(cb->delayMemoryPtr,zeroCrossingDataIndex,cb->readPointer>>PS3_READ_POINTER_FRACT);
				zcp--;
				zcp &= (PS3_ZERO_CROSSINGS_SIZE -1);
                #ifdef PS3_DBG_PRINT
                printf("summed difference value %f\r\n",summedDifferences[candidatesFound-1]);
                #endif
                oldDistance = distance;
			}
			else
			{
				jumpCheckDone = 1;
			}
		}
        
		if (candidatesFound >= PS3_MIN_ZC_CANDIDATES)
		{
			currentMin = 99999999.9f;
			
            cb->zeroCrossingsAddedSinceLastJump = 0;
			for (uint8_t c=0;c < candidatesFound;c++)
			{
				if (summedDifferences[c] < currentMin)
				{
					minIdx = cb->zeroCrossings[(cb->zeroCrossingPtr - c) & (PS3_ZERO_CROSSINGS_SIZE -1)];
					currentMin = summedDifferences[c];
				}
			}
            #ifdef PS3_DBG_PRINT
			printf("pointer jump from %d to %d at sample # %d\r\n",cb->readPointer,minIdx << PS3_READ_POINTER_FRACT,cb->sampleCnt);
            #endif
			cb->readPointer = (minIdx << PS3_READ_POINTER_FRACT); 
		}

	}
	else if (zeroCrossed) // read pointer moving faster and getting out of sight of the write pointer, zero crossing on read anticipated
	{        
        candidatesFound = 0;
        // count zero crossing ahead of the read counter
        zcp = cb->zeroCrossingPtr;
        zeroCrossingDataIndex = cb->zeroCrossings[zcp];
        distance =  ((zeroCrossingDataIndex - (cb->readPointer >> PS3_READ_POINTER_FRACT)) & (PS3_MAX_BUFFER_SIZE - 1)) ;
        while (oldDistance > distance && zeroCrossingDataIndex != 0xFFFFFFFF && candidatesFound < 4)
        {
            oldDistance = distance;
            candidatesFound++; // semantically wrong, here zeros crossing before the read pointer are counted
            zcp--;
            zcp &= (PS3_ZERO_CROSSINGS_SIZE -1);
            zeroCrossingDataIndex = cb->zeroCrossings[zcp];
            distance =  ((zeroCrossingDataIndex - (cb->readPointer >> PS3_READ_POINTER_FRACT)) & (PS3_MAX_BUFFER_SIZE - 1)) ;
        }

        if (candidatesFound < 3)
        {
            candidatesFound = 0;
            uint32_t zcp_beyond_readptr = zcp;
            // only a few possibilities left to jump backwards, search the best option for the next zeros crossing past the read pointer
            while (candidatesFound < 4 && zeroCrossingDataIndex != 0xFFFFFFFF)
            {
                distance =  (((cb->readPointer >> PS3_READ_POINTER_FRACT) - zeroCrossingDataIndex) & (PS3_MAX_BUFFER_SIZE - 1)) ;
                if (distance > PS3_DISTANCE_THRESHOLD)
                {
                    if (candidatesFound == 0)
                    {
                        zcp_beyond_readptr = zcp;
                    }
                    summedDifferences[candidatesFound++] = ps3SummedDifferenceAbs(cb->delayMemoryPtr,zeroCrossingDataIndex,cb->readPointer>>PS3_READ_POINTER_FRACT);
                }
                zcp--;
                zcp &= (PS3_ZERO_CROSSINGS_SIZE -1);
                zeroCrossingDataIndex = cb->zeroCrossings[zcp];

            }
            currentMin = 99999999.9f;
			
			for (uint8_t c=0;c < candidatesFound;c++)
			{
				if (summedDifferences[c] < currentMin)
				{
					minIdx = cb->zeroCrossings[(zcp_beyond_readptr - c) & (PS3_ZERO_CROSSINGS_SIZE -1)];
					currentMin = summedDifferences[c];
				}
			}

            cb->readPointer = minIdx << PS3_READ_POINTER_FRACT;
        }
    }
    
	res = cb->delayMemoryPtr[cb->readPointer >> PS3_READ_POINTER_FRACT] + ((cb->delayMemoryPtr[((cb->readPointer >> PS3_READ_POINTER_FRACT) + 1) & (PS3_MAX_BUFFER_SIZE - 1)] - cb->delayMemoryPtr[cb->readPointer >> PS3_READ_POINTER_FRACT])*((float)(cb->readPointer & ((1 << PS3_READ_POINTER_FRACT)-1))))/((float)(1 << PS3_READ_POINTER_FRACT));
	ps3IncrementPointer(cb);
    cb->sampleCnt++;
    return res;
}

__QSPI_CODE
void iniPitchShifter3(Pitchshifter3DataType*data)
{


    data->writePointer = 1;
    data->readPointer = ((data->writePointer - PS3_DISTANCE_THRESHOLD) & (PS3_MAX_BUFFER_SIZE-1)) << PS3_READ_POINTER_FRACT;
    data->pointerIncrement = 6;
    data->zeroCrossingPtr = PS3_ZERO_CROSSINGS_SIZE -1 ;
    data->delayMemoryPtr = mallocDelayMemory(PS3_MAX_BUFFER_SIZE<<2);
    data->lpFilter.alpha = 0.05f;
    data->lpFilter.oldVal = 0.0f;
    data->lpFilter.oldXVal = 0.0f;
    data->zeroCrossingsAddedSinceLastJump = 0;
    data->sampleCnt = 0;
	for (uint32_t c=0;c<PS3_MAX_BUFFER_SIZE;c++)
	{
		data->delayMemoryPtr[c]=0.0f;
	}
	for (uint32_t c=0;c<PS3_ZERO_CROSSINGS_SIZE;c++)
	{
		data->zeroCrossings[c]=0xFFFFFFFF;
	}
}


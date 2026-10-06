
#include "audio/pitchshifter.h"
#include "audio/delay.h"
#include "audio/audiotools.h"
#include "memoryRegions.h"
#include "pipicofx/delayMemoryHandler.h"
#include "stdlib.h"
#ifdef PS3_DBG_PRINT
#include "stdio.h"
FILE * fid;
#endif

__ITCM_CODE
float pitchShifterProcessSample(float sampleIn,GrainsDataType*data)
{
    float sampleOut=0.0f;
    for (uint8_t c=0;c<data->readPointerCount;c++)
    {
        sampleOut += (data->delayBufferPtr[data->readPointers[c] >> 2] + 
            ((data->delayBufferPtr[((data->readPointers[c] >> 2) + 1) & (data->bufferSize - 1)] - data->delayBufferPtr[data->readPointers[c] >> 2])
            *((float)(data->readPointers[c] & (4-1))))/(4.0f))*grainEnvelopeValue(data,c)/((float)data->readPointerCount); 
    }
    *(data->delayBufferPtr + data->writePointer) = sampleIn;
    updatePointers(data);
    return sampleOut;
}

__ITCM_CODE
uint16_t distanceFromGrainEdge(GrainsDataType*data,uint8_t readPointerNr)
{
    return (data->writePointer - (data->readPointers[readPointerNr] >> 2)) & (data->bufferSize - 1);
}

__ITCM_CODE
float grainEnvelopeValue(GrainsDataType*data,uint8_t readPointerNr)
{
    uint16_t currentDistance = distanceFromGrainEdge(data,readPointerNr);
    float relDistance = ((float)currentDistance)/((float)data->grainSize);
    if (relDistance < 0.5f)
    {
        return 2.0f*relDistance;
    }
    else
    {
        return 2.0f - 2.0f*relDistance;
    }
}

__ITCM_CODE
void updatePointers(GrainsDataType* data)
{
    uint16_t distanceOld;
    data->writePointer++;
    data->writePointer &= (data->bufferSize-1);
    for (uint8_t c=0;c<data->readPointerCount;c++)
    {
        distanceOld = distanceFromGrainEdge(data,c);
        data->readPointers[c] += data->pointerIncrement;
        data->readPointers[c] &= ((data->bufferSize << 2)-1);
        if (data->pointerIncrement < 4 && distanceFromGrainEdge(data,c) > data->grainSize)
        {
            data->readPointers[c] += (data->grainSize << 2);
            data->readPointers[c] &= ((data->bufferSize<<2)-1);
        }
        else if (data -> pointerIncrement > 4 && distanceFromGrainEdge(data,c) > data->grainSize)
        {
            data->readPointers[c] -= (data->grainSize << 2);
            data->readPointers[c] &= ((data->bufferSize<<2)-1);
        }
    }
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
void initPitchshifter(GrainsDataType*data)
{
    data->bufferSize=8192;
    //data->readPointerCount = 3;
    //data->grainSize = 192;
    data->writePointer = 0;
    data->delayBufferPtr = mallocDelayMemory(data->bufferSize<<2);
    data->readPointers = malloc(data->readPointerCount*sizeof(uint16_t));
    for (uint8_t c=0;c<data->readPointerCount;c++)
    {
        data->readPointers[c] = (-((c*(data->grainSize/data->readPointerCount)) << 2)) & ((data->bufferSize << 2)-1);
    }
    for (uint16_t c=0;c<data->bufferSize;c++)
    {
        *(data->delayBufferPtr + c) = 0.0f;
    }

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
void deinitPitchshifter(GrainsDataType*data)
{
    freeDelayMemory(data->delayBufferPtr);
    free(data->readPointers);
}

__QSPI_CODE
void deinitPitchshifter2(Pitchshifter2DataType*data)
{
    freeDelayMemory(data->delayMemoryPtr);
}

__ITCM_CODE
float ps3SummedDifferenceAbs(float * data,uint32_t idxa, uint32_t idxb,uint16_t windowSize)
{
	float res=0.0f;
    float diff;
	for (uint16_t c = 0;c<windowSize;c++)
	{
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
    float newSampleLowpassed=newSample;
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
    // SHIFT DOWN
	if (cb->pointerIncrement < (1 << PS3_READ_POINTER_FRACT) && currentDistance > PS3_DISTANCE_THRESHOLD && zeroCrossed) // read pointer moving slower and away from the write pointer, zero cross on read anticipated
	{
        zcp = cb->zeroCrossingPtr;
		// check the newest zero crossings written into the buffer for the best candidate according to the summed difference and move the pointer to the best value
        candidatesFound = 0;

		while (jumpCheckDone == 0)
		{
			zeroCrossingDataIndex = cb->zeroCrossings[zcp];
            distance =  ((zeroCrossingDataIndex - (cb->readPointer >> PS3_READ_POINTER_FRACT)) & (PS3_MAX_BUFFER_SIZE - 1)) ;
			if (oldDistance > distance && cb->zeroCrossings[zcp] != 0xFFFFFFFF && candidatesFound < (PS3_MIN_ZC_CANDIDATES + 1) && distance > PS3_ANALYSIS_WINDOW_SIZE_DOWNSHIFT)
			{
				summedDifferences[candidatesFound++] = ps3SummedDifferenceAbs(cb->delayMemoryPtr,zeroCrossingDataIndex,cb->readPointer>>PS3_READ_POINTER_FRACT,PS3_ANALYSIS_WINDOW_SIZE_DOWNSHIFT);
				zcp--;
				zcp &= (PS3_ZERO_CROSSINGS_SIZE -1);
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
			for (uint16_t c=0;c < candidatesFound;c++)
			{
				if (summedDifferences[c] < currentMin)
				{
					minIdx = cb->zeroCrossings[(cb->zeroCrossingPtr - c) & (PS3_ZERO_CROSSINGS_SIZE -1)];
					currentMin = summedDifferences[c];
				}
			}
            #ifdef PS3_DBG_PRINT
            fprintf(fid,"%u, %u, %u, %u\r\n",cb->sampleCnt,cb->readPointer>>PS3_READ_POINTER_FRACT,minIdx,(minIdx-(cb->readPointer>>PS3_READ_POINTER_FRACT)) & (PS3_MAX_BUFFER_SIZE - 1) );
            #endif
			cb->readPointer = (minIdx << PS3_READ_POINTER_FRACT); 
		}

	}
    // SHIFT UP
	else if ((cb->pointerIncrement > (1 << PS3_READ_POINTER_FRACT)) && zeroCrossed) // read pointer moving faster and getting out of sight of the write pointer, zero crossing on read anticipated
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

        if (candidatesFound < PS3_ZERO_CROSSINGS_AHEAD)
        {
            candidatesFound = 0;
            uint32_t zcp_beyond_readptr = zcp;
            // only a few possibilities left to jump backwards, search the best option for the next zeros crossing past the read pointer
            while (candidatesFound < PS3_ZERO_CROSSINGS_AHEAD+1 && zeroCrossingDataIndex != 0xFFFFFFFF)
            {
                distance =  (((cb->readPointer >> PS3_READ_POINTER_FRACT) - zeroCrossingDataIndex) & (PS3_MAX_BUFFER_SIZE - 1)) ;
                if (distance > PS3_DISTANCE_THRESHOLD)
                {
                    if (candidatesFound == 0)
                    {
                        zcp_beyond_readptr = zcp;
                    }
                    summedDifferences[candidatesFound++] = ps3SummedDifferenceAbs(cb->delayMemoryPtr,zeroCrossingDataIndex,cb->readPointer>>PS3_READ_POINTER_FRACT,PS3_ANALYSIS_WINDOW_SIZE_UPSHIFT);
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
            #ifdef PS3_DBG_PRINT
            fprintf(fid,"%u, %u, %u, %u\r\n",cb->sampleCnt,cb->readPointer>>PS3_READ_POINTER_FRACT,minIdx,((cb->readPointer>>PS3_READ_POINTER_FRACT)-minIdx) & (PS3_MAX_BUFFER_SIZE - 1) );
            #endif
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
    #ifdef PS3_DBG_PRINT
    fid =fopen("ps3.csv","wt");
    fprintf(fid,"'sampleCnt','jump from', 'jump to', 'distance'\r\n");
    #endif
}

__QSPI_CODE
void deinitPitchShifter3(Pitchshifter3DataType*data)
{
    (void*)data;
    #ifdef PS3_DBG_PRINT
    fclose(fid);
    #endif
}
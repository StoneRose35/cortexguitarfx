#include <iostream>
#include "stdint.h"
#include "math.h"
#include "stdio.h"

using namespace std;
#define ZERO_CROSSINGS_SIZE 256
#define MAX_BUFFER_SIZE 16384
#define DISTANCE_THRESHOLD 512
#define ANALYSIS_WINDOW_SIZE 512
#define READ_POINTER_FRACT 2 // how many positions the dot is away from the right in in fixed point notation of the read pointer, 2 means quarters, i.e. 1 is 1/4, 2 is 2/4 3 is 3/3 4 is 1

typedef struct {
	uint32_t readPointer;
	uint32_t writePointer;
	float data[MAX_BUFFER_SIZE];
	uint16_t pointerIncrement; 
	uint32_t zeroCrossings[ZERO_CROSSINGS_SIZE];
	uint16_t zeroCrossingPtr;
} CircularBufferType;

float summedDifferenceAbs(float * data,uint32_t idxa, uint32_t idxb)
{
	float res=0.0;
	for (uint16_t c = 0;c<ANALYSIS_WINDOW_SIZE;c++)
	{
		res += abs(*(data+idxa) - *(data+idxb));
		idxa--;
		idxa &= (MAX_BUFFER_SIZE-1);
		idxb--;
		idxb &= (MAX_BUFFER_SIZE-1);
	}
	return res;
}

uint32_t distanceFromWritePointer(CircularBufferType * cb)
{
	if (cb->pointerIncrement < (1 << READ_POINTER_FRACT)) // slower than the write pointer
	{	
		return ((cb->writePointer - (cb->readPointer>>READ_POINTER_FRACT)) & (MAX_BUFFER_SIZE - 1));
	}
	else
	{
		return (((cb->readPointer>>2) - cb->writePointer) & (MAX_BUFFER_SIZE - 1)); 
	}
}

void pushZeroCrossing(uint32_t zcIdx, CircularBufferType*cb)
{
	cb->zeroCrossingPtr++;
	cb->zeroCrossingPtr &= (ZERO_CROSSINGS_SIZE - 1);
	cb->zeroCrossings[cb->zeroCrossingPtr] = zcIdx;
}

void incrementPointer(CircularBufferType*cb)
{
	cb->writePointer++;
	cb->writePointer &= (MAX_BUFFER_SIZE - 1);
	cb->readPointer += cb->pointerIncrement;
	cb->readPointer &= ((MAX_BUFFER_SIZE << 2) - 1);
}

void process(float newSample,CircularBufferType*cb)
{
	uint8_t zeroCrossed = 0;
	float currentSample = cb->data[(cb->writePointer-1) & (MAX_BUFFER_SIZE-1)];
	if ((currentSample > 0.0f && newSample < 0.0f) || (currentSample < 0.0f && newSample > 0.0f))
	{
		pushZeroCrossing(cb->writePointer,cb);
	}
	uint32_t readPtrOld = (((cb->readPointer + cb->pointerIncrement) >> READ_POINTER_FRACT) & (MAX_BUFFER_SIZE -1));
	if ((cb->data[cb->readPointer>>READ_POINTER_FRACT] > 0.0f && cb->data[readPtrOld] < 0.0f) || (cb->data[cb->readPointer>>READ_POINTER_FRACT] < 0.0f && cb->data[readPtrOld] > 0.0f))
	{
		zeroCrossed = 1;
	}
	//incrementPointer(cb);
	cb->data[cb->writePointer] = newSample;

	uint32_t currentDistance = distanceFromWritePointer(cb);
	if (cb->pointerIncrement < (1 << READ_POINTER_FRACT) && currentDistance > ANALYSIS_WINDOW_SIZE && zeroCrossed) // read pointer moving slower and away from the write pointer, zero cross on read anticipated
	{
		// check the newest zero crossings written into the buffer for the best candidate according to the summed difference and move the pointer to the best value
		uint8_t jumpCheckDone = 0;
		uint8_t candidatesFound = 0;
		float summedDifferences[256];
		uint16_t zcp = cb->zeroCrossingPtr;
		while (jumpCheckDone == 0)
		{
			uint32_t zeroCrossingDataIndex = cb->zeroCrossings[zcp];
			if ((zeroCrossingDataIndex - (cb->readPointer >> 2)) & (MAX_BUFFER_SIZE - 1) > ANALYSIS_WINDOW_SIZE && cb->zeroCrossings[zcp] != 0xFFFFFFFF)
			{
				summedDifferences[candidatesFound++] = summedDifferenceAbs(cb->data,zeroCrossingDataIndex,cb->readPointer>>2);
				zcp--;
				zcp &= (ZERO_CROSSINGS_SIZE -1);
			}
			else
			{
				jumpCheckDone = 1;
			}
		}

		if (candidatesFound > 1)
		{
			uint32_t minIdx=0;
			float currentMin = 99999999.9f;
			for (uint8_t c=0;c < candidatesFound;c++)
			{
				if (summedDifferences[c] < currentMin)
				{
					minIdx = cb->zeroCrossings[cb->zeroCrossingPtr - c];
					currentMin = summedDifferences[c];
				}
			}
			printf("pointer jump from %d to %d\r\n",cb->readPointer,minIdx << READ_POINTER_FRACT);
			cb->readPointer = minIdx << READ_POINTER_FRACT;
		}

	}
	else if (currentDistance < DISTANCE_THRESHOLD && zeroCrossed) // read pointer moving faster and getting out of sight of the write pointer, zero crossing on read anticipated
	{
		// check the older zero crossings written into the buffer for the best candidate according to the summed difference and move the pointer to the best value
	}

	if ((cb->readPointer >> 2) == cb->writePointer)
	{
		printf("cursors crossing@%d\r\n",cb->writePointer);
	}
	printf("out %f\r\n",cb->data[cb->readPointer >> 2] + ((cb->data[(cb->readPointer >> 2) + 1] - cb->data[cb->readPointer >> 2])*((float)(cb->readPointer & ((1 << READ_POINTER_FRACT)-1))))/((float)(1 << READ_POINTER_FRACT)));
	incrementPointer(cb);
}

float stoopidMethod(float a,float mix)
{
	return a*mix + (1.0f-mix)*0.0f;
}

/*
calculated the summed difference (absolute) from array "data" and "testData"
starting at index "idx" in "data" and 0 in "testData" over the length "windowSize"
*/




int main(int argc,char ** argv)
{
	float pinc = 432.0f*6.28f/48000.0f;
	float p;
	float sample;
	CircularBufferType cb={
		.readPointer = 0,
		.writePointer = 1,
		.data = {},
		.pointerIncrement = 1,
		.zeroCrossings= {},
		.zeroCrossingPtr = ZERO_CROSSINGS_SIZE-1
	};

	for (uint32_t c=0;c<MAX_BUFFER_SIZE;c++)
	{
		cb.data[c]=0.0f;
	}
	for (uint32_t c=0;c<ZERO_CROSSINGS_SIZE;c++)
	{
		cb.zeroCrossings[c]=0xFFFFFFFF;
	}
	for (uint32_t t=0;t<48000;t++)
	{
		sample = sinf(p);
		p += pinc;
		process(sample,&cb);
	}
	return 0;
}
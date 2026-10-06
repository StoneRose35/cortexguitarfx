#ifndef _PITCHSHIFTER_H_
#define _PITCHSHIFTER_H_

#include "stdint.h"
#include "firstOrderIirFilter.h"

#define PS3_ZERO_CROSSINGS_SIZE 256 // array size to store the zero crossings
#define PS3_MAX_BUFFER_SIZE 8192 // the delay buffer size
#define PS3_DISTANCE_THRESHOLD 256 // the distance threshhold below which repositioning the pointer is considered
#define PS3_ANALYSIS_WINDOW_SIZE_UPSHIFT 128 // size of the window for summed difference when shifting up
#define PS3_ANALYSIS_WINDOW_SIZE_DOWNSHIFT 128 // size of the window for summed difference when shifting down
#define PS3_READ_POINTER_FRACT 2 // how many positions the dot is away from the right in in fixed point notation of the read pointer, 2 means quarters, i.e. 1 is 1/4, 2 is 2/4 3 is 3/3 4 is 1
#define PS3_MIN_ZC_CANDIDATES 5 // minimum zero crossing candidates to be considered
#define PS3_ZERO_CROSSINGS_AHEAD 4 // the number of zero crossing ahead that must existing before backtracking the pointer when shifting up
#define ZERO_PUSH_MIN_DIST 2 // the minimum distance in samples between two zero crossings

typedef struct 
{
    float * delayBufferPtr;
    int16_t delayLength1, delayLength2;
    uint16_t currentDelayPosition;
    int16_t delayIncrement; // fixed point decimal 1=1/4, decimal point after bis position 1, position 0 being lsb
    uint16_t buffersizePowerTwo;
    uint16_t buffersize;
} PitchshifterDataType;

typedef struct 
{
    float * delayBufferPtr;
    uint16_t pointerIncrement; // fixed point decimal 1=1/4, decimal point after bis position 1, position 0 being lsb
    uint8_t readPointerCount; // number of readpointers
    uint16_t writePointer;
    uint16_t * readPointers;
    uint16_t bufferSize; // the total size of the delay buffers
    uint16_t grainSize; // in samples, musn't be larger that bufferSize
} GrainsDataType;

typedef struct 
{
    float * delayMemoryPtr;
    int16_t delayPointer1,delayPointer2;
    uint16_t currentDelayPosition;
    int16_t delayIncrement; // fixed point decimal 1=1/4, decimal point after bis position 1, position 0 being lsb
    uint16_t buffersizePowerTwo;
    uint16_t buffersize;
    uint16_t crossFadeWidth;
    uint8_t crossFadeWidthPwr2;
} Pitchshifter2DataType;

typedef struct {
	uint32_t readPointer;
	uint32_t writePointer;
    float * delayMemoryPtr;
	uint16_t pointerIncrement; 
    FirstOrderIirType lpFilter;
	uint32_t zeroCrossings[PS3_ZERO_CROSSINGS_SIZE];
	uint16_t zeroCrossingPtr;
    uint32_t zeroCrossingsAddedSinceLastJump;
    uint32_t sampleCnt;
} Pitchshifter3DataType;

#define PITSHIFTER_BUFFER_SIZE_TWOS_POWER 11
#define PITCHSHIFTER_BUFFER_SIZE (1<<PITSHIFTER_BUFFER_SIZE_TWOS_POWER)
float pitchShifterProcessSample(float sampleIn,GrainsDataType*data);
void updatePointers(GrainsDataType* data);
float grainEnvelopeValue(GrainsDataType*data,uint8_t readPointerNr);
uint16_t distanceFromGrainEdge(GrainsDataType*data,uint8_t readPointerNr);
void initPitchshifter(GrainsDataType*data);
void deinitPitchshifter(GrainsDataType*data);
float pitchShifter2ProcessSample(float sampleIn,Pitchshifter2DataType*data);
void initPitchshifter2(Pitchshifter2DataType*data);
void deinitPitchshifter2(Pitchshifter2DataType*data);


void iniPitchShifter3(Pitchshifter3DataType*data);
void deinitPitchShifter3(Pitchshifter3DataType*data);
float ps3SummedDifferenceAbs(float * data,uint32_t idxa, uint32_t idxb,uint16_t windowSize);
uint32_t ps3DistanceFromWritePointer(Pitchshifter3DataType * cb);
uint16_t ps3PushZeroCrossing(uint32_t zcIdx,uint16_t minDist, Pitchshifter3DataType*cb);
void ps3IncrementPointer(Pitchshifter3DataType*cb);
float pitchShifter3ProcessSample(float newSample,Pitchshifter3DataType*cb);
#endif
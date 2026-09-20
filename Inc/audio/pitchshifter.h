#ifndef _PITCHSHIFTER_H_
#define _PITCHSHIFTER_H_

#include "stdint.h"
#include "firstOrderIirFilter.h"

#define PS3_ZERO_CROSSINGS_SIZE 256
#define PS3_MAX_BUFFER_SIZE 8192
#define PS3_DISTANCE_THRESHOLD 128
#define PS3_ANALYSIS_WINDOW_SIZE 64
#define PS3_READ_POINTER_FRACT 2 // how many positions the dot is away from the right in in fixed point notation of the read pointer, 2 means quarters, i.e. 1 is 1/4, 2 is 2/4 3 is 3/3 4 is 1
#define PS3_MIN_ZC_CANDIDATES 4
#define ZERO_PUSH_MIN_DIST 2

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
float pitchShifterProcessSample(float sampleIn,PitchshifterDataType*data);
void initPitchshifter(PitchshifterDataType*data);
void deinitPitchshifter(PitchshifterDataType*data);
float pitchShifter2ProcessSample(float sampleIn,Pitchshifter2DataType*data);
void initPitchshifter2(Pitchshifter2DataType*data);
void deinitPitchshifter2(Pitchshifter2DataType*data);


void iniPitchShifter3(Pitchshifter3DataType*data);
float ps3SummedDifferenceAbs(float * data,uint32_t idxa, uint32_t idxb);
uint32_t ps3DistanceFromWritePointer(Pitchshifter3DataType * cb);
uint16_t ps3PushZeroCrossing(uint32_t zcIdx,uint16_t minDist, Pitchshifter3DataType*cb);
void ps3IncrementPointer(Pitchshifter3DataType*cb);
float pitchShifter3ProcessSample(float newSample,Pitchshifter3DataType*cb);
#endif
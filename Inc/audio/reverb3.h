#ifndef _REVERB3_H_
#define _REVERB3_H_
#include "stdint.h"
#include "reverb.h"
#include "delay.h"

#define REV3_MODULATOR_STATE_RISING 0
#define REV3_MODULATOR_STATE_FALLING 1

typedef struct 
{
    HadamardDiffuserType diffusers[8];
    DelayDataType delay;
    int16_t delayModulationTime;
    float delayModulationDepth;
    int16_t sampleCounter;
    uint16_t delayLength;
    uint8_t modulatorState; // 0: rising, 1: falling
    uint8_t modulation;
} Reverb3Type;

void initReverb3(Reverb3Type*data);
float reverb3processSample(float sampleIn,Reverb3Type*data);


#endif
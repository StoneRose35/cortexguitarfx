#ifndef _REVERB3_H_
#define _REVERB3_H_
#include "stdint.h"
#include "reverb.h"
#include "delay.h"
typedef struct 
{
    HadamardDiffuserType diffusers[4];
    DelayDataType delay;
} Reverb3Type;

void initReverb3(Reverb3Type*data);
int16_t reverb3processSample(int16_t sampleIn,Reverb3Type*data);


#endif
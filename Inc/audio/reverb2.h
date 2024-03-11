#ifndef _REVERB_2_H_
#define _REVERB_2_H_
#include "stdint.h"
#include "reverb.h"
#include "delay.h"
#include "firstOrderIirFilter.h"

typedef struct 
{
    AllpassType aps[8];
    DelayDataType delaylines[4];
    FirstOrderIirType lowpass;
    float mix;
    float decay;
    float outs[4];
} Reverb2Type;


float reverb2ProcessSample(float sampleIn,Reverb2Type*reverbData);
void initReverb2(Reverb2Type*reverbData);

#endif
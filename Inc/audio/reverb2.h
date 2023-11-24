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
    int16_t mix;
    int16_t decay;
    int16_t outs[4];
} Reverb2Type;


int16_t reverb2ProcessSample(int16_t sampleIn,Reverb2Type*reverbData);
void initReverb2(Reverb2Type*reverbData);

#endif
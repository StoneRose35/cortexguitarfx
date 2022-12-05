#ifndef _SINE_CHORUS_H_
#define _SINE_CHORUS_H_
#include "stdint.h"

#define SINE_CHORUS_DELAY_SIZE 2048
#define SINE_CHORUS_LFO_DIVIDER 256
#ifndef FLOAT_AUDIO

#else
typedef struct 
{
    float delayBuffer[SINE_CHORUS_DELAY_SIZE];
    int16_t frequency; // in Hz/100, max 9375 --> 93.75 Hz
    int16_t depth; // 0 to 255
    float mix; // 0.0 to 1.0
    int16_t lfoVal; // from -255 to +255
    int16_t lfoValOld;
    uint32_t lfoPhaseinc;
    uint32_t lfoPhase;
    uint16_t delayInputPtr;
    uint16_t lfoUpdateCnt;
} SineChorusType;

float sineChorusProcessSample(float sampleIn,SineChorusType*data);
void initSineChorus(SineChorusType*data);
void sineChorusSetFrequency(uint16_t freq,SineChorusType*data);
float sineChorusInterpolatedProcessSample(float sampleIn,SineChorusType*data);

#endif

int16_t getSineValue(uint32_t);

#endif
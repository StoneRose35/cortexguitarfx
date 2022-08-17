#ifndef _SIMPLE_CHORUS_H_
#define _SIMPLE_CHORUS_H_
#include <stdint.h>

#define SIMPLE_CHORUS_DELAY_SIZE 2048
#define SIMPLE_CHORUS_LFO_DIVIDER 256
typedef struct 
{
    int16_t delayBuffer[SIMPLE_CHORUS_DELAY_SIZE];
    int16_t frequency; // in Hz/100, max 9375 --> 93.75 Hz
    int16_t depth; // 0 to 255
    int16_t mix; // 0 to 255
    int16_t lfoVal; // from -255 to +255
    int16_t lfoValOld;
    uint8_t lfoQuadrant; // 0 (ascending) or 1 (descending)
    int16_t lfoPhaseinc;
    uint16_t delayInputPtr;
    uint16_t lfoUpdateCnt;
} SimpleChorusType;

void simpleChorusSetFrequency(uint16_t freq,SimpleChorusType*data);

void initSimpleChorus(SimpleChorusType*data);

int16_t simpleChorusProcessSample(int16_t sampleIn,SimpleChorusType*data);
#endif
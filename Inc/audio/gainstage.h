#ifndef _GAINSTAGE_H_
#define _GAINSTAGE_H_
#include <stdint.h>

#ifndef FLOAT_AUDIO

typedef struct
{
    int16_t gain; // 256 is unity gain
    int16_t offset; // full range
    /* data */
} gainStageData;

int16_t gainStageProcessSample(int16_t sampleIn,gainStageData*data);
#else

typedef struct
{
    float gain; 
    float offset; 
    /* data */
} gainStageData;

float gainStageProcessSample(float sampleIn,gainStageData*data);

#endif

void initGainstage(gainStageData*data);

#endif
#ifndef _GAINSTAGE_H_
#define _GAINSTAGE_H_
#include <stdint.h>

typedef struct
{
    float gain; // 1.0f is unity gain
    float offset; // full range
    /* data */
} GainStageDataType;

void initGainstage(GainStageDataType*data);

float gainStageProcessSample(float sampleIn,GainStageDataType*data);

#endif
#ifndef _GAINSTAGE_H_
#define _GAINSTAGE_H_
#include <stdint.h>

typedef struct
{
    float gain; // 256 is unity gain
    float offset; // full range
    /* data */
} GainStageDataType;

void initGainstage(GainStageDataType*data);

float gainStageProcessSample(float sampleIn,GainStageDataType*data);

#endif
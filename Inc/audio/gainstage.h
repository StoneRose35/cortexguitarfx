#ifndef _GAINSTAGE_H_
#define _GAINSTAGE_H_
#include <stdint.h>

typedef struct
{
    int16_t gain; // 256 is unity gain
    int16_t offset; // full range
    /* data */
} GainStageDataType;

void initGainstage(GainStageDataType*data);

int16_t gainStageProcessSample(int16_t sampleIn,GainStageDataType*data);

#endif
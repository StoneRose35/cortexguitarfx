#include "audio/gainstage.h"


void initGainstage(GainStageDataType*data)
{
    data->gain=256;
    data->offset=0;
}

int16_t gainStageProcessSample(int16_t sampleIn,GainStageDataType*data)
{
    int16_t sampleOut;
    int32_t sampleWord = (int32_t)sampleIn;
    sampleWord = sampleWord* data->gain;
    sampleWord >>= 8;
    sampleWord = sampleWord + data->offset;
    if( sampleWord < -32768)
    {
        sampleOut = -32768;
    }
    else if (sampleWord > 32767)
    {
        sampleOut = 32767;
    }
    else
    {
        sampleOut=(int16_t)(sampleWord & 0xFFFF);
    }
    return sampleOut;
}
#include "audio/gainstage.h"

#ifndef FLOAT_AUDIO
void initGainstage(gainStageData*data)
{
    data->gain=255;
    data->offset=0;
}
int16_t gainStageProcessSample(int16_t sampleIn,gainStageData*data)
{
    int32_t sampleWord;
    int16_t sampleOut;

    sampleWord = (sampleIn * data->gain) >> 8;
    sampleWord = sampleWord + data->offset;
    
    if( sampleOut < -32768)
    {
        sampleOut = -32768;
    }
    else if (sampleOut > 32767)
    {
        sampleOut = 32767;
    }
    return sampleOut;
}
#else
void initGainstage(gainStageData*data)
{
    data->gain=1.0f;
    data->offset=0.0f;
}

__attribute__ ((section (".qspi_code")))
float gainStageProcessSample(float sampleIn,gainStageData*data)
{
    float sampleOut;

    sampleOut = sampleIn * data->gain + data->offset;
    
    if( sampleOut < -1.0f)
    {
        sampleOut = -1.0f;
    }
    else if (sampleOut > 1.0f)
    {
        sampleOut = 1.0f;
    }
    return sampleOut;
}
#endif
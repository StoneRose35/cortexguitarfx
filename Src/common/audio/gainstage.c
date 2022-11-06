#include "audio/gainstage.h"


void initGainstage(gainStageData*data)
{
    data->gain=1.0f;
    data->offset=0.0f;
}

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
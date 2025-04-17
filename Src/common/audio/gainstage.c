#include "audio/gainstage.h"
#include "audio/audiotools.h"
#include "memoryRegions.h"

__QSPI_CODE
void initGainstage(GainStageDataType*data)
{
    data->gain=1.0f;
    data->offset=0.0f;
}

__ITCM_CODE
float gainStageProcessSample(float sampleIn,GainStageDataType*data)
{
    float sampleOut;
    sampleOut = sampleIn * data->gain + data->offset;
    
    sampleOut=sampleOut;
    return sampleOut;
}

#include "audio/gainstage.h"
#include "audio/audiotools.h"


__attribute__((section (".qspi_code")))
void initGainstage(GainStageDataType*data)
{
    data->gain=1.0f;
    data->offset=0.0f;
}

__attribute__ ((section (".qspi_code")))
float gainStageProcessSample(float sampleIn,GainStageDataType*data)
{
    float sampleOut;
    sampleOut = sampleIn * data->gain + data->offset;
    
    sampleOut=sampleOut;
    return sampleOut;
}

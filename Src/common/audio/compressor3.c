#include "stdint.h"
#include "audio/compressor.h"
#include "stdio.h"
#include "fastExpLog.h"

float compressor3ProcessSample(float sampleIn,CompressorDataType*data)
{
    float absSample;
    float sampleOut;
    int32_t intermAvg;

    sampleOut = applyGain(sampleIn,data->currentAvg,data);

    if(sampleIn < 0)
    {
        absSample = -sampleIn;
    }
    else
    {
        absSample = sampleIn;
    }
    intermAvg = firstOrderIirDualCoeffLPProcessSample(absSample,&data->avgLowpass);
    data->currentAvg = intermAvg; 
    return sampleOut;
}

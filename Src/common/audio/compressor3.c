#include "stdint.h"
#include "audio/compressor.h"
#include "romfunc.h"
#include "fastExpLog.h"
#include "audio/firstOrderIirFilter.h"



int16_t compressor3ProcessSample(int16_t sampleIn,CompressorDataType*data)
{
    int16_t absSample;
    int16_t sampleOut;
    int32_t intermAvg;

    sampleOut = applyGain(sampleIn,data->currentAvg,data);
    /*
    if(sampleOut < 0)
    {
        absSample = -sampleOut;
    }
    else
    {
        absSample = sampleOut;
    }*/
    if(sampleIn < 0)
    {
        absSample = -sampleIn;
    }
    else
    {
        absSample = sampleIn;
    }
    intermAvg = firstOrderIirDualCoeffLPProcessSample(absSample,&data->avgLowpass);
    data->currentAvg = (int16_t)intermAvg; 
    return sampleOut;
}


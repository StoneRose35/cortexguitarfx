#include "audio/multimodefilter.h"
#include "memoryRegions.h"

__ITCM_CODE
float MMFilterProcessSample(float sampleIn,MultimodeFilterType*data)
{
    data->interm0 += data->cutoff*(sampleIn - data->interm0 + data->feedback*(data->interm0 - data->interm1));
    data->interm1 += data->cutoff*(data->interm0 - data->interm1);
    switch (data->type)
    {
    case MM_FILTER_LOWPASS:
        return data->interm1;
    case MM_FILTER_HIGHPASS:
        return sampleIn - data->interm1;
    case MM_FILTER_BANDPASS:
        return data->interm0 - data->interm1;
    default:
        return 0.0f;
    }
}


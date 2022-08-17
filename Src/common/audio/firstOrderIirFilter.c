
#include "audio/firstOrderIirFilter.h"

void initFirstOrderIirFilter(FirstOrderIirType*data)
{
    data->oldVal=0;
    data->alpha=10;
}

int16_t firstOrderIirLowpassProcessSample(int16_t sampleIn,FirstOrderIirType*data)
{
    //y[n] = alpha*y[n-1] + (1 - alpha)*x[n]
    data->oldVal = ((data->alpha*data->oldVal) >> 15) + ((((1 << 15) - data->alpha)*sampleIn) >> 15);
    return data->oldVal;
}

int16_t firstOrderIirHighpassProcessSample(int16_t sampleIn,FirstOrderIirType*data)
{
    //y[n] = 0.5*(1+alpha)*(x[n] - x[n-1])  + alpha*y[n-1]
    data->oldVal = (((((1 << 15)  + data->alpha) >> 1)*(sampleIn - data->oldXVal)) >> 15)   + ((data->alpha*data->oldVal) >> 15);
    data->oldXVal = sampleIn;
    return data->oldVal;
}
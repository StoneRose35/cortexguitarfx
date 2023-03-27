#include "audio/secondOrderIirFilter.h"

#ifndef FLOAT_AUDIO
void initSecondOrderIirFilter(SecondOrderIirFilterType* data)
{
    data->w[0]=0;
    data->w[1]=0;
    data->w[2]=0;
}

int16_t secondOrderIirFilterProcessSampleHiRes(int16_t sampleIn,SecondOrderIirFilterType*data)
{
    int32_t out;
    int64_t qprod;
    qprod = sampleIn;
    qprod -= ((int64_t)data->coeffA[0]*(int64_t)data->w[1]) >> 15;
    qprod -= ((int64_t)data->coeffA[1]*(int64_t)data->w[2]) >> 15;
    data->w[0] = (int32_t)qprod;

    qprod = ((int64_t)data->coeffB[0]*(int64_t)data->w[0]) >> 15;
    qprod += ((int64_t)data->coeffB[1]*(int64_t)data->w[1]) >> 15;
    qprod += ((int64_t)data->coeffB[2]*(int64_t)data->w[2]) >> 15;
    if (qprod > 32767)
    {
        qprod = 32767;
    }
    else if (qprod < -32767)
    {
        qprod = -32767;
    }
    out = (int32_t)qprod;
    data->w[2] = data->w[1];
    data->w[1] = data->w[0];

    return (int16_t)(out & 0xFFFF);
}

int16_t secondOrderIirFilterProcessSample(int16_t sampleIn,SecondOrderIirFilterType*data)
{
    int32_t out;
    sampleIn >>= (16 - data->bitRes);
    data->w[0] = (int32_t)sampleIn;
    data->w[0] -= (((int32_t)data->coeffA[0]*(int32_t)data->w[1]) >> (data->bitRes -1));
    data->w[0] -= (((int32_t)data->coeffA[1]*(int32_t)data->w[2]) >> (data->bitRes -1));

    out = (((int32_t)data->coeffB[0]*(int32_t)data->w[0]) >> (data->bitRes -1))
        + (((int32_t)data->coeffB[1]*(int32_t)data->w[1]) >> (data->bitRes -1))
        + (((int32_t)data->coeffB[2]*(int32_t)data->w[2]) >> (data->bitRes -1));
    data->w[2] = data->w[1];
    data->w[1] = data->w[0];
    out <<= (16 - data->bitRes);
    if (out > 32767)
    {
        out = 32767;
    }
    else if (out < -32767)
    {
        out = -32767;
    }
    return (int16_t)(out & 0xFFFF);
}
#else

__attribute__ ((section (".qspi_code")))
float secondOrderIirFilterProcessSample(float sampleIn,SecondOrderIirFilterType*data)
{
    float out;
    data->w[0] = sampleIn - data->coeffA[0]*data->w[1] - data->coeffA[1]*data->w[2];
    out = data->coeffB[0]*data->w[0]+data->coeffB[1]*data->w[1]+data->coeffB[2]*data->w[2];
    data->w[2]=data->w[1];
    data->w[1]=data->w[0];
    return out;
}

void initSecondOrderIirFilter(SecondOrderIirFilterType* data)
{
    data->w[0]=0.0f;
    data->w[1]=0.0f;
    data->w[2]=0.0f;
}

#endif
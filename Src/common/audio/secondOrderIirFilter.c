#include "audio/secondOrderIirFilter.h"

/*
implementation taken from 
https://dsp.stackexchange.com/questions/21792/best-implementation-of-a-real-time-fixed-point-iir-filter-with-constant-coeffic
*/

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
    /*
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
    */
    int16_t res;
    data->acc += data->coeffB[0]*sampleIn;
    data->acc += data->coeffB[1]*data->x1;
    data->acc += data->coeffB[2]*data->x2;
    data->acc -= data->coeffA[0]*data->y1;
    data->acc -= data->coeffA[1]*data->y2;
    if (data->acc > ((1 << 29)-1) ) // two q14 numbers multipled and added give a maximum of (1 << 14+14+1), -1 for maximum since two's complement range are asymmetric
    {
        data->acc = ((1 << 29)-1); 
    }   
    if (data->acc < -(1 << 29))
    {
        data->acc = -(1 << 29);
    }
    res = (int16_t)(data->acc >> 14);
    data->x2 = data->x1;
    data->x1 = sampleIn;
    data->y2 = data->y1;
    data->y1 = res;
    data->acc &= ((1 << 14)-1);

    return res;

}


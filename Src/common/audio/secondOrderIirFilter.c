#include "audio/secondOrderIirFilter.h"

/*
implementation taken from 
https://dsp.stackexchange.com/questions/21792/best-implementation-of-a-real-time-fixed-point-iir-filter-with-constant-coeffic
*/

void initSecondOrderIirFilter(SecondOrderIirFilterType* data)
{
    data->x1=0;
    data->x2=0;
    data->y1=0;
    data->y2=0;
    data->acc=0;
}


int16_t secondOrderIirFilterProcessSample(int16_t sampleIn,SecondOrderIirFilterType*data)
{
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

void secondOrderIirFilterReset(SecondOrderIirFilterType*data)
{
    data->acc=0;
    data->x1=0;
    data->x2=0;
    data->y1=0;
    data->y2=0;
}


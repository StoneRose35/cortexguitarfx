#include "audio/secondOrderIirFilter.h"

void initSecondOrderIirFilter(SecondOrderIirFilterType* data)
{
    data->x1=0.0f;
    data->x2=0.0f;
    data->y1=0.0f;
    data->y2=0.0f;
    data->acc=0.0f;
}

__attribute__ ((section (".qspi_code")))
float secondOrderIirFilterProcessSample(float sampleIn,SecondOrderIirFilterType*data)
{
    data->acc += data->coeffB[0]*sampleIn +
                 data->coeffB[1]*data->x1 + 
                 data->coeffB[2]*data->x2 -
                 data->coeffA[0]*data->y1 -
                 data->coeffA[2]*data->y2;
    data->x2 = data->x1;
    data->x1 = sampleIn;
    data->y2 = data->y1;
    data->y1 = data->acc;
    return data->acc;
}

__attribute__((section (".qspi_code")))
void secondOrderIirFilterReset(SecondOrderIirFilterType*data)
{
    data->acc=0.0f;
    data->x1=0.0f;
    data->x2=0.0f;
    data->y1=0.0f;
    data->y2=0.0f;
}


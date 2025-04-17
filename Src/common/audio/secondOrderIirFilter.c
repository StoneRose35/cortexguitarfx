#include "audio/secondOrderIirFilter.h"
#include "memoryRegions.h"

__QSPI_CODE
void initSecondOrderIirFilter(SecondOrderIirFilterType* data)
{
    data->x1=0.0f;
    data->x2=0.0f;
    data->y1=0.0f;
    data->y2=0.0f;
    data->acc=0.0f;
}

__ITCM_CODE
float secondOrderIirFilterProcessSample(float sampleIn,SecondOrderIirFilterType*data)
{
    data->acc = data->coeffB[0]*sampleIn;
    data->acc += data->coeffB[1]*data->x1; 
    data->acc += data->coeffB[2]*data->x2;
    data->acc -= data->coeffA[0]*data->y1;
    data->acc -= data->coeffA[1]*data->y2;
    data->x2 = data->x1;
    data->x1 = sampleIn;
    data->y2 = data->y1;
    data->y1 = data->acc;
    return data->acc;
}

__QSPI_CODE
void secondOrderIirFilterReset(SecondOrderIirFilterType*data)
{
    data->acc=0.0f;
    data->x1=0.0f;
    data->x2=0.0f;
    data->y1=0.0f;
    data->y2=0.0f;
}


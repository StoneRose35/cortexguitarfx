

#include "stdint.h"
#include "stm32h750/helpers.h"
#include "audio/fxprogram/fxProgram.h"


int32_t convolvetest()
{
    float coefficients[64];
    float data[64];
    volatile float convolveres;
    volatile int32_t failures = 0;
    volatile uint32_t cntr;
    for (uint8_t c=0;c<64;c++)
    {
        *(data+c)=((float)c);
        *(coefficients+c)=0.0f;
    }
    *(coefficients+0)=0.5f;
    *(coefficients+63)=0.5f;
    for (cntr=0;cntr<64;cntr++)
    {
        convolveres = convolve(coefficients,data,cntr);
        if ((convolveres - ((float)cntr)) < -0.001f || (convolveres - ((float)cntr)) > 0.001f)
        {
            failures++;
        }
    }
    return failures;
}

uint32_t firfiltertest()
{
    FxProgram1DataType* fp1data = (FxProgram1DataType*)fxProgram1.data;
    initfirFilter(&fp1data->filter3);
    volatile float fout;
    volatile uint32_t failures=0;
    for (uint8_t c=0;c<64;c++)
    {
        if (c==0)
        {
            fout = firFilterProcessSample(8388607.0f,&fp1data->filter3);
        }
        else
        {
            fout = firFilterProcessSample(0.0f,&fp1data->filter3);
        }
        if ((fout - (8388607.0f*fp1data->filter3.coefficients[c])) < -0.001f || (fout - (8388607.0f*fp1data->filter3.coefficients[c])) > 0.001f)
        {
            failures++;
        }
    }
    return failures;
}


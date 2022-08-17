
#include "stdint.h"
#include "audio/compressor.h"
#include "stdio.h"
#include "stdlib.h"
#include "math.h"

float int2float(int32_t a)
{
    return (float)a;
}

int32_t float2int(float a)
{
    return (int32_t)a;
}


#define COMPRESSORTEST_F0 326.0f
void sineTest()
{
    CompressorDataType compressor = {
        .attack=(20 << 3),
        .release=(20 << 3),
        .currentAvg=0,
        .gainFunction.gainReduction = 3,
        .gainFunction.threshhold=9600
    };
    float samplef;
    int16_t sample, sampleOut;

    for (uint32_t q=0;q<10000;q++)
    {
        samplef = sinf(6.28318f/48000.0f*q*COMPRESSORTEST_F0)*32767.0f;
        sample = float2int(samplef);
        sampleOut = compressorProcessSample(sample,&compressor);
        printf("In: %d, Out: %d, Average: %d\n",sample,sampleOut,compressor.currentAvg);
    }
    
}

void gainFunctionTest()
{
    CompressorDataType comp;
    int16_t outGain;
    comp.gainFunction.gainReduction=2;
    comp.gainFunction.threshhold=16384;

    for(int16_t c=0;c<101;c++)
    {
        outGain = applyGain((c-50)*655,abs((c-50)*655),&comp);
        printf("Input: %d, Output: %d\n",(c-50)*655,outGain);
    }
}

int main(int argc,char ** argv)
{
    sineTest();
}
#include <stdint.h>
#include "stdio.h"
#include "audio/sineplayer.h"
#include "audio/secondOrderIirFilter.h"
#include "audio/waveShaper.h"
#include "math.h"

float int2float(int32_t a)
{
    return (float)a;
}

int32_t float2int(float a)
{
    return (int32_t)a;
}

void impulseTest()
{
    FILE * fid;
    int16_t cval;
    int16_t val_out;
    SecondOrderIirFilterType testFilter = {
        .coeffB = {1007, -2014, 1007},
        .coeffA = {-2013,991},
        .w= {0,0,0},
        .bitRes=11
    };

    fid = fopen("audioout.txt","wt");
    for (uint16_t c=0;c< 1024;c++)
    {
        if (c==0)
        {
            cval = 32767;
        }
        else
        {
            cval = 0;
        }
        val_out = secondOrderIirFilterProcessSample(cval,&testFilter);
        fprintf(fid,"%d\r\n",val_out);
    }
    fclose(fid);
}

void waveshaperTest()
{
    WaveShaperDataType wsData;
    initWaveShaper(&wsData,&waveShaperDefaultOverdrive);

    waveShaperProcessSample(0,&wsData);
    waveShaperProcessSample(100,&wsData); 
    waveShaperProcessSample(-100,&wsData);   
}

int main()
{
    //impulseTest();
    waveshaperTest();
}
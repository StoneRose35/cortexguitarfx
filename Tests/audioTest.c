#include <stdint.h>
#include "stdio.h"
#include "audio/sineplayer.h"
#include "audio/secondOrderIirFilter.h"
#include "audio/waveShaper.h"
#include "math.h"

#define INCREMENT_DELTA_2 1
#define INCREMENT_DELTA_1 3
# define SPLICING_AREA_SIZE 16

float int2float(int32_t a)
{
    return (float)a;
}

int32_t float2int(float a)
{
    return (int32_t)a;
}

float fln(float a)
{
    return logf(a);
}


void impulseTest()
{
    FILE * fid;
    int16_t cval;
    int16_t val_out;
    SecondOrderIirFilterType testFilter = {
        .coeffB = {1007, -2014, 1007},
        .coeffA = {-2013,991}
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

void delayTimeDiffTest()
{
    // index 1 is incrementing slower than index 2
    int16_t idx1 = 0;
    int16_t idx2 = 123;
    uint32_t bufferSize=128;
    int16_t idx1New,idx2New;

    int16_t deltaIdx;
    int16_t mask =  (bufferSize - 1);
    for (uint32_t c=0;c<768;c++)
    {
        if (idx1 >= idx2)
        {
            deltaIdx = (idx1-idx2);
        }
        else
        {
            deltaIdx = (bufferSize - idx2 + idx1);
        }
        printf("current position idx1: %d, idx2: %d, delta: %d\r\n",idx1,idx2,deltaIdx);
        uint8_t idx1BehindIdx2 = idx2 > idx1; // slower behind faster?
        idx1New = (idx1 + INCREMENT_DELTA_1) & (bufferSize - 1);
        idx2New = (idx2 + INCREMENT_DELTA_2) & (bufferSize - 1);
        uint8_t idx1Jumped = idx1New < idx1; // slower jumped?
        uint8_t idx2Jumped = idx2New < idx2; // faster jumped?
        uint8_t idx1BehindIdx2New = idx2New > idx1New; // slower behind faster after increment of both?

        if (((idx2Jumped) && (idx1BehindIdx2 == idx1BehindIdx2New)) || // faster index wrapped around and surpassed at the same time
            (!idx1BehindIdx2 && idx1BehindIdx2New && !idx1Jumped)) // slower index now is behind faster one, slower index didn't wrap around
        {
            printf("index 2 surpassed index 1\r\n");
        }

        idx1 = idx1New;
        idx2 = idx2New;
    }
}

void bitMaskingTest()
{
    uint16_t bitMask = 0xFFF8;
    int16_t maskedVal;
    for (int16_t val = -32;val < 33;val++)
    {
        maskedVal = (int16_t)(((uint16_t)val) & bitMask);
        printf("original: %d, masked: %d\n",val,maskedVal);

    }
}

int main()
{
    //impulseTest();
    //waveshaperTest();
    //delayTimeDiffTest();
    bitMaskingTest();
}
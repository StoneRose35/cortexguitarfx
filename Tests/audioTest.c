#include <stdint.h>
#include "stdio.h"
#include "audio/secondOrderIirFilter.h"
#include "audio/waveShaper.h"
#include "audio/sineChorus.h"
#include "math.h"
#include "ln.h"
#include "fastExpLog.h"

float int2float(int32_t a)
{
    return (float)a;
}

int32_t float2int(float a)
{
    return (int32_t)a;
}

float convolve(const float*coeffs,float*data,uint32_t offset)
{
    float res=0.0f;
    for(uint32_t c=0;c<64;c++)
    {
        res += *(coeffs + c) * *(data + ((offset + c)&0x3F));
    }
    return res;
}

void impulseTest()
{
    FILE * fid;
    int16_t cval;
    int16_t val_out;
    SecondOrderIirFilterType testFilter = {
        .coeffB = {1007.f/32768.f, -2014.f/32768.f, 1007.f/32768.f},
        .coeffA = {-2013.f/32768.f,991.f/32768.f},
        .x1=0.f,
        .x2=0.f,
        .y1=0.f,
        .y2=0.f,
        .acc=0.f
    };

    fid = fopen("audioout.txt","wt");
    for (uint16_t c=0;c< 1024;c++)
    {
        if (c==0)
        {
            cval = 1.0f;
        }
        else
        {
            cval = 0.f;
        }
        val_out = secondOrderIirFilterProcessSample(cval,&testFilter);
        fprintf(fid,"%d\r\n",val_out);
    }
    fclose(fid);
}

void sineFunctionTest()
{
    int16_t sval;
    for (uint32_t c=0;c<65536;c+=512)
    {
        sval = getSineValue((uint16_t)c);
        printf("%d,%d\r\n",c,sval);
    }
}

void waveshaperTest()
{
    WaveShaperDataType wsData;
    initWaveShaper(&wsData,&waveShaperDefaultOverdrive);

    waveShaperProcessSample(0,&wsData);
    waveShaperProcessSample(100,&wsData); 
    waveShaperProcessSample(-100,&wsData);   
}

void multiWaveShaperTest()
{
    MultiWaveShaperDataType wsData;
    initMultiWaveShaper(&wsData,&multiWaveshaper1);
    wsData.functionIndex = 45;
    multiWaveShaperProcessSample(0.0f,&wsData);
    multiWaveShaperProcessSample(0.01f,&wsData);
    multiWaveShaperProcessSample(-0.01f,&wsData);
}

void logComparisonTest()
{
    FILE * fid;
    float xval;
    float xInterm;
    float yval1,yval2,diff;
    xval = 0.0f;
    fid = fopen("logdiff.txt","wt");
    while (xval < 1.0f)
    {
        xval += 0.001;
        xInterm = xval;
        yval1 = toDb(xInterm);
        xInterm = xval;
        yval2 = fastlog(xInterm);
        diff = yval2 - yval1;
        fprintf(fid,"%f\r\n",diff);
    }

    fclose(fid);
}

void linComparisonTest()
{
    FILE * fid;
    float xval;
    float xInterm;
    float yval1,yval2,diff;
    xval = -120.0f;
    fid = fopen("lindiff.txt","wt");
    while (xval < 0.0f)
    {
        xval += 0.1;
        xInterm = xval;
        yval1 = toLin(xInterm);
        xInterm = xval;
        yval2 = fastexp(xInterm);
        diff = yval2 - yval1;
        fprintf(fid,"%f\r\n",diff);
    }

    fclose(fid);
}

void logAndInverseTest()
{
    FILE * fid;
    float xval;
    float xInterm;
    float yval1,xrecalculated,diff;

    xval = 0.0f;
    fid = fopen("loginverse.txt","wt");
    while (xval < 1.0f)
    {
        xval += 0.0001;
        xInterm = xval;
        yval1 = toDb(xInterm);
        xrecalculated = toLin(yval1);
        diff = xrecalculated - xval;
        fprintf(fid,"%f\r\n",diff);
    }
    fclose(fid);
}

void logAndInverseTest2()
{
    FILE * fid;
    float xval;
    float xInterm;
    float yval1,xrecalculated,diff;

    xval = 0.0f;
    fid = fopen("loginverse2.txt","wt");
    while (xval < 1.0f)
    {
        xval += 0.0001;
        xInterm = xval;
        yval1 = fastlog(xInterm);
        xrecalculated = fastexp(yval1);
        diff = xrecalculated - xval;
        fprintf(fid,"%f\r\n",diff);
    }
    fclose(fid);
}

void toExpTest()
{
    float xval=0.0f;
    float yval;
    while (xval < 4096.0f)
    {    
        yval=0.01f*toExp(xval*0.0011245f);
        printf("x: %f, 0.01*exp(x*0.0011245): %f\r\n",xval,yval);
        xval += 8.0f;
    }

}

int main()
{
    //impulseTest();
    //waveshaperTest();
    //sineFunctionTest();
    //multiWaveShaperTest();
    //logComparisonTest();
    //linComparisonTest();
    //logAndInverseTest2();
    toExpTest();
}
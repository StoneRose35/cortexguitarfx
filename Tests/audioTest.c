#include <stdlib.h>
#include <time.h>
#include "stdio.h"
#include "audio/secondOrderIirFilter.h"
#include "audio/waveShaper.h"
#include "audio/sineChorus.h"
#include "audio/genericDistortion.h"
#include "audio/genericDistortionSimple.h"
#include "math.h"
#include "matrixMath.h"
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
    float xval=-16.0;
    float yval;
    while (xval < 16.0f)
    {    
        yval=toExp(xval);
        printf("x: %f, exp(x): %f\r\n",xval,yval);
        xval += 0.1f;
    }
}

void matrixInverseTest()
{
    Mat4x4Type a;
    Mat4x4Type b;
    Mat4x4Type res;
    uint8_t failure=0;
    srand(34352);
    for (uint8_t c=0;c<4;c++)
    {
        for (uint8_t cc=0;cc<4;cc++)
        {
            a.mat[cc][c] = ((float)rand())/(float)RAND_MAX*4.0f;
        }
    }
    minv(&a,&b);
    mmult(&a,&b,&res);
    for (uint8_t q=0;q<4;q++)
    {
        if ((res.mat[q][q] -1.0f) > 0.00001f || (res.mat[q][q] -1.0f) < -0.00001f)
        {
            failure = 1;
            printf("diagonal element %i is %f instead of 1.0f\r\n",q,res.mat[q][q]);
        }
    }
    for (uint8_t c=0;c<4;c++)
    {
        for (uint8_t cc=0;cc<4;cc++)
        {
            if (cc != c && ( res.mat[cc][c] > 0.00001f || res.mat[cc][c] < -0.00001f))
            {
                failure = 1;
                printf("element [%i][%i] is %f instead of 0.0f\r\n",cc,c,res.mat[cc][c]);
            }
        }
    }
    if (!failure)
    {
        printf("Matrix inversion Test OK\r\n");
    }
}

void genericDistortionTest()
{
    GenericDistortionType dist;
    float point[2];
    point[0]=0.1f;
    point[1] = 0.04f;
    gdSetPoint(GD_POINT_A,point,&dist);

    point[0] = 0.13f;
    point[1] = 0.05f;
    gdSetPoint(GD_POINT_B,point,&dist);

    point[0] = 0.46f;
    point[1]= 0.9f;
    gdSetPoint(GD_POINT_C,point,&dist);

    point[0] = 0.13f;
    point[1] = 0.05f;
    gdSetPoint(GD_POINT_B,point,&dist);

    point[0] = 0.47f;
    point[1]= 0.91f;
    gdSetPoint(GD_POINT_D,point,&dist);

    float xval, yval;
    for (uint8_t c=0;c<127;c++)
    {
        xval = ((float)c)/127.0f;
        yval = gdGetValue(xval,&dist);
        printf("%f,%f\r\n",xval,yval);
    }
}

void genericDistortionSimpleTest()
{
    GenericDistortionSimpleType dist;
    float points[4];
    points[0] = 0.01f;
    points[1] = 0.01f;

    points[2] = 0.6f;
    points[3] = 0.93f;

    gdsSetAllPoints(points,&dist);


    float xval, yval;
    for (uint8_t c=0;c<127;c++)
    {
        xval = ((float)c)/127.0f;
        yval = gdsGetValue(xval,&dist);
        printf("%f,%f\r\n",xval,yval);
    }
}


void pointerDifferenceTest()
{
    uint16_t ptr1=122;
    uint16_t ptr2= 115;
    for(uint8_t c=0;c <12;c++)
    {
        printf("ptr1:%d,ptr2:%d, diff:%d\r\n",ptr1,ptr2, (ptr1-ptr2)&0x7F);
        ptr1++;
        ptr1 &= (127);
        ptr2++;
        ptr2 &= (127);
    }
}

void shiftRightTest()
{
    int16_t val = -12;
    val >>= 1;
    printf("excepting %d to be -6",val);
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
    //toExpTest();
    //matrixInverseTest();
    //genericDistortionSimpleTest();
    //pointerDifferenceTest();
    shiftRightTest();
}
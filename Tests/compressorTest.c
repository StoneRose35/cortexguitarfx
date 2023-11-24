
#include "stdint.h"
#include "inc/wavReader.h"
#include "audio/compressor.h"
#include <stdio.h>
#include <stdlib.h> 
#include <string.h>
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
        .avgLowpass.alphaFalling = 13,
        .avgLowpass.alphaRising = 13,
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


void processSampleThoughCompressor(char * filenameIn, char * filenameOut)
{
    char filenameInBfr[256];
    char filenameOutBfr[256];
    WavFileType wavFileIn;
    WavFileType wavFileOut;
    uint32_t byteCnt=0;
    int16_t sample[2];
    int16_t sampleOut;
    CompressorDataType compressor = {
        .avgLowpass.alphaFalling = 32766,
        .avgLowpass.alphaRising = 1,
        .currentAvg=0,
        .gainFunction.gainReduction = 4,
        .gainFunction.threshhold=32767
    };
    strcpy(filenameInBfr,filenameIn);
    openWavFile(filenameIn,&wavFileIn);
    strcpy(filenameOutBfr,filenameOut);
    createWavFile(filenameOutBfr,&wavFileOut,wavFileIn.dataSize);
    printf("In,Out,Average\n");
    while(byteCnt < wavFileIn.dataSize)
    {
        sample[0] = wavFileIn.data[byteCnt>>1];
        if (wavFileIn.wavFormat.wChannels == 2)
        {
            sample[1] = wavFileIn.data[(byteCnt>>1)+1];
        }
        sampleOut = compressorProcessSample(sample[0],&compressor);
        printf("%d,%d,%d\n",sample[0],sampleOut,compressor.currentAvg);
        wavFileOut.data[byteCnt>>1]=compressor.currentAvg;
        if (wavFileIn.wavFormat.wChannels==2)
        {
            byteCnt+=4;
        }
        else
        {
            byteCnt+=2;
        }
    }
    writeWavFile(&wavFileOut);
    fclose(wavFileIn.filePointer);
    fclose(wavFileOut.filePointer);
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
    //sineTest();
    processSampleThoughCompressor(argv[1], argv[2]);
}
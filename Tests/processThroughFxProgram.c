#include "inc/wavReader.h"
#include "audio/fxprogram/fxProgram.h"
#include "romfunc.h"
#include <stdio.h>
#include <stdlib.h> 
#include <string.h>


#define SELECTED_FX_PROGRAM 1
#define FX_PROGRAM_PARAM1_VAL 200 // amp model 2: gain 
#define FX_PROGRAM_PARAM2_VAL 3800 // 
#define FX_PROGRAM_PARAM3_VAL 520 // 
#ifndef FLOAT_AUDIO
#define TAIL_TIME 48000
#else
#define TAIL_TIME (48000*3)
#endif

void zeroString(char*data,int16_t len)
{
    for (uint16_t c=0;c<len;c++)
    {
        *(data+c)=0;
    }
}

float int2float(int32_t a)
{
    return (float)a;
}

int32_t float2int(float a)
{
    return (int32_t)a;
}

int main(int argc, char ** argv)
{
    uint32_t fileInPtr=0;
    #ifndef FLOAT_AUDIO
    WavFileType wavFileIn;
    WavFileType wavFileOut;
    int16_t sample[2];
    int16_t dataOut;
    #else
    WavFileTypeFloat wavFileIn;
    WavFileTypeFloat wavFileOut;
    float sample[2];
    float dataOut;
    #endif

    char filenameOut[256];
    char filenameIn[256];
    char paramDisplay[64];
    size_t fnameLength;
    uint16_t params[8]={0,0,0,0,0,0,0,0};
    uint8_t fxProgramNr;
    printf("Offline FxProgram Processor v0.1\r\n\r\n");
    if (argc == 1)
    {
        printf("Useage: processThroughFxProgram <fileIn.wav> <fxProgram> <param1> <param2> <param3> (<param4>..)\r\n");
        printf("processing hard-coded values\r\n");
        params[0]=FX_PROGRAM_PARAM1_VAL;
        params[1]=FX_PROGRAM_PARAM2_VAL;
        params[2]=FX_PROGRAM_PARAM3_VAL;
        fxProgramNr=SELECTED_FX_PROGRAM;
        strcpy(filenameIn,"./audiosamples/guit_riff_16bit.wav");
        strcpy(filenameOut,"./audiosamples/guit_riff_16bit_proc.wav");
    }
    else
    {
        strcpy(filenameIn,argv[1]);
        strcpy(filenameOut,argv[1]);
        fnameLength = strlen(filenameOut);
        filenameOut[fnameLength-4]=0;
        strcat(filenameOut,"_proc.wav");
        fxProgramNr = (uint8_t)atoi(argv[2]);
        for (uint8_t c=3;c<argc;c++)
        {
            params[c-3]=(uint16_t)atoi(argv[c]);
        }

    }
    printf("processing file %s\r\n",filenameIn);
    printf("\tFx Program: %s\r\n",fxPrograms[fxProgramNr]->name);
    size_t fnamelen = strlen(filenameIn);
    strcpy(filenameOut+fnamelen-4,"_proc.wav");
    #ifndef FLOAT_AUDIO
    openWavFile(filenameIn,&wavFileIn);
    createWavFile(filenameOut,&wavFileOut,wavFileIn.dataSize+TAIL_TIME);
    #else
    openWavFileFloat(filenameIn,&wavFileIn);
    createWavFileFloat(filenameOut,&wavFileOut,wavFileIn.dataSize+TAIL_TIME,wavFileIn.wavFormat.wBitsPerSample);
    wavFileOut.wavFormat.wBitsPerSample = wavFileIn.wavFormat.wBitsPerSample;
    #endif

    for (uint16_t c=0;c<N_FX_PROGRAMS;c++)
    {
        if (fxPrograms[c]->setup != 0)
        {
            fxPrograms[c]->setup(fxPrograms[c]->data);
        }
    }
    for (uint8_t c=0;c<fxPrograms[fxProgramNr]->nParameters;c++)
    {
        if (fxPrograms[fxProgramNr]->parameters[c].control == 0)
        {
            zeroString(paramDisplay,64);
            fxPrograms[fxProgramNr]->parameters[c].setParameter(params[0],fxPrograms[fxProgramNr]->data);
            fxPrograms[fxProgramNr]->parameters[c].getParameterDisplay(fxPrograms[fxProgramNr]->data,paramDisplay);

            printf("\tParameter 1: %s\r\n",paramDisplay);
        }
        else if (fxPrograms[fxProgramNr]->parameters[c].control == 1)
        {
            zeroString(paramDisplay,64);
            fxPrograms[fxProgramNr]->parameters[c].setParameter(params[1],fxPrograms[fxProgramNr]->data);
            fxPrograms[fxProgramNr]->parameters[c].getParameterDisplay(fxPrograms[fxProgramNr]->data,paramDisplay);
            printf("\tParameter 2: %s\r\n",paramDisplay);
        }
        else if (fxPrograms[fxProgramNr]->parameters[c].control == 2)
        {
            zeroString(paramDisplay,64);           
            fxPrograms[fxProgramNr]->parameters[c].setParameter(params[2],fxPrograms[fxProgramNr]->data);
            fxPrograms[fxProgramNr]->parameters[c].getParameterDisplay(fxPrograms[fxProgramNr]->data,paramDisplay);
            printf("\tParameter 3: %s\r\n",paramDisplay);   
        }
        else
        {
            zeroString(paramDisplay,64);           
            fxPrograms[fxProgramNr]->parameters[c].setParameter(params[c],fxPrograms[fxProgramNr]->data);
            fxPrograms[fxProgramNr]->parameters[c].getParameterDisplay(fxPrograms[fxProgramNr]->data,paramDisplay);
            printf("\tParameter %d (opt): %s\r\n",c+1,paramDisplay);   
        }

    }
    #ifndef FLOAT_AUDIO
    while(fileInPtr < wavFileIn.dataSize+TAIL_TIME)
    {
        if (fileInPtr < wavFileIn.dataSize)
        {
            sample[0] = wavFileIn.data[fileInPtr>>1];
            if (wavFileIn.wavFormat.wChannels == 2)
            {
                sample[1] = wavFileIn.data[(fileInPtr>>1)+1];
            }
        }
        else
        {
            sample[0]=0;
        }
        dataOut = fxPrograms[fxProgramNr]->processSample(sample[0],fxPrograms[fxProgramNr]->data);
        if (wavFileIn.wavFormat.wChannels==2)
        {
            wavFileOut.data[fileInPtr >> 2]=dataOut;
            fileInPtr+=4;
        }
        else
        {
            wavFileOut.data[fileInPtr>>1]=dataOut;
            fileInPtr+=2;
        }
    }
    writeWavFile(&wavFileOut);
    #else
    const uint32_t lengthInSamples = ((wavFileIn.dataSize/(wavFileIn.wavFormat.wBitsPerSample >> 3)*wavFileIn.wavFormat.wChannels));
    while(fileInPtr < lengthInSamples +TAIL_TIME/(wavFileIn.wavFormat.wBitsPerSample >> 3))
    {
        if (fileInPtr < lengthInSamples)
        {
            sample[0]= wavFileIn.data[fileInPtr];
            if (wavFileIn.wavFormat.wChannels == 2)
            {
                sample[1] = wavFileIn.data[fileInPtr+1];
            }
        }
        else
        {
            sample[0]=0.0f;
        }
        dataOut = fxPrograms[fxProgramNr]->processSample(sample[0],fxPrograms[fxProgramNr]->data);
        if (wavFileIn.wavFormat.wChannels==2)
        {
            wavFileOut.data[fileInPtr>>1];
            fileInPtr+=2;
        }
        else
        {
            wavFileOut.data[fileInPtr++];
        }
    }
    writeWavFileFloat(&wavFileOut);
    #endif

    fclose(wavFileIn.filePointer);
    fclose(wavFileOut.filePointer);
}
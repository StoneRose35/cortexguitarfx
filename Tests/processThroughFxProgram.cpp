#include "pipicofx/FxProgramLoader.hpp"
#include "pipicofx/FxProgram.hpp"
extern "C" {
#include "inc/wavReader.h"
#include "audio/audiotools.h"
#include "pipicofx/delayMemoryHandler.h"
#include <stdio.h>
#include <stdlib.h> 
#include <string.h>
#include "math.h"
}
using namespace PiPicoFX;
#define SELECTED_FX_PROGRAM 1
#define FX_PROGRAM_PARAM1_VAL 200 // amp model 2: gain 
#define FX_PROGRAM_PARAM2_VAL 3800 // 
#define FX_PROGRAM_PARAM3_VAL 520 // 
#define TAIL_TIME 48000

extern "C" {
void zeroString(char*data,int16_t len)
{
    for (uint16_t c=0;c<len;c++)
    {
        *(data+c)=0;
    }
}

/*
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
*/

float convolve(const float*coeffs,float*data,uint32_t offset)
{
    float res=0.0f;
    for(uint32_t c=0;c<64;c++)
    {
        res += *(coeffs + c) * *(data + ((offset + c)&0x3F));
    }
    return res;
}

}

uint32_t processOffline(int argc,char ** argv,char * jsonBfr)
{
uint32_t byteCnt=0;
    char filenameOut[256];
    char filenameIn[256];
    uint8_t fxProgramNr;
    float dataOut;
    size_t fnameLength;
    WavFileType wavFileIn;
    WavFileType wavFileOut;
    char paramDisplay[64];
    uint16_t params[64]={   0,0,0,0,0,0,0,0,
                            0,0,0,0,0,0,0,0,
                            0,0,0,0,0,0,0,0,
                            0,0,0,0,0,0,0,0,
                            0,0,0,0,0,0,0,0,
                            0,0,0,0,0,0,0,0,
                            0,0,0,0,0,0,0,0,
                            0,0,0,0,0,0,0,0};
    int16_t sample[2];
    uint32_t jsonStringPtr=0;
    FxProgram * currentProgram;
    strcpy(filenameIn,argv[2]);
    strcpy(filenameOut,argv[2]);
    fnameLength = strlen(filenameOut);
    filenameOut[fnameLength-4]=0;
    strcat(filenameOut,"_proc.wav");
    fxProgramNr = (uint8_t)atoi(argv[3]);
    for (uint8_t c=4;c<argc;c++)
    {
        params[c-4]=(uint16_t)atoi(argv[c]);
    }
    currentProgram = PiPicoFX::loadProgram(fxProgramNr);
    jsonStringPtr += sprintf(jsonBfr+jsonStringPtr,"{\"inputFile\": \"%s\",",filenameIn);
    jsonStringPtr += sprintf(jsonBfr+jsonStringPtr,"\"FxProgram\": \"%s\",\r\n",currentProgram->getName());
    size_t fnamelen = strlen(filenameIn);
    strcpy(filenameOut+fnamelen-4,"_proc.wav");
    openWavFile(filenameIn,&wavFileIn);
    createWavFile(filenameOut,&wavFileOut,wavFileIn.dataSize+TAIL_TIME);
    jsonStringPtr += sprintf(jsonBfr+jsonStringPtr,"\"parameters\":[");
    for (uint8_t c=0;c<currentProgram->getParameterCount();c++)
    {
        if (currentProgram->getParameter(c)->getControl() == 0)
        {
            zeroString(paramDisplay,64);
            currentProgram->getParameter(c)->parameterCallback(params[0]);
            currentProgram->getParameter(c)->parameterDisplay(paramDisplay);
            jsonStringPtr += sprintf(jsonBfr+jsonStringPtr,"{\"nr\": 0,\"value\": \"%s\"}, \r\n",paramDisplay);
        }
        else if (currentProgram->getParameter(c)->getControl() == 1)
        {
            zeroString(paramDisplay,64);
            currentProgram->getParameter(c)->parameterCallback(params[1]);
            currentProgram->getParameter(c)->parameterDisplay(paramDisplay);
            jsonStringPtr += sprintf(jsonBfr+jsonStringPtr,"{\"nr\": 1, \"value\": \"%s\"}, \r\n",paramDisplay);
        }
        else if (currentProgram->getParameter(c)->getControl() == 2)
        {
            zeroString(paramDisplay,64);           
            currentProgram->getParameter(c)->parameterCallback(params[2]);
            currentProgram->getParameter(c)->parameterDisplay(paramDisplay);
            jsonStringPtr += sprintf(jsonBfr+jsonStringPtr,"{\"nr\": 2, \"value\": \"%s\"}, \r\n",paramDisplay);
        }
        else
        {
            zeroString(paramDisplay,64);           
            currentProgram->getParameter(c)->parameterCallback(params[c]);
            currentProgram->getParameter(c)->parameterDisplay(paramDisplay);
            jsonStringPtr += sprintf(jsonBfr+jsonStringPtr,"{\"nr\": %d, \"value\": \"%s\"}, \r\n",c+1,paramDisplay);
        }

    }
    currentProgram->switchOn();
    if(currentProgram->getParameterCount() > 0)
    {
        jsonStringPtr -=4;
        *(jsonBfr + jsonStringPtr) = 0;
    }
    jsonStringPtr += sprintf(jsonBfr+jsonStringPtr,"]}");

    const uint32_t lengthInSamples = ((wavFileIn.dataSize/(wavFileIn.wavFormat.wBitsPerSample >> 3)*wavFileIn.wavFormat.wChannels));
    while(byteCnt < lengthInSamples +TAIL_TIME/(wavFileIn.wavFormat.wBitsPerSample >> 3))
    {
        if (byteCnt < lengthInSamples)
        {
            sample[0]= wavFileIn.data[byteCnt];
            if (wavFileIn.wavFormat.wChannels == 2)
            {
                sample[1] = wavFileIn.data[byteCnt+1];
            }
        }
        else
        {
            sample[0]=0.0f;
        }
        dataOut = currentProgram->processSample(((float)sample[0])/32768.0f);
        dataOut = clip(dataOut,getAudioStatePtr());
        if (wavFileIn.wavFormat.wChannels==2)
        {
            wavFileOut.data[byteCnt >> 1]=(int16_t)(dataOut*32767.0);
            byteCnt+=2;
        }
        else
        {
            wavFileOut.data[byteCnt++]=(int16_t)(dataOut*32767.0);
        }
    }
    writeWavFile(&wavFileOut);
    fclose(wavFileIn.filePointer);
    fclose(wavFileOut.filePointer);
    return jsonStringPtr;
}

uint32_t getOverview(int args,char ** argv,char * jsonBfr)
{
    uint32_t jsonStringPtr=0;
    FxProgram * fxProgram;
    jsonStringPtr += sprintf(jsonBfr+jsonStringPtr,"{\"programs\": [");
    for (uint16_t c=0;c<N_FX_PROGRAMS;c++)
    {
        fxProgram = PiPicoFX::loadProgram(c);
        jsonStringPtr += sprintf(jsonBfr+jsonStringPtr,"\"%s\", ",fxProgram->getName());
        delete fxProgram;
    }
    jsonStringPtr -= 2;
    *(jsonBfr + jsonStringPtr) = 0;
    jsonStringPtr += sprintf(jsonBfr+jsonStringPtr,"]}");
    return jsonStringPtr;
}

uint32_t getParamNames(int args,char ** argv,char * jsonBfr)
{
    uint32_t jsonStringPtr=0;
    uint8_t fxProgramNr;
    FxProgram * fxProgram;
    jsonStringPtr += sprintf(jsonBfr+jsonStringPtr,"{\"parameterNames\":[");

    fxProgramNr = (uint8_t)atoi(argv[2]);
    fxProgram = PiPicoFX::loadProgram(fxProgramNr);
    for (uint8_t c=0;c<fxProgram->getParameterCount();c++)
    {
        jsonStringPtr += sprintf(jsonBfr+jsonStringPtr,"{\"name\": \"%s\",\"control\": %d}, \r\n",fxProgram->getParameter(c)->getParameterName(),fxProgram->getParameter(c)->getControl());

    }
    if(fxProgram->getParameterCount() > 0)
    {
        jsonStringPtr -= 4;
        *(jsonBfr + jsonStringPtr) = 0;
    }
    jsonStringPtr += sprintf(jsonBfr+jsonStringPtr,"]}");
    return jsonStringPtr;
}

uint32_t getParamValues(int argc,char ** argv,char * jsonBfr)
{

    uint32_t byteCnt=0;
    uint8_t fxProgramNr;
    int16_t dataOut;
    size_t fnameLength;
    char paramDisplay[64];
    uint16_t params[64]=    {0,0,0,0,0,0,0,0,
                            0,0,0,0,0,0,0,0,
                            0,0,0,0,0,0,0,0,
                            0,0,0,0,0,0,0,0,
                            0,0,0,0,0,0,0,0,
                            0,0,0,0,0,0,0,0,
                            0,0,0,0,0,0,0,0,
                            0,0,0,0,0,0,0,0};
    int16_t sample[2];
    uint32_t jsonStringPtr=0;
    FxProgram * fxProgram;
    fxProgramNr = (uint8_t)atoi(argv[2]);
    for (uint8_t c=3;c<argc;c++)
    {
        params[c-3]=(uint16_t)atoi(argv[c]);
    }
    jsonStringPtr += sprintf(jsonBfr+jsonStringPtr,"{\"parameterValues\":[");
    fxProgram = PiPicoFX::loadProgram(fxProgramNr);

    for (uint8_t c=0;c<fxProgram->getParameterCount();c++)
    {
        zeroString(paramDisplay,64);
        fxProgram->getParameter(c)->parameterCallback(params[c]);
        fxProgram->getParameter(c)->parameterDisplay(paramDisplay);
        jsonStringPtr += sprintf(jsonBfr+jsonStringPtr,"{\"control\": %d,\"name\": \"%s\",\"value\": \"%s\"}, \r\n",
            fxProgram->getParameter(c)->getControl(),
            fxProgram->getParameter(c)->getParameterName(),
            paramDisplay);
    }
    if (fxProgram->getParameterCount() > 0)
    {
        jsonStringPtr -= 4;
        *(jsonBfr + jsonStringPtr) = 0;
    }
    jsonStringPtr += sprintf(jsonBfr+jsonStringPtr,"]}");
    return jsonStringPtr;
}

int main(int argc, char ** argv)
{
    
    char jsonBfr[2048];
    uint32_t charsWritten;
    initDelayMemoryHandler();
    if (argc == 1)
    {
        printf("Offline FxProgram Processor v0.1\r\n\r\n");
        printf("Commands\r\n\r\n");
        printf("Processes Wave file, returns the parameters used in a json structure\r\n");
        printf("\t-p <fileIn.wav> <fxProgram> <param1> <param2> <param3> (<param4>..)\r\n\r\n");
        printf("Returns an Overview of all Fx Programs in a json structure\r\n");
        printf("\t-p\r\n\r\n");
        printf("Returns the Parameter Names and Control assignment from prgram Nr <fxProgram>\r\n");
        printf("\t-n <fxProgram>\r\n\r\n");
        printf("Returns the Parameter Values as displayed for program <fxProgram> and the parameters <param1>, <param2> etc..\r\n");
        printf("all parameter values must be defined\r\n");
        printf("\t-v <fxProgram> <param1> <param2> <param3> (<param4>..)\r\n\r\n");
    }
    else
    {
        if (strcmp(argv[1],"-p")==0) // -p: process
        {
            charsWritten = processOffline(argc,argv,jsonBfr);
        }
        else if (strcmp(argv[1],"-o")==0) // -o: overview, return all programs in a json array
        {
            charsWritten = getOverview(argc,argv,jsonBfr);
        }
        else if (strcmp(argv[1],"-n")==0) // get parameter names and controls
        {
            charsWritten = getParamNames(argc,argv,jsonBfr);
        }
        else if (strcmp(argv[1],"-v")==0) // get parameter values
        {
            charsWritten = getParamValues(argc,argv,jsonBfr);
        }
        printf("%s",jsonBfr);
    }
        
    
}

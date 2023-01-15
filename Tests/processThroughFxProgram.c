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
#define TAIL_TIME 48000

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



uint32_t processOffline(int argc,char ** argv,char * jsonBfr)
{
    uint32_t byteCnt=0;
    char filenameOut[256];
    char filenameIn[256];
    uint8_t fxProgramNr;
    int16_t dataOut;
    size_t fnameLength;
    WavFileType wavFileIn;
    WavFileType wavFileOut;
    char paramDisplay[64];
    uint16_t params[8]={0,0,0,0,0,0,0,0};
    int16_t sample[2];
    uint32_t jsonStringPtr=0;

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
    jsonStringPtr += sprintf(jsonBfr+jsonStringPtr,"{\"inputFile\": \"%s\",",filenameIn);
    jsonStringPtr += sprintf(jsonBfr+jsonStringPtr,"\"FxProgram\": \"%s\",\r\n",fxPrograms[fxProgramNr]->name);
    size_t fnamelen = strlen(filenameIn);
    strcpy(filenameOut+fnamelen-4,"_proc.wav");
    openWavFile(filenameIn,&wavFileIn);
    createWavFile(filenameOut,&wavFileOut,wavFileIn.dataSize+TAIL_TIME);
    for (uint16_t c=0;c<N_FX_PROGRAMS;c++)
    {
        if (fxPrograms[c]->setup != 0)
        {
            fxPrograms[c]->setup(fxPrograms[c]->data);
        }
    }
    jsonStringPtr += sprintf(jsonBfr+jsonStringPtr,"\"parameters\":[");
    for (uint8_t c=0;c<fxPrograms[fxProgramNr]->nParameters;c++)
    {
        if (fxPrograms[fxProgramNr]->parameters[c].control == 0)
        {
            zeroString(paramDisplay,64);
            fxPrograms[fxProgramNr]->parameters[c].setParameter(params[0],fxPrograms[fxProgramNr]->data);
            fxPrograms[fxProgramNr]->parameters[c].getParameterDisplay(fxPrograms[fxProgramNr]->data,paramDisplay);
            jsonStringPtr += sprintf(jsonBfr+jsonStringPtr,"{\"nr\": 0,\"value\": \"%s\"}, \r\n",paramDisplay);
        }
        else if (fxPrograms[fxProgramNr]->parameters[c].control == 1)
        {
            zeroString(paramDisplay,64);
            fxPrograms[fxProgramNr]->parameters[c].setParameter(params[1],fxPrograms[fxProgramNr]->data);
            fxPrograms[fxProgramNr]->parameters[c].getParameterDisplay(fxPrograms[fxProgramNr]->data,paramDisplay);
            jsonStringPtr += sprintf(jsonBfr+jsonStringPtr,"{\"nr\": 1, \"value\": \"%s\"}, \r\n",paramDisplay);
        }
        else if (fxPrograms[fxProgramNr]->parameters[c].control == 2)
        {
            zeroString(paramDisplay,64);           
            fxPrograms[fxProgramNr]->parameters[c].setParameter(params[2],fxPrograms[fxProgramNr]->data);
            fxPrograms[fxProgramNr]->parameters[c].getParameterDisplay(fxPrograms[fxProgramNr]->data,paramDisplay);
            jsonStringPtr += sprintf(jsonBfr+jsonStringPtr,"{\"nr\": 2, \"value\": \"%s\"}, \r\n",paramDisplay);
        }
        else
        {
            zeroString(paramDisplay,64);           
            fxPrograms[fxProgramNr]->parameters[c].setParameter(params[c],fxPrograms[fxProgramNr]->data);
            fxPrograms[fxProgramNr]->parameters[c].getParameterDisplay(fxPrograms[fxProgramNr]->data,paramDisplay);
            jsonStringPtr += sprintf(jsonBfr+jsonStringPtr,"{\"nr\": %d, \"value\": \"%s\"}, \r\n",c+1,paramDisplay);
        }

    }
    if(fxPrograms[fxProgramNr]->nParameters > 0)
    {
        jsonStringPtr -=4;
        *(jsonBfr + jsonStringPtr) = 0;
    }
    jsonStringPtr += sprintf(jsonBfr+jsonStringPtr,"]}");

    while(byteCnt < wavFileIn.dataSize+TAIL_TIME)
    {
        if (byteCnt < wavFileIn.dataSize)
        {
            sample[0] = wavFileIn.data[byteCnt>>1];
            if (wavFileIn.wavFormat.wChannels == 2)
            {
                sample[1] = wavFileIn.data[(byteCnt>>1)+1];
            }
        }
        else
        {
            sample[0]=0;
        }
        dataOut = fxPrograms[fxProgramNr]->processSample(sample[0],fxPrograms[fxProgramNr]->data);
        if (wavFileIn.wavFormat.wChannels==2)
        {
            wavFileOut.data[byteCnt >> 2]=dataOut;
            byteCnt+=4;
        }
        else
        {
            wavFileOut.data[byteCnt>>1]=dataOut;
            byteCnt+=2;
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

    jsonStringPtr += sprintf(jsonBfr+jsonStringPtr,"{\"programs\": [");
    for (uint16_t c=0;c<N_FX_PROGRAMS;c++)
    {
        if (fxPrograms[c]->setup != 0)
        {
            fxPrograms[c]->setup(fxPrograms[c]->data);
        }
        jsonStringPtr += sprintf(jsonBfr+jsonStringPtr,"\"%s\", ",fxPrograms[c]->name);
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
    jsonStringPtr += sprintf(jsonBfr+jsonStringPtr,"{\"parameterNames\":[");
    for (uint16_t c=0;c<N_FX_PROGRAMS;c++)
    {
        if (fxPrograms[c]->setup != 0)
        {
            fxPrograms[c]->setup(fxPrograms[c]->data);
        }
        fxPrograms[c]->name;
    }
    fxProgramNr = (uint8_t)atoi(argv[2]);
    for (uint8_t c=0;c<fxPrograms[fxProgramNr]->nParameters;c++)
    {
        jsonStringPtr += sprintf(jsonBfr+jsonStringPtr,"{\"name\": \"%s\",\"control\": %d}, \r\n",fxPrograms[fxProgramNr]->parameters[c].name,fxPrograms[fxProgramNr]->parameters[c].control);

    }
    if(fxPrograms[fxProgramNr]->nParameters > 0)
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
    char filenameOut[256];
    char filenameIn[256];
    uint8_t fxProgramNr;
    int16_t dataOut;
    size_t fnameLength;
    WavFileType wavFileIn;
    WavFileType wavFileOut;
    char paramDisplay[64];
    uint16_t params[8]={0,0,0,0,0,0,0,0};
    int16_t sample[2];
    uint32_t jsonStringPtr=0;

    strcpy(filenameIn,argv[2]);
    strcpy(filenameOut,argv[2]);
    fnameLength = strlen(filenameOut);
    filenameOut[fnameLength-4]=0;
    strcat(filenameOut,"_proc.wav");
    fxProgramNr = (uint8_t)atoi(argv[2]);
    for (uint8_t c=3;c<argc;c++)
    {
        params[c-3]=(uint16_t)atoi(argv[c]);
    }
    jsonStringPtr += sprintf(jsonBfr+jsonStringPtr,"{\"parameterValues\":[");

    for (uint16_t c=0;c<N_FX_PROGRAMS;c++)
    {
        if (fxPrograms[c]->setup != 0)
        {
            fxPrograms[c]->setup(fxPrograms[c]->data);
        }
    }
    for (uint8_t c=0;c<fxPrograms[fxProgramNr]->nParameters;c++)
    {
        zeroString(paramDisplay,64);
        fxPrograms[fxProgramNr]->parameters[c].setParameter(params[c],fxPrograms[fxProgramNr]->data);
        fxPrograms[fxProgramNr]->parameters[c].getParameterDisplay(fxPrograms[fxProgramNr]->data,paramDisplay);
        jsonStringPtr += sprintf(jsonBfr+jsonStringPtr,"{\"control\": %d,\"name\": \"%s\",\"value\": \"%s\"}, \r\n",
            fxPrograms[fxProgramNr]->parameters[c].control,
            fxPrograms[fxProgramNr]->parameters[c].name,
            paramDisplay);
    }
    if (fxPrograms[fxProgramNr]->nParameters > 0)
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
#include <stdint.h>
#include "audio/fxprogram/fxProgram.h"
#include "stringFunctions.h"
#include "romfunc.h"

static int16_t fxProgramProcessSample(int16_t sampleIn,void*data)
{
    FxProgram8DataType * pData=(FxProgram8DataType*)data;
    return compressorProcessSample(sampleIn,&pData->compressor);
}

static void fxProgramP1Callback(uint16_t val,void*data) 
{
    FxProgram8DataType * pData=(FxProgram8DataType*)data;
    pData->compressor.attack = val << 3;
}

static void fxProgramP1Display(void*data,char*res)
{
    FxProgram8DataType * pData=(FxProgram8DataType*)data;
    float attackFloat;
    float dval;
    int32_t ival;
    int16_t i16val;
    uint8_t c=0;
    attackFloat = int2float(pData->compressor.attack)+ 0.1f;
    dval = 682655.744f/attackFloat;
    if (dval > 1000.0f)
    {
        dval = dval/1000.0f;
        ival = float2int(dval);
        i16val = (int16_t)ival;
        Int16ToChar(i16val,res);

        while (*(res+c) != 0)
        {
            c++;
        }
        *(res + c++)=' ';
        *(res + c++)='m';
        *(res + c++)='s';
    }
    else
    {
        ival = float2int(dval);
        i16val = (int16_t)ival;
        Int16ToChar(i16val,res);
        while (*(res+c) != 0)
        {
            c++;
        }
        *(res + c++)=' ';
        *(res + c++)='u';
        *(res + c++)='s';
    }
    for(uint8_t c2=c;c2<PARAMETER_NAME_MAXLEN;c2++)
    {
        *(res+c2)=' ';
    }
}

static void fxProgramP2Callback(uint16_t val,void*data) 
{
    FxProgram8DataType * pData=(FxProgram8DataType*)data;
    pData->compressor.release = val << 3;
}

static void fxProgramP2Display(void*data,char*res)
{
    FxProgram8DataType * pData=(FxProgram8DataType*)data;
    float releaseFloat;
    float dval;
    int32_t ival;
    int16_t i16val;
    uint8_t c=0;
    releaseFloat = int2float(pData->compressor.release)+ 0.1f;
    dval = 682655.744f/releaseFloat;
    if (dval > 1000.0f)
    {
        dval = dval/1000.0f;
        ival = float2int(dval);
        i16val = (int16_t)ival;
        Int16ToChar(i16val,res);

        while (*(res+c) != 0)
        {
            c++;
        }
        *(res + c++)=' ';
        *(res + c++)='m';
        *(res + c++)='s';
    }
    else
    {
        ival = float2int(dval);
        i16val = (int16_t)ival;
        Int16ToChar(i16val,res);
        while (*(res+c) != 0)
        {
            c++;
        }
        *(res + c++)=' ';
        *(res + c++)='u';
        *(res + c++)='s';
    }
    for(uint8_t c2=c;c2<PARAMETER_NAME_MAXLEN;c2++)
    {
        *(res+c2)=' ';
    }
}

static void fxProgramP3Callback(uint16_t val,void*data) 
{
    FxProgram8DataType * pData=(FxProgram8DataType*)data;
    uint16_t enumVal = val >> 9;
    if (enumVal > 4)
    {
        enumVal=4;
    }
    pData->compressor.gainFunction.gainReduction = enumVal;
}

static void fxProgramP3Display(void*data,char*res)
{
    FxProgram8DataType * pData=(FxProgram8DataType*)data;
    const char* dstrings[5];
    dstrings[0]="1:2            ";
    dstrings[1]="1:4            ";
    dstrings[2]="1:8            ";
    dstrings[3]="1:16           ";
    dstrings[4]="1:Inf          ";

    for(uint8_t c=0;c<16;c++)
    {
        *(res+c)=*(dstrings[pData->compressor.gainFunction.gainReduction] + c);
    }
}

static void fxProgramP4Callback(uint16_t val,void*data) 
{
    FxProgram8DataType * pData=(FxProgram8DataType*)data;
    pData->compressor.gainFunction.threshhold = val << 3;
}

static void fxProgramP4Display(void*data,char*res)
{
    FxProgram8DataType * pData=(FxProgram8DataType*)data;
    Int16ToChar(pData->compressor.gainFunction.threshhold,res);
}


FxProgram8DataType fxProgram8Data =
{
    .compressor.attack = 400,
    .compressor.release=400,
    .compressor.currentAvg=0,
    .compressor.gainFunction.gainReduction=0,
    .compressor.gainFunction.threshhold=32767
};


FxProgramType fxProgram8 = {
    .data = (void*)&fxProgram8Data,
    .name = "Compressor             ",
    .nParameters=4,
    .parameters = {
        {
            .name="Attack         ",
            .control=0,
            .getParameterDisplay=&fxProgramP1Display,
            .setParameter=&fxProgramP1Callback,
            .increment=64,
            .rawValue=0,
        },
        {
            .name="Release        ",
            .control=1,
            .getParameterDisplay=&fxProgramP2Display,
            .setParameter=&fxProgramP2Callback,
            .increment=64,
            .rawValue=0,
        },
        {
            .name="Ratio          ",
            .control=2,
            .getParameterDisplay=&fxProgramP3Display,
            .setParameter=&fxProgramP3Callback,
            .increment=512,
            .rawValue=0
        },
        {
            .name="Threshhold     ",
            .control=0xFF,
            .getParameterDisplay=&fxProgramP4Display,
            .setParameter=&fxProgramP4Callback,
            .increment=64,
            .rawValue=0
        }
    },
    .processSample=&fxProgramProcessSample,
    .setup=0
};

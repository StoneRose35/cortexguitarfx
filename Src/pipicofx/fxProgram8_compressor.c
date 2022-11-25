#include <stdint.h>
#include "audio/fxprogram/fxProgram.h"
#include "stringFunctions.h"
#include "romfunc.h"

static float fxProgramProcessSample(float sampleIn,void*data)
{
    FxProgram8DataType * pData=(FxProgram8DataType*)data;
    return compressorProcessSample(sampleIn,&pData->compressor);
}

static void fxProgramP1Callback(uint16_t val,void*data) 
{
    FxProgram8DataType * pData=(FxProgram8DataType*)data;
    setAttack(val << 3,&pData->compressor);
}

static void fxProgramP1Display(void*data,char*res)
{
    FxProgram8DataType * pData=(FxProgram8DataType*)data;
    float releaseFloat;
    int16_t i16val;
    uint8_t c=0;
    releaseFloat = 20.833f/pData->compressor.attack; // attack in microseconds
    if (releaseFloat > 1000.0f)
    {
        releaseFloat = releaseFloat/1000.0f;
        i16val = (int16_t)(int32_t)releaseFloat;
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
        i16val = (int16_t)(int32_t)releaseFloat;
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
    setRelease(val << 3,&pData->compressor);
}

static void fxProgramP2Display(void*data,char*res)
{
    FxProgram8DataType * pData=(FxProgram8DataType*)data;
    float releaseFloat;
    int16_t i16val;
    uint8_t c=0;
    releaseFloat = 20.833f/pData->compressor.release; // release in microseconds
    if (releaseFloat > 1000.0f)
    {
        releaseFloat = releaseFloat/1000.0f;
        i16val = (int16_t)(int32_t)releaseFloat;
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
        i16val = (int16_t)(int32_t)releaseFloat;
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
    switch (enumVal)
    {
        case 0:
            pData->compressor.gainFunction.gainReduction = 2.0f;
            break;
        case 1:
            pData->compressor.gainFunction.gainReduction = 4.0f;
            break;
        case 2:
            pData->compressor.gainFunction.gainReduction = 8.0f;
            break;
        case 3:
            pData->compressor.gainFunction.gainReduction = 16.0f;
            break;
        default:
            pData->compressor.gainFunction.gainReduction = 1000.0f;
            break;
    }
}

static void fxProgramP3Display(void*data,char*res)
{
    FxProgram8DataType * pData=(FxProgram8DataType*)data;
    uint16_t c=0,cres=0;
    const char* infDisplay="Inf          ";
    int32_t gainReductionInt = (int32_t)pData->compressor.gainFunction.gainReduction;
    *(res+cres++)='1';
    *(res+cres++)=':';
    if (gainReductionInt > 16)
    {
        while(*(infDisplay+c) !=0)
        {
            *(res+cres++)=*(infDisplay+c++);
        }
    }
    else
    {
        cres +=  decimalInt16ToChar((int16_t)(gainReductionInt*10),res+cres,1);
        for (c=0;c<16-cres;c++)
        {
            *(res+cres++)=' ';
        }
    }
}

static void fxProgramP4Callback(uint16_t val,void*data) 
{
    FxProgram8DataType * pData=(FxProgram8DataType*)data;
    pData->compressor.gainFunction.threshhold = ((float)val)/4095.0f*0.99f+0.01f;
}

static void fxProgramP4Display(void*data,char*res)
{
    FxProgram8DataType * pData=(FxProgram8DataType*)data;
    decimalInt16ToChar((int16_t)(int32_t)(pData->compressor.gainFunction.threshhold*100.0f),res,2);
}


FxProgram8DataType fxProgram8Data =
{
    .compressor.attack = 400,
    .compressor.release=400,
    .compressor.currentAvg=0.0f,
    .compressor.gainFunction.gainReduction=2.0f,
    .compressor.gainFunction.threshhold=0.8f
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

#include <stdint.h>
#include "audio/fxprogram/fxProgram.h"
#include "stringFunctions.h"
#include "romfunc.h"
#include "fastExpLog.h"

static int16_t fxProgramProcessSample(int16_t sampleIn,void*data)
{
    FxProgram8DataType * pData=(FxProgram8DataType*)data;
    sampleIn = compressorProcessSample(sampleIn,&pData->compressor);
    sampleIn = gainStageProcessSample(sampleIn,&pData->makeupGain);
    return sampleIn;
}

static void fxProgramP1Callback(uint16_t val,void*data) 
{
    FxProgram8DataType * pData=(FxProgram8DataType*)data;
    pData->compressor.attack = (val>>4)+1;
}

static void fxProgramP1Display(void*data,char*res)
{
    FxProgram8DataType * pData=(FxProgram8DataType*)data;
    float attackFloat;
    float dval;
    int32_t ival;
    int16_t i16val;
    attackFloat = int2float(pData->compressor.attack);
    dval = 682655.744f/attackFloat;
    if (dval > 1000.0f)
    {
        dval = dval/1000.0f;
        ival = float2int(dval);
        i16val = (int16_t)ival;
        Int16ToChar(i16val,res);
        appendToString(res," ms");
    }
    else
    {
        ival = float2int(dval);
        i16val = (int16_t)ival;
        Int16ToChar(i16val,res);
        appendToString(res," us");
    }
}

static void fxProgramP2Callback(uint16_t val,void*data) 
{
    FxProgram8DataType * pData=(FxProgram8DataType*)data;
    pData->compressor.release = (val>>4)+1;
}

static void fxProgramP2Display(void*data,char*res)
{
    FxProgram8DataType * pData=(FxProgram8DataType*)data;
    float releaseFloat;
    float dval;
    int32_t ival;
    int16_t i16val;
    releaseFloat = int2float(pData->compressor.release)+ 0.1f;
    dval = 682655.744f/releaseFloat;
    if (dval > 1000.0f)
    {
        dval = dval/1000.0f;
        ival = float2int(dval);
        i16val = (int16_t)ival;
        Int16ToChar(i16val,res);
        appendToString(res," ms");
    }
    else
    {
        ival = float2int(dval);
        i16val = (int16_t)ival;
        Int16ToChar(i16val,res);
        appendToString(res," us");
    }

}

static void fxProgramP3Callback(uint16_t val,void*data) 
{
    FxProgram8DataType * pData=(FxProgram8DataType*)data;
    uint16_t enumVal = (val >> 9) + 1;
    if (enumVal > 5)
    {
        enumVal=5;
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
        *(res+c)=*(dstrings[pData->compressor.gainFunction.gainReduction-1] + c);
    }
}

static void fxProgramP4Callback(uint16_t val,void*data) 
{
    FxProgram8DataType * pData=(FxProgram8DataType*)data;
    pData->compressor.gainFunction.threshhold = val << 3;
}

static void fxProgramP4Display(void*data,char*res)
{
    int16_t dbval;
    FxProgram8DataType * pData=(FxProgram8DataType*)data;
    dbval = asDb(pData->compressor.gainFunction.threshhold);
    decimalInt16ToChar(dbval,res,1);
    appendToString(res," dB");
}


static void fxProgramP5Callback(uint16_t val,void*data) 
{
    FxProgram8DataType * pData=(FxProgram8DataType*)data;
    pData->makeupGain.gain = val + 256;
}

static void fxProgramP5Display(void*data,char*res)
{
    FxProgram8DataType * pData=(FxProgram8DataType*)data;
    uint32_t dval;
    dval = pData->makeupGain.gain*100;
    dval >>= 8;
    decimalInt16ToChar((int16_t)dval,res,2);
}

FxProgram8DataType fxProgram8Data =
{
    .compressor.attack = 400,
    .compressor.release=400,
    .compressor.currentAvg=0,
    .compressor.gainFunction.gainReduction=0,
    .compressor.gainFunction.threshhold=32767,
    .makeupGain.gain=0x100,
    .makeupGain.offset=0
};


FxProgramType fxProgram8 = {
    .data = (void*)&fxProgram8Data,
    .name = "Compressor             ",
    .nParameters=5,
    .parameters = {
        {
            .name="Threshhold     ",
            .control=0x0,
            .getParameterDisplay=&fxProgramP4Display,
            .setParameter=&fxProgramP4Callback,
            .increment=64,
            .rawValue=0
        },
        {
            .name="Ratio          ",
            .control=1,
            .getParameterDisplay=&fxProgramP3Display,
            .setParameter=&fxProgramP3Callback,
            .increment=512,
            .rawValue=0
        },
        {
            .name="Makeup Gain    ",
            .control=2,
            .getParameterDisplay=&fxProgramP5Display,
            .setParameter=&fxProgramP5Callback,
            .increment=64,
            .rawValue=0,
        },
        {
            .name="Attack         ",
            .control=0xff,
            .getParameterDisplay=&fxProgramP1Display,
            .setParameter=&fxProgramP1Callback,
            .increment=64,
            .rawValue=0,
        },
        {
            .name="Release        ",
            .control=0xff,
            .getParameterDisplay=&fxProgramP2Display,
            .setParameter=&fxProgramP2Callback,
            .increment=64,
            .rawValue=0,
        },


    },
    .processSample=&fxProgramProcessSample,
    .setup=0
};

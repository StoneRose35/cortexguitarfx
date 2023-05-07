#include <stdint.h>
#include "pipicofx/fxPrograms.h"
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
    pData->compressor.avgLowpass.alphaRising = (1 << 15) - 2 - (val >> 6);
    fxProgram8.parameters[0].rawValue = val;
}

static void fxProgramP1Display(void*data,char*res)
{
    FxProgram8DataType * pData=(FxProgram8DataType*)data;
    float attackFloat;
    float t60;
    int32_t ival;
    int16_t i16val;
    attackFloat = int2float(pData->compressor.avgLowpass.alphaRising)/32767.0f;
    t60=-0.143911568f/fln(attackFloat); // -3*ln(10)/(ln(attack)*f_sample)*1000., result in t60 in ms
    ival = float2int(t60);
    i16val = (int16_t)ival;
    Int16ToChar(i16val,res);
    appendToString(res," ms");
}

static void fxProgramP2Callback(uint16_t val,void*data) 
{
    FxProgram8DataType * pData=(FxProgram8DataType*)data;
    pData->compressor.avgLowpass.alphaFalling = (1 << 15) - 2 - (val >> 6);
    fxProgram8.parameters[1].rawValue = val;
}

static void fxProgramP2Display(void*data,char*res)
{
    FxProgram8DataType * pData=(FxProgram8DataType*)data;
    float releaseFloat;
    float t60;
    int32_t ival;
    int16_t i16val;
    releaseFloat = int2float(pData->compressor.avgLowpass.alphaFalling)/32767.0f;
    t60=-0.143911568f/fln(releaseFloat); // -3*ln(10)/(ln(release)*f_sample)*1000., result in t60 in ms
    ival = float2int(t60);
    i16val = (int16_t)ival;
    Int16ToChar(i16val,res);
    appendToString(res," ms");
}

static void fxProgramP3Callback(uint16_t val,void*data) 
{
    FxProgram8DataType * pData=(FxProgram8DataType*)data;
    uint16_t enumVal = (val >> 9) + 1;
    if (enumVal > 5)
    {
        enumVal=5;
    }
    fxProgram8.parameters[2].rawValue = val;
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
    fxProgram8.parameters[3].rawValue = val;
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
    fxProgram8.parameters[4].rawValue = val;
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
    .compressor.avgLowpass.alphaFalling = 10,
    .compressor.avgLowpass.alphaRising = 10,
    .compressor.avgLowpass.oldVal = 0,
    .compressor.avgLowpass.oldXVal = 0,
    .compressor.currentAvg = 0,
    .compressor.gainFunction.gainReduction = 0,
    .compressor.gainFunction.threshhold = 32767,
    .makeupGain.gain = 0x100,
    .makeupGain.offset = 0
};

static void fxProgramReset(void*data)
{
    FxProgram8DataType * pData=(FxProgram8DataType*)data;
    compressorReset(&pData->compressor);
} 

FxProgramType fxProgram8 = {
    .data = (void*)&fxProgram8Data,
    .name = "Compressor",
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
    .reset = &fxProgramReset,
    .setup=0
};

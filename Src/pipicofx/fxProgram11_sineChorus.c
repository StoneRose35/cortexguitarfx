#include <stdint.h>
#include "pipicofx/fxPrograms.h"
#include "stringFunctions.h"

#ifndef FLOAT_AUDIO
static int16_t fxProgramprocessSample(int16_t sampleIn,void*data)
{
    FxProgram11DataType* pData = (FxProgram11DataType*)data;
    sampleIn >>= 1;
    return sineChorusInterpolatedProcessSample(sampleIn,&pData->sineChorus);
}
#else
static float fxProgramprocessSample(float sampleIn,void*data)
{
    FxProgram11DataType* pData = (FxProgram11DataType*)data;
    return sineChorusProcessSample(sampleIn,&pData->sineChorus);
}
#endif

static void fxProgramParam1Callback(uint16_t val,void*data) // frequency
{
    FxProgram11DataType* pData = (FxProgram11DataType*)data;
    // map 0 - 4095 to 1 1000
    val = ((val*250) >> 10) + 1;
    sineChorusSetFrequency(val,&pData->sineChorus);
}

static void fxProgramParam1Display(void*data,char*res)
{
    FxProgram11DataType* pData = (FxProgram11DataType*)data;
    decimalInt16ToChar(pData->sineChorus.frequency,res,2);
    appendToString(res," Hz");
}

static void fxProgramParam2Callback(uint16_t val,void*data) // depth
{
    FxProgram11DataType* pData = (FxProgram11DataType*)data;
    // map to 0 to 255
    val >>= 4;
    pData->sineChorus.depth = (uint8_t)val;
}

static void fxProgramParam2Display(void*data,char*res)
{
    int16_t dVal;
    FxProgram11DataType* pData = (FxProgram11DataType*)data;
    dVal = pData->sineChorus.depth; 
    Int16ToChar(dVal,res);
}

static void fxProgramParam3Callback(uint16_t val,void*data) // mix
{
    FxProgram11DataType* pData = (FxProgram11DataType*)data;
    pData->sineChorus.mix = val << 3;
}

static void fxProgramParam3Display(void*data,char*res)
{
    FxProgram11DataType* pData = (FxProgram11DataType*)data;
    Int16ToChar(pData->sineChorus.mix/328,res);
    appendToString(res,"%");
}

static void fxProgramParam4Callback(uint16_t val,void*data)
{
    FxProgram11DataType* pData = (FxProgram11DataType*)data;
    pData->sineChorus.offset = 49 + (val >> 1);
}

static void fxProgramParam4Display(void*data,char*res)
{
    FxProgram11DataType* pData = (FxProgram11DataType*)data;
    uint16_t msValue;
    msValue = (uint16_t)(((uint32_t)pData->sineChorus.offset*21) >> 10);
    UInt16ToChar(msValue,res);
    appendToString(res, " ms");
}

static void fxProgramParam5Callback(uint16_t val,void*data)
{
    FxProgram11DataType* pData = (FxProgram11DataType*)data;
    pData->sineChorus.feedback = val << 3;

}

static void fxProgramParam5Display(void*data,char*res)
{
    FxProgram11DataType* pData = (FxProgram11DataType*)data;
    Int16ToChar(pData->sineChorus.feedback/328,res);
    appendToString(res,"%");
}

static void fxProgramSetup(void*data)
{
    FxProgram11DataType* pData = (FxProgram11DataType*)data;
    initSineChorus(&pData->sineChorus,0);
}

FxProgram11DataType fxProgram11data = {
    .sineChorus = {
        .mix = 16384,
        .frequency = 500,
        .depth = 10,
        .feedback = 0,
        .offset = 49
    }
};

FxProgramType fxProgram11 = {
    .name = "Sine Chorus",
    .nParameters=5,
    .processSample = &fxProgramprocessSample,
    .parameters = {
        {
            .name = "Frequency      ",
            .control=0,
            .increment = 64,
            .rawValue=0,
            .getParameterDisplay=&fxProgramParam1Display,
            .getParameterValue=0,
            .setParameter=&fxProgramParam1Callback
        },
        {
            .name = "Depth          ",
            .control=1,
            .increment = 64,
            .rawValue=0,
            .getParameterDisplay=&fxProgramParam2Display,
            .getParameterValue=0,
            .setParameter=&fxProgramParam2Callback
        },
        {
            .name = "Blend         ",
            .control=2,
            .increment=64,
            .rawValue=0,
            .getParameterDisplay=&fxProgramParam3Display,
            .getParameterValue=0,
            .setParameter=&fxProgramParam3Callback
        },
        {
            .name = "Offset         ",
            .control=0xFF,
            .increment=64,
            .rawValue=0,
            .getParameterDisplay=&fxProgramParam4Display,
            .getParameterValue=0,
            .setParameter=&fxProgramParam4Callback
        },
        {
            .name = "Feedback       ",
            .control=0xFF,
            .increment=64,
            .rawValue=0,
            .getParameterDisplay=&fxProgramParam5Display,
            .getParameterValue=0,
            .setParameter=&fxProgramParam5Callback
        }

    },
    .setup = &fxProgramSetup,
    .reset = 0,
    .data = (void*)&fxProgram11data
};

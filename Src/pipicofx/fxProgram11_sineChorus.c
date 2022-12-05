#include <stdint.h>
#include "audio/fxprogram/fxProgram.h"
#include "stringFunctions.h"

#ifndef FLOAT_AUDIO
static int16_t fxProgram2processSample(int16_t sampleIn,void*data)
{
    FxProgram2DataType* pData = (FxProgram2DataType*)data;
    sampleIn >>= 1;
    return simpleChorusProcessSample(sampleIn,&pData->chorusData);
}
#else
static float fxProgramprocessSample(float sampleIn,void*data)
{
    FxProgram11DataType* pData = (FxProgram11DataType*)data;
    return sineChorusInterpolatedProcessSample(sampleIn,&pData->sineChorus);
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
    pData->sineChorus.mix = ((float)val)/4095.0f;
}

static void fxProgramParam3Display(void*data,char*res)
{
    int16_t dVal;
    FxProgram11DataType* pData = (FxProgram11DataType*)data;
    dVal = (int16_t)(pData->sineChorus.mix*100.0f);
    Int16ToChar(dVal,res);
    appendToString(res,"%");
}

static void fxProgramSetup(void*data)
{
    FxProgram11DataType* pData = (FxProgram11DataType*)data;
    initSineChorus(&pData->sineChorus);
}

FxProgram11DataType fxProgram11data = {
    .sineChorus = {
        .mix = 0.5f,
        .frequency = 500,
        .depth = 10
    }
};

FxProgramType fxProgram11 = {
    .name = "Sine Chorus        ",
    .nParameters=3,
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
            .name = "Mix            ",
            .control=2,
            .increment=64,
            .rawValue=0,
            .getParameterDisplay=&fxProgramParam3Display,
            .getParameterValue=0,
            .setParameter=&fxProgramParam3Callback
        }
    },
    .setup = &fxProgramSetup,
    .data = (void*)&fxProgram11data
};

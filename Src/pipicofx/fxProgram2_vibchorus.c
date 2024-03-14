#include <stdint.h>
#include "pipicofx/fxPrograms.h"
#include "stringFunctions.h"

__attribute__ ((section (".qspi_code")))
static float fxProgram2processSample(float sampleIn,void*data)
{
    FxProgram2DataType* pData = (FxProgram2DataType*)data;
    return simpleChorusProcessSample(sampleIn,&pData->chorusData);
}

__attribute__ ((section (".qspi_code")))
static void fxProgram2Param1Callback(uint16_t val,void*data) // frequency
{
    FxProgram2DataType* pData = (FxProgram2DataType*)data;
    // map 0 - 4095 to 1 1000
    val = ((val*250) >> 10) + 1;
    simpleChorusSetFrequency(val,&pData->chorusData);
    fxProgram2.parameters[0].rawValue = val;
}

__attribute__ ((section (".qspi_code")))
static void fxProgram2Param1Display(void*data,char*res)
{
    FxProgram2DataType* pData = (FxProgram2DataType*)data;
    decimalInt16ToChar(pData->chorusData.frequency,res,2);
    for (uint8_t c=0;c<PARAMETER_NAME_MAXLEN-3;c++)
    {
        if(*(res+c)==0)
        {
            *(res+c)=' ';
            *(res+c+1)='H';
            *(res+c+2)='z';
            *(res+c+3)=(char)0;
            break;
        }
    }
}

__attribute__ ((section (".qspi_code")))
static void fxProgram2Param2Callback(uint16_t val,void*data) // depth
{
    FxProgram2DataType* pData = (FxProgram2DataType*)data;
    // map to 0 to 255
    val >>= 4;
    pData->chorusData.depth = (uint8_t)val;
    fxProgram2.parameters[1].rawValue = val;
}

__attribute__ ((section (".qspi_code")))
static void fxProgram2Param2Display(void*data,char*res)
{
    int16_t dVal;
    FxProgram2DataType* fData=(FxProgram2DataType*)data;
    dVal = (fData->chorusData.depth*100.0f); ///164;
    Int16ToChar(dVal,res);
    appendToString(res,"%");
}

__attribute__ ((section (".qspi_code")))
static void fxProgram2Param3Callback(uint16_t val,void*data) // mix
{
    FxProgram2DataType* pData = (FxProgram2DataType*)data;
    pData->chorusData.mix = ((float)val)/4095.0f;
    fxProgram2.parameters[2].rawValue = val;
}

__attribute__ ((section (".qspi_code")))
static void fxProgram2Param3Display(void*data,char*res)
{
    int16_t dVal;
    FxProgram2DataType* fData = (FxProgram2DataType*)data;
    dVal = (int16_t)(fData->chorusData.mix*100.0f);
    Int16ToChar(dVal,res);
    appendToString(res,"%");
}

__attribute__ ((section (".qspi_code")))
static void fxProgram2Setup(void*data)
{
    FxProgram2DataType* pData = (FxProgram2DataType*)data;
    initSimpleChorus(&pData->chorusData);
}

FxProgram2DataType fxProgram2data = {
    .chorusData = {
        .mix = 0.5f,
        .frequency = 500,
        .depth = 10
    }
};

FxProgramType fxProgram2 = {
    .name = "Vibrato/Chorus",
    .nParameters=3,
    .processSample = &fxProgram2processSample,
    .parameters = {
        {
            .name = "Frequency      ",
            .control=0,
            .increment = 32,
            .rawValue=0,
            .getParameterDisplay=&fxProgram2Param1Display,
            .getParameterValue=0,
            .setParameter=&fxProgram2Param1Callback
        },
        {
            .name = "Depth          ",
            .control=1,
            .increment = 32,
            .rawValue=0,
            .getParameterDisplay=&fxProgram2Param2Display,
            .getParameterValue=0,
            .setParameter=&fxProgram2Param2Callback
        },
        {
            .name = "Mix            ",
            .control=2,
            .increment=32,
            .rawValue=0,
            .getParameterDisplay=&fxProgram2Param3Display,
            .getParameterValue=0,
            .setParameter=&fxProgram2Param3Callback
        }
    },
    .setup = &fxProgram2Setup,
    .reset=0,
    .data = (void*)&fxProgram2data
};

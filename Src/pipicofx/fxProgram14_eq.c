#include "pipicofx/fxPrograms.h"
#include "stringFunctions.h"

__attribute__ ((section (".qspi_code")))
static float fxProgramProcessSample(float sampleIn,void*data)
{
    FxProgram14DataType* pData= (FxProgram14DataType*)data;
    return threeBandEqProcessSample(sampleIn,&pData->eq);
}

__attribute__ ((section (".qspi_code")))
static void fxProgramSetup(void*data)
{
    FxProgram14DataType* pData= (FxProgram14DataType*)data;
    initThreeBandEq(&pData->eq);
}

__attribute__ ((section (".qspi_code")))
static void fxProgramReset(void*data)
{
    FxProgram14DataType* pData= (FxProgram14DataType*)data;
    threeBandEqReset(&pData->eq);   
}

__attribute__ ((section (".qspi_code")))
static void fxProgramParam1Callback(uint16_t val,void*data) // low
{
    FxProgram14DataType* pData= (FxProgram14DataType*)data;
    pData->eq.lowFactor = (float)val/512.0f-1.0f;
    fxProgram14.parameters[0].rawValue = val;
}

__attribute__ ((section (".qspi_code")))
static void fxProgramParam1Display(void*data,char*res)
{
    FxProgram14DataType* pData= (FxProgram14DataType*)data;
    decimalInt16ToChar((int16_t)(pData->eq.lowFactor),res,2);
}

__attribute__ ((section (".qspi_code")))
static void fxProgramParam2Callback(uint16_t val,void*data) // mid
{
    FxProgram14DataType* pData= (FxProgram14DataType*)data;
    pData->eq.midFactor = (float)val/512.0f-1.0f;
    fxProgram14.parameters[1].rawValue = val;
}

__attribute__ ((section (".qspi_code")))
static void fxProgramParam2Display(void*data,char*res)
{
    FxProgram14DataType* pData= (FxProgram14DataType*)data;
    decimalInt16ToChar((int16_t)(pData->eq.midFactor),res,2);
}

__attribute__ ((section (".qspi_code")))
static void fxProgramParam3Callback(uint16_t val,void*data) // high
{
    FxProgram14DataType* pData= (FxProgram14DataType*)data;
    pData->eq.highFactor = (float)val/512.0f-1.0f;
    fxProgram14.parameters[2].rawValue = val;
}

__attribute__ ((section (".qspi_code")))
static void fxProgramParam3Display(void*data,char*res)
{
    FxProgram14DataType* pData= (FxProgram14DataType*)data;
    decimalInt16ToChar((int16_t)(pData->eq.highFactor),res,2);
}



FxProgram14DataType fxProgram14data;

FxProgramType fxProgram14 = {
    .name = "3-Band Equalizer",
    .nParameters=3,
    .parameters = {
        {
            .name = "Low           ",
            .control=0,
            .increment=32,
            .rawValue=0,
            .getParameterDisplay=&fxProgramParam1Display,
            .getParameterValue=0,
            .setParameter=&fxProgramParam1Callback
        },
        {
            .name = "Mid            ",
            .control=1,
            .increment=32,
            .rawValue=0,
            .getParameterDisplay=&fxProgramParam2Display,
            .getParameterValue=0,
            .setParameter=&fxProgramParam2Callback
        },
        {
            .name = "High           ",
            .control=2,
            .increment=32,
            .rawValue=0,
            .getParameterDisplay=&fxProgramParam3Display,
            .getParameterValue=0,
            .setParameter=&fxProgramParam3Callback
        }
    },
    .processSample = &fxProgramProcessSample,
    .setup = &fxProgramSetup,
    .reset = &fxProgramReset,
    .data = (void*)&fxProgram14data
};
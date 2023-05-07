#include "pipicofx/fxPrograms.h"
#include "stringFunctions.h"
#include "romfunc.h"
static int16_t fxProgramProcessSample(int16_t sampleIn,void*data)
{
    FxProgram14DataType* pData= (FxProgram14DataType*)data;
    return threeBandEqProcessSample(sampleIn,&pData->eq);
}

static void fxProgramSetup(void*data)
{
    FxProgram14DataType* pData= (FxProgram14DataType*)data;
    initThreeBandEq(&pData->eq);
}

static void fxProgramReset(void*data)
{
    FxProgram14DataType* pData= (FxProgram14DataType*)data;
    threeBandEqReset(&pData->eq);   
}

static void fxProgramParam1Callback(uint16_t val,void*data) // low
{
    FxProgram14DataType* pData= (FxProgram14DataType*)data;
    pData->eq.lowFactor = (-(1 << 12)) + (val << 3);
    fxProgram14.parameters[0].rawValue = val;
}

static void fxProgramParam1Display(void*data,char*res)
{
    FxProgram14DataType* pData= (FxProgram14DataType*)data;
    float fFactor;
    fFactor = int2float((int32_t)pData->eq.lowFactor);
    fFactor /= 40.96f;
    decimalInt16ToChar((int16_t)float2int(fFactor),res,2);
}

static void fxProgramParam2Callback(uint16_t val,void*data) // mid
{
    FxProgram14DataType* pData= (FxProgram14DataType*)data;
    pData->eq.midFactor = (-(1 << 12)) + (val << 3);
    fxProgram14.parameters[1].rawValue = val;
}

static void fxProgramParam2Display(void*data,char*res)
{
    FxProgram14DataType* pData= (FxProgram14DataType*)data;
    float fFactor;
    fFactor = int2float((int32_t)pData->eq.midFactor);
    fFactor /= 40.96f;
    decimalInt16ToChar((int16_t)float2int(fFactor),res,2);
}

static void fxProgramParam3Callback(uint16_t val,void*data) // high
{
    FxProgram14DataType* pData= (FxProgram14DataType*)data;
    pData->eq.highFactor = (-(1 << 12)) + (val << 3);
    fxProgram14.parameters[2].rawValue = val;
}

static void fxProgramParam3Display(void*data,char*res)
{
    FxProgram14DataType* pData= (FxProgram14DataType*)data;
    float fFactor;
    fFactor = int2float((int32_t)pData->eq.highFactor);
    fFactor /= 40.96f;
    decimalInt16ToChar((int16_t)float2int(fFactor),res,2);
}



FxProgram14DataType fxProgram14data;

FxProgramType fxProgram14 = {
    .name = "3-Band Equalizer",
    .nParameters=3,
    .parameters = {
        {
            .name = "Low           ",
            .control=0,
            .increment=64,
            .rawValue=0,
            .getParameterDisplay=&fxProgramParam1Display,
            .getParameterValue=0,
            .setParameter=&fxProgramParam1Callback
        },
        {
            .name = "Mid            ",
            .control=1,
            .increment=64,
            .rawValue=0,
            .getParameterDisplay=&fxProgramParam2Display,
            .getParameterValue=0,
            .setParameter=&fxProgramParam2Callback
        },
        {
            .name = "High           ",
            .control=2,
            .increment=64,
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

#include "pipicofx/fxPrograms.h"
#include "stringFunctions.h"

__attribute__ ((section (".qspi_code")))
static float fxProgramprocessSample(float sampleIn,void*data)
{
    FxProgram10DataType* pData= (FxProgram10DataType*)data;
    return reverbProcessSample(sampleIn,&pData->reverb);
}

__attribute__ ((section (".qspi_code")))
static void fxProgramParam1Callback(uint16_t val,void*data) // reverb time
{
    FxProgram10DataType* pData= (FxProgram10DataType*)data;
    pData->reverbTime = ((float)val)/4095.0f*(2.0f-0.1f) + 0.1f;
    setReverbTime(pData->reverbTime,&pData->reverb);
    fxProgram10.parameters[0].rawValue = val;
}

__attribute__ ((section (".qspi_code")))
static void fxProgramParam1Display(void*data,char*res)
{
    FxProgram10DataType* pData= (FxProgram10DataType*)data;
    int16_t reverbms = (int16_t)(pData->reverbTime*1000.0f);
    Int16ToChar(reverbms,res);
    appendToString(res," ms");
}

__attribute__ ((section (".qspi_code")))
static void fxProgramParam2Callback(uint16_t val,void*data) // Mix
{
    FxProgram10DataType* pData= (FxProgram10DataType*)data;
    float fval=(float)val/4095.0;
    pData->reverb.mix=fval;
    fxProgram10.parameters[1].rawValue = val;
}

__attribute__ ((section (".qspi_code")))
static void fxProgramParam2Display(void*data,char*res)
{
    FxProgram10DataType* pData= (FxProgram10DataType*)data;
    int16_t mixpercent = (int16_t)(pData->reverb.mix*100.0);
    Int16ToChar(mixpercent,res);
    appendToString(res,"%");
}

__attribute__ ((section (".qspi_code")))
static void fxProgramParam3Callback(uint16_t val,void*data) // Parameter
{
    FxProgram10DataType* pData= (FxProgram10DataType*)data;
    pData->reverb.paramNr=(val >> 10);
    initReverb(&pData->reverb,pData->reverbTime);
    fxProgram10.parameters[2].rawValue = val;
}

__attribute__ ((section (".qspi_code")))
static void fxProgramParam3Display(void*data,char*res)
{
    FxProgram10DataType* pData= (FxProgram10DataType*)data;
    *res=0;
    appendToString(res,getReverbParameterSetName(&pData->reverb));
}


__attribute__ ((section (".qspi_code")))
static void fxProgramSetup(void*data)
{
    FxProgram10DataType* pData= (FxProgram10DataType*)data;
    initReverb(&pData->reverb,pData->reverbTime);
}

FxProgram10DataType fxProgram10data=
{
    .reverbTime=0.3f,
    .reverb.paramNr=0
};

FxProgramType fxProgram10 = {
    .name = "Reverb",
    .nParameters=3,
    .parameters = {
        {
            .name = "Time           ",
            .control=0,
            .increment=32,
            .rawValue=0,
            .getParameterDisplay=&fxProgramParam1Display,
            .getParameterValue=0,
            .setParameter=&fxProgramParam1Callback
        },
        {
            .name = "Mix            ",
            .control=1,
            .increment=32,
            .rawValue=0,
            .getParameterDisplay=&fxProgramParam2Display,
            .getParameterValue=0,
            .setParameter=&fxProgramParam2Callback
        },
        {
            .name = "ParameterSet   ",
            .control=2,
            .increment=1024,
            .rawValue=0,
            .getParameterDisplay=&fxProgramParam3Display,
            .getParameterValue=0,
            .setParameter=&fxProgramParam3Callback
        }
    },
    .processSample = &fxProgramprocessSample,
    .setup = &fxProgramSetup,
    .reset = 0,
    .data = (void*)&fxProgram10data
};
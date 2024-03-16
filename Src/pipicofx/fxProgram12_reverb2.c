
#include "pipicofx/fxPrograms.h"
#include "stringFunctions.h"

__attribute__ ((section (".qspi_code")))
static float fxProgramprocessSample(float sampleIn,void*data)
{
    FxProgram12DataType* pData= (FxProgram12DataType*)data;
    return reverb2ProcessSample(sampleIn,&pData->reverb);
}

__attribute__ ((section (".qspi_code")))
static void fxProgramParam1Callback(uint16_t val,void*data) // reverb time
{
    FxProgram12DataType* pData= (FxProgram12DataType*)data;
    pData->reverb.decay = (float)val/4095.0f;
    fxProgram12.parameters[0].rawValue = val;
}

__attribute__ ((section (".qspi_code")))
static void fxProgramParam1Display(void*data,char*res)
{
    FxProgram12DataType* pData= (FxProgram12DataType*)data;
    Int16ToChar((int16_t)(pData->reverb.decay*100.0f), res);
}

__attribute__ ((section (".qspi_code")))
static void fxProgramParam2Callback(uint16_t val,void*data) // Mix
{
    FxProgram12DataType* pData= (FxProgram12DataType*)data;
    pData->reverb.mix=val/4095.0f;
    fxProgram12.parameters[1].rawValue = val;
}

__attribute__ ((section (".qspi_code")))
static void fxProgramParam2Display(void*data,char*res)
{
    FxProgram12DataType* pData= (FxProgram12DataType*)data;
    Int16ToChar((int16_t)(pData->reverb.mix*100.0f),res);
    appendToString(res,"%");

}

__attribute__ ((section (".qspi_code")))
static void fxProgramSetup(void*data)
{
    FxProgram12DataType* pData= (FxProgram12DataType*)data;
    initReverb2(&pData->reverb);
}

FxProgram12DataType fxProgram12data;

FxProgramType fxProgram12 = {
    .name = "Allpass Reverb",
    .nParameters=2,
    .parameters = {
        {
            .name = "Decay          ",
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
        }
    },
    .processSample = &fxProgramprocessSample,
    .setup = &fxProgramSetup,
    .reset = 0,
    .data = (void*)&fxProgram12data
};

#include "pipicofx/fxPrograms.h"
#include "stringFunctions.h"
#include "drivers/adc.h"

__attribute__ ((section (".qspi_code")))
static float fxProgramProcessSample(float sampleIn,void*data)
{
    FxProgram16DataType* pData= (FxProgram16DataType*)data;
    float processedSample = pitchShifterProcessSample(sampleIn,&pData->pitchShifter);
    float sampleOut= (sampleIn*(1.0f - pData->mix)) + (processedSample*pData->mix);
    return sampleOut;
}

__attribute__ ((section (".qspi_code")))
static void fxProgramParam1Callback(uint16_t val,void*data) // low
{
    FxProgram16DataType* pData= (FxProgram16DataType*)data;
    pData->pitchShifter.delayIncrement = (val >> 9) - 4;
    if (pData->pitchShifter.delayIncrement>=0)
    {
        pData->pitchShifter.delayIncrement+=1;
    }
    fxProgram16.parameters[0].rawValue = val;
}

__attribute__ ((section (".qspi_code")))
static void fxProgramParam1Display(void*data,char*res)
{
    FxProgram16DataType* pData= (FxProgram16DataType*)data;
    *res=0;
    switch (pData->pitchShifter.delayIncrement)
    {
    case -4:
        appendToString(res,"OctUp");
        break;
    case -3:
        appendToString(res,"MoreUp");
        break;
    case -2:
        appendToString(res,"HalfUp");
        break;
    case -1:
        appendToString(res,"LittleUp");
        break;
    case 1:
        appendToString(res,"LittleDown");
        break;
    case 2:
        appendToString(res,"HalfDown");
        break;
    case 3:
        appendToString(res,"MoreDown");
        break;
    case 4:
        appendToString(res,"OctDown");
        break;
    default:
        appendToString(res,"Static");
        break;
    }
}

__attribute__ ((section (".qspi_code")))
static void fxProgramParam2Callback(uint16_t val,void*data) // Mix
{
    FxProgram16DataType* pData= (FxProgram16DataType*)data;
    pData->mix=val/4095.0f;
    fxProgram16.parameters[1].rawValue = val;
}

__attribute__ ((section (".qspi_code")))
static void fxProgramParam2Display(void*data,char*res)
{
    FxProgram16DataType* pData= (FxProgram16DataType*)data;
    Int16ToChar(pData->mix*100.0f,res);
    appendToString(res,"%");
}

__attribute__ ((section (".qspi_code")))
static void fxProgramParam3Callback(uint16_t val,void*data) // BufferSize
{
    FxProgram16DataType* pData= (FxProgram16DataType*)data;
    uint16_t newVal = (val >> 10)+9;
    if (newVal != pData->pitchShifter.buffersizePowerTwo)
    {
        pData->pitchShifter.buffersizePowerTwo=newVal;
        initPitchshifter(&pData->pitchShifter);
    }
    fxProgram16.parameters[1].rawValue = val;
}

__attribute__ ((section (".qspi_code")))
static void fxProgramParam3Display(void*data,char*res)
{
    FxProgram16DataType* pData= (FxProgram16DataType*)data;
    int16_t avgDelayMs=((pData->pitchShifter.buffersize >> 1) / (AUDIO_SAMPLING_RATE/1000));
    Int16ToChar(avgDelayMs,res);
    appendToString(res, "ms");
}

__attribute__ ((section (".qspi_code")))
static void fxProgramSetup(void*data)
{
    FxProgram16DataType* pData= (FxProgram16DataType*)data;
    initPitchshifter(&pData->pitchShifter);
    
}

__attribute__ ((section (".qspi_code")))
static void fxProgramReset(void*data)
{
    FxProgram16DataType* pData= (FxProgram16DataType*)data;
    initPitchshifter(&pData->pitchShifter);  
}

FxProgram16DataType fxProgram16data=
{
    .pitchShifter.currentDelayPosition=0,
    .pitchShifter.delayIncrement=0x4
};

FxProgramType fxProgram16 = {
    .name = "Pitchshifter",
    .nParameters=3,
    .parameters = {
        {
            .name = "ShiftAmt",
            .control=0,
            .increment=32,
            .rawValue=0,
            .getParameterDisplay=&fxProgramParam1Display,
            .getParameterValue=0,
            .setParameter=&fxProgramParam1Callback
        },
        {
            .name = "Mix",
            .control=1,
            .increment=32,
            .rawValue=0,
            .getParameterDisplay=&fxProgramParam2Display,
            .getParameterValue=0,
            .setParameter=&fxProgramParam2Callback
        },
        {
            .name = "AvgDelay",
            .control=2,
            .increment=512,
            .rawValue=0,
            .getParameterDisplay=&fxProgramParam3Display,
            .getParameterValue=0,
            .setParameter=&fxProgramParam3Callback
        }
    },
    .processSample = &fxProgramProcessSample,
    .setup = &fxProgramSetup,
    .reset = &fxProgramReset,
    .data = (void*)&fxProgram16data
};
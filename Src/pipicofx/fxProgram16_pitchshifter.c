#include "pipicofx/fxPrograms.h"
#include "stringFunctions.h"

static int16_t fxProgramProcessSample(int16_t sampleIn,void*data)
{
    FxProgram16DataType* pData= (FxProgram16DataType*)data;
    int16_t processedSample = pitchShifterProcessSample(sampleIn,&pData->pitchShifter);
    int16_t sampleOut= ((processedSample)*((1 << 15) - pData->mix) >> 15) + ((sampleIn)*pData->mix >> 15);
    return sampleOut;
}

static void fxProgramParam1Callback(uint16_t val,void*data) // low
{
    FxProgram16DataType* pData= (FxProgram16DataType*)data;
    pData->pitchShifter.delayIncrement = val >> 9;
    fxProgram16.parameters[0].rawValue = val;
}

static void fxProgramParam1Display(void*data,char*res)
{
    FxProgram16DataType* pData= (FxProgram16DataType*)data;
    *res=0;
    switch (pData->pitchShifter.delayIncrement)
    {
    case 8:
        appendToString(res,"OctUp");
        break;
    case 7:
        appendToString(res,"4thUp");
        break;
    case 6:
        appendToString(res,"5thUp");
        break;
    case 5:
        appendToString(res,"WeirdUp");
        break;
    case 4:
        appendToString(res,"NoShift");
        break;
    case 3:
        appendToString(res,"4thDown");
        break;
    case 2:
        appendToString(res,"OctDown");
        break;
    case 1:
        appendToString(res,"2OctDown");
        break;
    default:
        appendToString(res,"Static");
        break;
    }
}


static void fxProgramParam2Callback(uint16_t val,void*data) // Mix
{
    FxProgram16DataType* pData= (FxProgram16DataType*)data;
    pData->mix=(val << 3);
    fxProgram16.parameters[1].rawValue = val;
}

static void fxProgramParam2Display(void*data,char*res)
{
    FxProgram16DataType* pData= (FxProgram16DataType*)data;
    int16_t mixpercent = (int16_t)(pData->mix/328);
    Int16ToChar(mixpercent,res);
    appendToString(res,"%");
}

static void fxProgramSetup(void*data)
{
    FxProgram16DataType* pData= (FxProgram16DataType*)data;
    initPitchshifter(&pData->pitchShifter);
    
}

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
    .nParameters=2,
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
        }
    },
    .processSample = &fxProgramProcessSample,
    .setup = &fxProgramSetup,
    .reset = &fxProgramReset,
    .data = (void*)&fxProgram16data
};
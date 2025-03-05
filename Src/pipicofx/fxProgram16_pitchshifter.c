#include "pipicofx/fxPrograms.h"
#include "stringFunctions.h"
#include "drivers/adc.h"

static int16_t fxProgramProcessSample(int16_t sampleIn,void*data)
{
    FxProgram16DataType* pData= (FxProgram16DataType*)data;
    int16_t processedSample = pitchShifter2ProcessSample(sampleIn,&pData->pitchShifter,getAudioStatePtr());
    int16_t sampleOut= ((sampleIn)*((1 << 15) - pData->mix) >> 15) + ((processedSample)*pData->mix >> 15);
    return sampleOut;
}

static void fxProgramParam1Callback(uint16_t val,void*data) // low
{
    FxProgram16DataType* pData= (FxProgram16DataType*)data;
    pData->pitchShifter.delayIncrement = (val >> 9) + 1;
    fxProgram16.parameters[0].rawValue = val;
}

static void fxProgramParam1Display(void*data,char*res)
{
    FxProgram16DataType* pData= (FxProgram16DataType*)data;
    *res=0;
    switch (pData->pitchShifter.delayIncrement)
    {
    case 1:
        appendToString(res,"2OctDown");
        break;
    case 2:
        appendToString(res,"OctDown");
        break;
    case 3:
        appendToString(res,"FourthDown");
        break;
    case 4:
        appendToString(res,"NoShift");
        break;
    case 5:
        appendToString(res,"ThirdUp");
        break;
    case 6:
        appendToString(res,"FifthUp");
        break;
    case 7:
        appendToString(res,"Devil666");
        break;
    case 8:
        appendToString(res,"OctUp");
        break;
    default:
        appendToString(res,"ERROR");
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

static void fxProgramParam3Callback(uint16_t val,void*data) // BufferSize
{
    FxProgram16DataType* pData= (FxProgram16DataType*)data;
    uint16_t newVal = (val >> 10)+9;
    if (newVal != pData->pitchShifter.buffersizePowerTwo)
    {
        pData->pitchShifter.buffersizePowerTwo=newVal;
        initPitchshifter2(&pData->pitchShifter);
    }
    fxProgram16.parameters[1].rawValue = val;
}

static void fxProgramParam3Display(void*data,char*res)
{
    FxProgram16DataType* pData= (FxProgram16DataType*)data;
    int16_t avgDelayMs=((pData->pitchShifter.buffersize >> 1) / (AUDIO_SAMPLING_RATE/1000));
    Int16ToChar(avgDelayMs,res);
    appendToString(res, "ms");
}


static void fxProgramSetup(void*data)
{
    FxProgram16DataType* pData= (FxProgram16DataType*)data;
    initPitchshifter2(&pData->pitchShifter);
    
}

static void fxProgramReset(void*data)
{
    FxProgram16DataType* pData= (FxProgram16DataType*)data;
    initPitchshifter2(&pData->pitchShifter);  
}

FxProgram16DataType fxProgram16data=
{
    .pitchShifter.currentDelayPosition=0,
    .pitchShifter.delayIncrement=0x4,
    .pitchShifter.crossFadeWidthPwr2=7
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
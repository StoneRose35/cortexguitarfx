
#include "audio/fxprogram/fxProgram.h"
#include "stringFunctions.h"

static int16_t fxProgramprocessSample(int16_t sampleIn,void*data)
{
    FxProgram12DataType* pData= (FxProgram12DataType*)data;
    return reverb2ProcessSample(sampleIn,&pData->reverb);
}

static void fxProgramParam1Callback(uint16_t val,void*data) // reverb time
{
    FxProgram12DataType* pData= (FxProgram12DataType*)data;
    pData->reverb.decay = val << 3;
}

static void fxProgramParam1Display(void*data,char*res)
{
    FxProgram12DataType* pData= (FxProgram12DataType*)data;
    Int16ToChar(pData->reverb.decay, res);
}

static void fxProgramParam2Callback(uint16_t val,void*data) // Mix
{
    FxProgram12DataType* pData= (FxProgram12DataType*)data;
    pData->reverb.mix=(val << 3);
}

static void fxProgramParam2Display(void*data,char*res)
{
    FxProgram12DataType* pData= (FxProgram12DataType*)data;
    int16_t mixpercent = (int16_t)(pData->reverb.mix/328);
    Int16ToChar(mixpercent,res);
    appendToString(res,"%");

}


static void fxProgramSetup(void*data)
{
    FxProgram12DataType* pData= (FxProgram12DataType*)data;
    initReverb2(&pData->reverb);
}

FxProgram12DataType fxProgram12data;

FxProgramType fxProgram12 = {
    .name = "Allpass Reverb         ",
    .nParameters=2,
    .parameters = {
        {
            .name = "Decay          ",
            .control=0,
            .increment=64,
            .rawValue=0,
            .getParameterDisplay=&fxProgramParam1Display,
            .getParameterValue=0,
            .setParameter=&fxProgramParam1Callback
        },
        {
            .name = "Mix            ",
            .control=1,
            .increment=64,
            .rawValue=0,
            .getParameterDisplay=&fxProgramParam2Display,
            .getParameterValue=0,
            .setParameter=&fxProgramParam2Callback
        }
    },
    .processSample = &fxProgramprocessSample,
    .setup = &fxProgramSetup,
    .data = (void*)&fxProgram12data
};


#include "audio/fxprogram/fxProgram.h"
#include "stringFunctions.h"

static float fxProgramprocessSample(float sampleIn,void*data)
{
    FxProgram10DataType* pData= (FxProgram10DataType*)data;
    return reverbProcessSample(sampleIn,&pData->reverb);
}

static void fxProgramParam1Callback(uint16_t val,void*data) // reverb time
{
    FxProgram10DataType* pData= (FxProgram10DataType*)data;
    pData->reverbTime = ((float)val)/4095.0f*(2.0f-0.1f) + 0.1;
    setReverbTime(pData->reverbTime,&pData->reverb);
}

static void fxProgramParam1Display(void*data,char*res)
{
    FxProgram10DataType* pData= (FxProgram10DataType*)data;
    int16_t reverbms = (int16_t)(pData->reverbTime*1000.0f);
    Int16ToChar(reverbms,res);
    appendToString(res," ms");
}

static void fxProgramParam2Callback(uint16_t val,void*data) // Mix
{
    FxProgram10DataType* pData= (FxProgram10DataType*)data;
    float fval=(float)val/4095.0;
    pData->reverb.mix=fval;
}

static void fxProgramParam2Display(void*data,char*res)
{
    FxProgram10DataType* pData= (FxProgram10DataType*)data;
    int16_t mixpercent = (int16_t)(pData->reverb.mix*100.0);
    Int16ToChar(mixpercent,res);
    appendToString(res,"%");

}


static void fxProgramSetup(void*data)
{
    FxProgram10DataType* pData= (FxProgram10DataType*)data;
    initReverb(&pData->reverb,pData->reverbTime);
}

FxProgram10DataType fxProgram10data=
{
    .reverbTime=0.3f
};

FxProgramType fxProgram10 = {
    .name = "Reverb                 ",
    .nParameters=2,
    .parameters = {
        {
            .name = "Time           ",
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
    .data = (void*)&fxProgram10data
};
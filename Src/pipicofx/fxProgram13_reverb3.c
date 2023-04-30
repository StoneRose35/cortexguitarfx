#include "audio/fxprogram/fxProgram.h"
#include "stringFunctions.h"
#include "romfunc.h"

static int16_t fxProgramprocessSample(int16_t sampleIn,void*data)
{
    int16_t reverberatedSample;
    FxProgram13DataType* pData= (FxProgram13DataType*)data;
    reverberatedSample = reverb3processSample(sampleIn,&pData->reverb);
    return (((0x7FFF - pData->mix)*sampleIn) >> 15) + ((pData->mix*reverberatedSample) >> 15);
}

static void fxProgramParam1Callback(uint16_t val,void*data) // decay / delay feedback
{
    int16_t intermVal;
    FxProgram13DataType* pData= (FxProgram13DataType*)data;
    intermVal = val << 3;
    if (intermVal > 0x7FFD)
    {
        intermVal = 0x7FFD;
    }
    pData->reverb.delay.feedback = intermVal;
}

static void fxProgramParam1Display(void*data,char*res)
{
    float ffbk;
    int16_t t60;
    FxProgram13DataType* pData= (FxProgram13DataType*)data;
    ffbk = int2float(pData->reverb.delay.feedback)/32767.0f;
    if (ffbk < 0.0000305)
    {
        t60=0;
    }
    else
    {
        t60=(int16_t)float2int(-589.03004f/fln(ffbk)); // t60 in ms
    }
    Int16ToChar(t60,res);
    appendToString(res," ms");
}

static void fxProgramParam2Callback(uint16_t val,void*data) // mix
{
    FxProgram13DataType* pData= (FxProgram13DataType*)data;
        pData->mix=(val << 3);
}

static void fxProgramParam2Display(void*data,char*res)
{
    FxProgram13DataType* pData= (FxProgram13DataType*)data;
    int16_t mixpercent = (int16_t)(pData->mix/328);
    Int16ToChar(mixpercent,res);
    appendToString(res,"%");
}

static void fxProgramSetup(void*data)
{
    FxProgram13DataType* pData= (FxProgram13DataType*)data;
    initReverb3(&pData->reverb);
}

FxProgram13DataType fxProgram13data;

FxProgramType fxProgram13 = {
    .name = "Hadamard Reverb",
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
    .reset = 0,
    .data = (void*)&fxProgram13data
};
#include "pipicofx/fxPrograms.h"
#include "stringFunctions.h"
#include "ln.h"

__attribute__ ((section (".qspi_code")))
static float fxProgramprocessSample(float sampleIn,void*data)
{
    float reverberatedSample;
    FxProgram13DataType* pData= (FxProgram13DataType*)data;
    reverberatedSample = reverb3processSample(sampleIn,&pData->reverb);
    return ((1.0f - pData->mix)*sampleIn) + (pData->mix*reverberatedSample);
}

__attribute__ ((section (".qspi_code")))
static void fxProgramParam1Callback(uint16_t val,void*data) // decay / delay feedback
{
    float intermVal;
    FxProgram13DataType* pData= (FxProgram13DataType*)data;
    intermVal = (float)val/4095.0f;
    if (intermVal > 0x7FFD/32768.0f)
    {
        intermVal = 0x7FFD/32768.0f;
    }
    pData->reverb.delay.feedback = intermVal;
    fxProgram13.parameters[0].rawValue = val;
}

__attribute__ ((section (".qspi_code")))
static void fxProgramParam1Display(void*data,char*res)
{
    float ffbk;
    int16_t t60;
    FxProgram13DataType* pData= (FxProgram13DataType*)data;
    ffbk = (float)(pData->reverb.delay.feedback)/32767.0f;
    if (ffbk < 0.0000305f)
    {
        t60=0;
    }
    else
    {
        t60=(int16_t)(-589.03004f/fln(ffbk)); // t60 in ms
    }
    Int16ToChar(t60,res);
    appendToString(res," ms");
}

__attribute__ ((section (".qspi_code")))
static void fxProgramParam2Callback(uint16_t val,void*data) // mix
{
    FxProgram13DataType* pData= (FxProgram13DataType*)data;
    pData->mix=val/4095.0f;
    fxProgram13.parameters[1].rawValue = val;
}

__attribute__ ((section (".qspi_code")))
static void fxProgramParam2Display(void*data,char*res)
{
    FxProgram13DataType* pData= (FxProgram13DataType*)data;
    int16_t mixpercent = (int16_t)(pData->mix*100.0f);
    Int16ToChar(mixpercent,res);
    appendToString(res,"%");
}

__attribute__ ((section (".qspi_code")))
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
    .data = (void*)&fxProgram13data
};
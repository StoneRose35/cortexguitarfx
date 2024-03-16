#include "pipicofx/fxPrograms.h"
#include "stringFunctions.h"
#include "romfunc.h"
#include "audio/compressor.h"

__attribute__ ((section (".qspi_code")))
static float fxProgramProcessSample(float sampleIn,void*data)
{
    FxProgram15DataType* pData= (FxProgram15DataType*)data;
    sampleIn = threeBandEqProcessSample(sampleIn,&pData->eq);
    sampleIn = compressor2ProcessSample(sampleIn,&pData->comp);
    sampleIn = gainStageProcessSample(sampleIn,&pData->postGain);
    sampleIn = reverbProcessSample(sampleIn,&pData->reverb);
    return sampleIn;
}

__attribute__ ((section (".qspi_code")))
static void fxProgramSetup(void*data)
{
    FxProgram15DataType* pData= (FxProgram15DataType*)data;
    initThreeBandEq(&pData->eq);
    initReverb(&pData->reverb,500);
}

__attribute__ ((section (".qspi_code")))
static void fxProgramReset(void*data)
{
    FxProgram15DataType* pData= (FxProgram15DataType*)data;
    threeBandEqReset(&pData->eq);   
}

__attribute__ ((section (".qspi_code")))
static void fxProgramParam1Callback(uint16_t val,void*data) // low
{
    FxProgram15DataType* pData= (FxProgram15DataType*)data;
    pData->eq.lowFactor = (float)val/512.0f-1.0f;
    fxProgram15.parameters[0].rawValue = val;
}

__attribute__ ((section (".qspi_code")))
static void fxProgramParam1Display(void*data,char*res)
{
    FxProgram15DataType* pData= (FxProgram15DataType*)data;
    decimalInt16ToChar((int16_t)(pData->eq.lowFactor),res,2);
}

__attribute__ ((section (".qspi_code")))
static void fxProgramParam2Callback(uint16_t val,void*data) // mid
{
    FxProgram15DataType* pData= (FxProgram15DataType*)data;
    pData->eq.midFactor = (float)val/512.0f-1.0f;
    fxProgram15.parameters[1].rawValue = val;
}

__attribute__ ((section (".qspi_code")))
static void fxProgramParam2Display(void*data,char*res)
{
    FxProgram15DataType* pData= (FxProgram15DataType*)data;
    decimalInt16ToChar((int16_t)(pData->eq.midFactor),res,2);
}

__attribute__ ((section (".qspi_code")))
static void fxProgramParam3Callback(uint16_t val,void*data) // high
{
    FxProgram15DataType* pData= (FxProgram15DataType*)data;
    pData->eq.highFactor = (float)val/512.0f-1.0f;
    fxProgram15.parameters[2].rawValue = val;
}

__attribute__ ((section (".qspi_code")))
static void fxProgramParam3Display(void*data,char*res)
{
    FxProgram15DataType* pData= (FxProgram15DataType*)data;
    decimalInt16ToChar((int16_t)(pData->eq.highFactor),res,2);
}

__attribute__ ((section (".qspi_code")))
static void fxProgramParam4Callback(uint16_t val,void*data) // compressor (one-knob setting)
{
    FxProgram15DataType* pData= (FxProgram15DataType*)data;
    float maxGain;
    // map val to a thresshold range of 0 to ~ -60dB /-8599
    pData->comp.gainFunction.threshhold = 1.0f - (float)val/16384.0f;
    // compute post Gain according to threshhold set
    maxGain = getMaxGain(&pData->comp);
    maxGain = 0.8f/maxGain; 
    pData->postGain.gain = 0.8f/maxGain; 
    fxProgram15.parameters[3].rawValue = val;
}

__attribute__ ((section (".qspi_code")))
static void fxProgramParam4Display(void*data,char*res)
{
    int16_t comppercent = (int16_t)(fxProgram15.parameters[3].rawValue << 3);
    Int16ToChar(comppercent/328,res);
    appendToString(res,"%");
}

__attribute__ ((section (".qspi_code")))
static void fxProgramParam5Callback(uint16_t val,void*data) // reverb time
{
    FxProgram15DataType* pData= (FxProgram15DataType*)data;
    pData->reverbTime = (((uint32_t)val*1900)>>12) + 100;
    setReverbTime(pData->reverbTime,&pData->reverb);
    fxProgram15.parameters[4].rawValue = val;
}

__attribute__ ((section (".qspi_code")))
static void fxProgramParam5Display(void*data,char*res)
{
    FxProgram15DataType* pData= (FxProgram15DataType*)data;
    Int16ToChar(pData->reverbTime,res);
    appendToString(res," ms");
}

__attribute__ ((section (".qspi_code")))
static void fxProgramParam6Callback(uint16_t val,void*data) // reverb mix
{
    FxProgram15DataType* pData= (FxProgram15DataType*)data;
    pData->reverb.mix=(float)val/4095.0f;
    fxProgram15.parameters[5].rawValue = val;
}

__attribute__ ((section (".qspi_code")))
static void fxProgramParam6Display(void*data,char*res)
{
    FxProgram15DataType* pData= (FxProgram15DataType*)data;
    int16_t mixpercent = (int16_t)(pData->reverb.mix*100.0f);
    Int16ToChar(mixpercent,res);
    appendToString(res,"%");
}




FxProgram15DataType fxProgram15data=
{
    .comp.gainFunction.gainReduction=2.0f,
    .comp.gainFunction.threshhold=1.0f,
    .comp.avgLowpass.alphaFalling = 32765.0f/32768.0f,
    .comp.avgLowpass.alphaRising = 15.0f/32768.0f,
    .eq.highFactor = 0.0f,
    .eq.midFactor = 0.0f,
    .eq.lowFactor = 0.0f,
    .reverb.mix = 0.0f,
    .reverb.paramNr = 1,
    .postGain.gain = 1.0f
};

FxProgramType fxProgram15 = {
    .name = "Acoustic Proc",
    .nParameters=6,
    .parameters = {
        {
            .name = "EQ Low",
            .control=0,
            .increment=32,
            .rawValue=0,
            .getParameterDisplay=&fxProgramParam1Display,
            .getParameterValue=0,
            .setParameter=&fxProgramParam1Callback
        },
        {
            .name = "EQ Mid",
            .control=1,
            .increment=32,
            .rawValue=0,
            .getParameterDisplay=&fxProgramParam2Display,
            .getParameterValue=0,
            .setParameter=&fxProgramParam2Callback
        },
        {
            .name = "EQ High",
            .control=2,
            .increment=32,
            .rawValue=0,
            .getParameterDisplay=&fxProgramParam3Display,
            .getParameterValue=0,
            .setParameter=&fxProgramParam3Callback
        },
        {
            .name = "Compressor Int.",
            .control=0xFF,
            .increment=32,
            .rawValue=0,
            .getParameterDisplay=&fxProgramParam4Display,
            .getParameterValue=0,
            .setParameter=&fxProgramParam4Callback            
        },
        {
            .name = "Reverb Time    ",
            .control=0xFF,
            .increment=32,
            .rawValue=0,
            .getParameterDisplay=&fxProgramParam5Display,
            .getParameterValue=0,
            .setParameter=&fxProgramParam5Callback            
        },
        {
            .name = "Reverb Mix     ",
            .control=0xFF,
            .increment=32,
            .rawValue=0,
            .getParameterDisplay=&fxProgramParam6Display,
            .getParameterValue=0,
            .setParameter=&fxProgramParam6Callback            
        }
    },
    .processSample = &fxProgramProcessSample,
    .setup = &fxProgramSetup,
    .reset = &fxProgramReset,
    .data = (void*)&fxProgram15data
};
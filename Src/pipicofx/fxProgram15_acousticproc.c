#include "pipicofx/fxPrograms.h"
#include "stringFunctions.h"
#include "romfunc.h"
#include "audio/compressor.h"

static int16_t fxProgramProcessSample(int16_t sampleIn,void*data)
{
    FxProgram15DataType* pData= (FxProgram15DataType*)data;
    sampleIn = threeBandEqProcessSample(sampleIn,&pData->eq);
    sampleIn = compressor2ProcessSample(sampleIn,&pData->comp);
    sampleIn = gainStageProcessSample(sampleIn,&pData->postGain);
    sampleIn = reverbProcessSample(sampleIn,&pData->reverb);
    return sampleIn;
}

static void fxProgramSetup(void*data)
{
    FxProgram15DataType* pData= (FxProgram15DataType*)data;
    initThreeBandEq(&pData->eq);
}

static void fxProgramReset(void*data)
{
    FxProgram15DataType* pData= (FxProgram15DataType*)data;
    threeBandEqReset(&pData->eq);   
}

static void fxProgramParam1Callback(uint16_t val,void*data) // low
{
    FxProgram15DataType* pData= (FxProgram15DataType*)data;
    pData->eq.lowFactor = (-(1 << 12)) + (val << 3);
    fxProgram15.parameters[0].rawValue = val;
}

static void fxProgramParam1Display(void*data,char*res)
{
    FxProgram15DataType* pData= (FxProgram15DataType*)data;
    float fFactor;
    fFactor = int2float((int32_t)pData->eq.lowFactor);
    fFactor /= 40.96f;
    decimalInt16ToChar((int16_t)float2int(fFactor),res,2);
}

static void fxProgramParam2Callback(uint16_t val,void*data) // mid
{
    FxProgram15DataType* pData= (FxProgram15DataType*)data;
    pData->eq.midFactor = (-(1 << 12)) + (val << 3);
    fxProgram15.parameters[1].rawValue = val;
}

static void fxProgramParam2Display(void*data,char*res)
{
    FxProgram15DataType* pData= (FxProgram15DataType*)data;
    float fFactor;
    fFactor = int2float((int32_t)pData->eq.midFactor);
    fFactor /= 40.96f;
    decimalInt16ToChar((int16_t)float2int(fFactor),res,2);
}

static void fxProgramParam3Callback(uint16_t val,void*data) // high
{
    FxProgram15DataType* pData= (FxProgram15DataType*)data;
    pData->eq.highFactor = (-(1 << 12)) + (val << 3);
    fxProgram15.parameters[2].rawValue = val;
}

static void fxProgramParam3Display(void*data,char*res)
{
    FxProgram15DataType* pData= (FxProgram15DataType*)data;
    float fFactor;
    fFactor = int2float((int32_t)pData->eq.highFactor);
    fFactor /= 40.96f;
    decimalInt16ToChar((int16_t)float2int(fFactor),res,2);
}

static void fxProgramParam4Callback(uint16_t val,void*data) // compressor (one-knob setting)
{
    FxProgram15DataType* pData= (FxProgram15DataType*)data;
    int32_t maxGain;
    // map val to a thresshold range of 0 to ~ -60dB /-8599
    pData->comp.gainFunction.threshhold = - (val << 1);
    // compute post Gain according to threshhold set
    maxGain = getMaxGain(&pData->comp);
    maxGain = 32767/maxGain; // 0.8 as q15
    maxGain >>= 8;
    pData->postGain.gain = (int16_t)maxGain;
    fxProgram15.parameters[3].rawValue = val;
}

static void fxProgramParam4Display(void*data,char*res)
{
    int16_t comppercent = (int16_t)(fxProgram15.parameters[3].rawValue << 3);
    Int16ToChar(comppercent,res);
    appendToString(res,"%");
}

static void fxProgramParam5Callback(uint16_t val,void*data) // reverb time
{
    FxProgram15DataType* pData= (FxProgram15DataType*)data;
    pData->reverbTime = (((uint32_t)val*1900)>>12) + 100;
    setReverbTime(pData->reverbTime,&pData->reverb);
    fxProgram15.parameters[4].rawValue = val;
}

static void fxProgramParam5Display(void*data,char*res)
{
    FxProgram15DataType* pData= (FxProgram15DataType*)data;
    Int16ToChar(pData->reverbTime,res);
    appendToString(res," ms");
}

static void fxProgramParam6Callback(uint16_t val,void*data) // reverb mix
{
    FxProgram15DataType* pData= (FxProgram15DataType*)data;
    pData->reverb.mix=(val << 3);
    fxProgram15.parameters[5].rawValue = val;
}

static void fxProgramParam6Display(void*data,char*res)
{
    FxProgram15DataType* pData= (FxProgram15DataType*)data;
    int16_t mixpercent = (int16_t)(pData->reverb.mix/328);
    Int16ToChar(mixpercent,res);
    appendToString(res,"%");
}




FxProgram15DataType fxProgram15data=
{
    .comp.gainFunction.gainReduction=1,
    .comp.gainFunction.threshhold=0x7FFF,
    .comp.avgLowpass.alphaFalling = 32765,
    .comp.avgLowpass.alphaRising = 15,
    .eq.highFactor = 0,
    .eq.midFactor = 0,
    .eq.lowFactor = 0,
    .reverb.mix = 0,
    .reverb.paramNr = 1,

};

FxProgramType fxProgram15 = {
    .name = "Acoustic Proc",
    .nParameters=6,
    .parameters = {
        {
            .name = "EQ Low        ",
            .control=0,
            .increment=32,
            .rawValue=0,
            .getParameterDisplay=&fxProgramParam1Display,
            .getParameterValue=0,
            .setParameter=&fxProgramParam1Callback
        },
        {
            .name = "EQ Mid         ",
            .control=1,
            .increment=32,
            .rawValue=0,
            .getParameterDisplay=&fxProgramParam2Display,
            .getParameterValue=0,
            .setParameter=&fxProgramParam2Callback
        },
        {
            .name = "EQ High        ",
            .control=2,
            .increment=32,
            .rawValue=0,
            .getParameterDisplay=&fxProgramParam3Display,
            .getParameterValue=0,
            .setParameter=&fxProgramParam3Callback
        },
        {
            .name = "Compressor Int. ",
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
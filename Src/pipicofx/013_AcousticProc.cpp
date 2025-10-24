
#include "pipicofx/013_AcousticProc.hpp"
extern "C" {
#include "stringFunctions.h"
#include "pipicofx/delayMemoryHandler.h"
#include "audio/gainstage.h"
#include "memoryRegions.h"
}
using namespace PiPicoFX;

__ITCM_CODE
float AcousticProc::AcousticProc::processSample(float sampleIn)
{
    sampleIn = compressor2ProcessSample(sampleIn,&this->comp);
    sampleIn = gainStageProcessSample(sampleIn,&this->postGain);
    sampleIn = gainStageProcessSample(sampleIn,&this->presetVolume);
    sampleIn = reverbProcessSample(sampleIn,&this->reverb);
    return sampleIn;
}

void AcousticProc::AcousticProc::setup()
{
    initThreeBandEq(&this->eq);
    initReverb(&this->reverb,500,mallocDelayMemory(20480<<2));
    this->addParameter(new Param1(this));
    this->addParameter(new Param2(this));
    this->addParameter(new Param3(this));
    this->addParameter(new Param4(this));
    this->addParameter(new Param5(this));
    this->addParameter(new Param6(this));
    this->addParameter(new Param7(this));
}

AcousticProc::AcousticProc::~AcousticProc()
{
    freeDelayMemory(this->reverb.delayPointers[0]);
}

/*
__QSPI_CODE
void fxProgramReset(void*data)
{
    FxProgram15DataType* pData= (FxProgram15DataType*)data;
    threeBandEqReset(&pData->eq);   
}
*/

void AcousticProc::Param1::parameterCallback(uint16_t val) // low
{
    pData->eq.lowFactor = (float)val/512.0f-1.0f;
    rawValue = val;
}

void AcousticProc::Param1::parameterDisplay(char*res)
{
    decimalInt16ToChar((int16_t)(pData->eq.lowFactor*100.f),res,2);
}

void AcousticProc::Param2::parameterCallback(uint16_t val) // mid
{
    pData->eq.midFactor = (float)val/512.0f-1.0f;
    rawValue = val;
}

void AcousticProc::Param2::parameterDisplay(char*res)
{
    decimalInt16ToChar((int16_t)(pData->eq.midFactor*100.0f),res,2);
}

void AcousticProc::Param3::parameterCallback(uint16_t val) // high
{
    pData->eq.highFactor = (float)val/512.0f-1.0f;
    rawValue = val;
}

void AcousticProc::Param3::parameterDisplay(char*res)
{
    decimalInt16ToChar((int16_t)(pData->eq.highFactor*100.0f),res,2);
}

void AcousticProc::Param4::parameterCallback(uint16_t val) // compressor (one-knob setting)
{
    float maxGain;
    // map val to a thresshold range of 0 to ~ -60dB /-8599
    pData->comp.gainFunction.threshhold = 1.0f - (float)val/16384.0f;
    // compute post Gain according to threshhold set
    maxGain = getMaxGain(&pData->comp);
    maxGain = 0.8f/maxGain; 
    pData->postGain.gain = 0.8f/maxGain; 
    rawValue = val;
}

void AcousticProc::Param4::parameterDisplay(char*res)
{
    int16_t comppercent = (int16_t)(rawValue << 3);
    Int16ToChar(comppercent/328,res);
    appendToString(res,"%");
}

void AcousticProc::Param5::parameterCallback(uint16_t val) // reverb time
{
    pData->reverbTime = 0.1f + val/4095.f*1.9f;
    setReverbTime(pData->reverbTime,&pData->reverb);
    rawValue = val;
}

void AcousticProc::Param5::parameterDisplay(char*res)
{
    Int16ToChar((float)(pData->reverbTime*1000.0f),res);
    appendToString(res," ms");
}

void AcousticProc::Param6::parameterCallback(uint16_t val) // reverb mix
{
    pData->reverb.mix=(float)val/4095.0f;
    rawValue = val;
}

void AcousticProc::Param6::parameterDisplay(char*res)
{
    int16_t mixpercent = (int16_t)(pData->reverb.mix*100.0f);
    Int16ToChar(mixpercent,res);
    appendToString(res,"%");
}

void AcousticProc::Param7::parameterCallback(uint16_t val)
{
    pData->presetVolume.gain = ((float)val)/1024.0f; // 0.0f up to 4.0f
    rawValue = val;
}


void AcousticProc::Param7::parameterDisplay(char*res)
{
    int16_t dVal;
    dVal=(int16_t)(pData->presetVolume.gain*100.0f);
    Int16ToChar(dVal,res);
    appendToString(res,"%");
}

/*

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

*/
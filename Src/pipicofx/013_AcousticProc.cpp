
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
    float newIn=0.0f;
    if (this->isOn())
    {
        newIn = sampleIn;
    }
    newIn = threeBandEqProcessSample(newIn,&this->eq);
    newIn = compressor2ProcessSample(newIn,&this->comp);
    newIn = gainStageProcessSample(newIn,&this->postGain);
    newIn = gainStageProcessSample(newIn,&this->presetVolume);
    newIn = reverbProcessSample(newIn,&this->reverb);
    if (!this->isOn())
    {
        return (sampleIn + newIn);
    }
    return newIn;
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
    uint16_t dVal;
    dVal=(uint16_t)(pData->presetVolume.gain*10000.0f);
    decimalUInt16ToChar(dVal,res,2);
    appendToString(res,"%");
}

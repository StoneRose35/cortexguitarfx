
#include "pipicofx/009_SineModulation.hpp"
extern "C" {
#include "stringFunctions.h"
#include "audio/gainstage.h"
#include "pipicofx/delayMemoryHandler.h"
#include "memoryRegions.h"
}
using namespace PiPicoFX;

__ITCM_CODE
float SineModulation::SineModulation::processSample(float sampleIn)
{
    float newIn=0.0f;
    if (this->isOn())
    {
        newIn = sampleIn;
    }
    newIn = gainStageProcessSample(newIn,&this->presetVolume);
    newIn = sineChorusInterpolatedProcessSample(newIn,&this->sineChorus);
    if (!this->isOn())
    {
        return (sampleIn + newIn);
    }
    return newIn;
}

void SineModulation::Param1::parameterCallback(uint16_t val) // frequency
{
    // map 0 - 4095 to 1 1000
    rawValue = val;
    val = ((val*250) >> 10) + 1;
    sineChorusSetFrequency(val,&pData->sineChorus);
}

void SineModulation::Param1::parameterDisplay(char*res)
{
    decimalInt16ToChar(pData->sineChorus.frequency,res,2);
    appendToString(res," Hz");
}

void SineModulation::Param2::parameterCallback(uint16_t val) // depth
{
    rawValue = val;
    // map to 0 to 255
    val >>= 4;
    pData->sineChorus.depth = (uint8_t)val;
}

void SineModulation::Param2::parameterDisplay(char*res)
{
    int16_t dVal;
    dVal = pData->sineChorus.depth; 
    Int16ToChar(dVal,res);
}

void SineModulation::Param3::parameterCallback(uint16_t val) // mix
{
    pData->sineChorus.mix = ((float)val)/4095.0f;
    rawValue = val;
}

void SineModulation::Param3::parameterDisplay(char*res)
{
    int16_t dVal;
    dVal = (int16_t)(pData->sineChorus.mix*100.0f);
    Int16ToChar(dVal,res);
    appendToString(res,"%");
}

void SineModulation::Param4::parameterCallback(uint16_t val)
{
    rawValue = val;
    pData->sineChorus.offset = 49 + (val >> 1);
}

void SineModulation::Param4::parameterDisplay(char*res)
{
    uint16_t msValue;
    msValue = (uint16_t)(((uint32_t)pData->sineChorus.offset*21) >> 10);
    UInt16ToChar(msValue,res);
    appendToString(res, " ms");
}

void SineModulation::Param5::parameterCallback(uint16_t val)
{
    pData->sineChorus.feedback = val/4095.0f;
    rawValue = val;
}

void SineModulation::Param5::parameterDisplay(char*res)
{
    Int16ToChar((int16_t)(pData->sineChorus.feedback*100.0f),res);
    appendToString(res,"%");
}

void SineModulation::Param6::parameterCallback(uint16_t val)
{
    pData->presetVolume.gain = ((float)val)/1024.0f; // 0.0f up to 4.0f
    rawValue = val;
}

void SineModulation::Param6::parameterDisplay(char*res)
{
    uint16_t dVal;
    dVal=(uint16_t)(pData->presetVolume.gain*10000.0f);
    decimalUInt16ToChar(dVal,res,2);
    appendToString(res,"%");
}

void SineModulation::SineModulation::setup(uint8_t allocateMemory)
{
    if(allocateMemory)
    {
        initSineChorus(&this->sineChorus,mallocDelayMemory(SINE_CHORUS_DELAY_SIZE<<2));
    }
    this->addParameter(new Param1(this));
    this->addParameter(new Param2(this));
    this->addParameter(new Param3(this));
    this->addParameter(new Param4(this));
    this->addParameter(new Param5(this));
    this->addParameter(new Param6(this));
    FxProgram::setup(allocateMemory);
}

SineModulation::SineModulation::~SineModulation()
{
    freeDelayMemory(this->sineChorus.delayBuffer);
}

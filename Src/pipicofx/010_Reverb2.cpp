
#include "pipicofx/010_Reverb2.hpp"
extern "C" {
#include "stringFunctions.h"
#include "audio/gainstage.h"
#include "pipicofx/delayMemoryHandler.h"
#include "memoryRegions.h"
}
using namespace PiPicoFX;

__ITCM_CODE
float Reverb2::Reverb2::processSample(float sampleIn)
{
    float newIn=0.0f;
    if (this->isOn())
    {
        newIn = sampleIn;
    }
    newIn = gainStageProcessSample(newIn,&this->presetVolume);
    newIn = reverb2ProcessSample(newIn,&this->reverb);
    if (!this->isOn())
    {
        return (sampleIn + newIn);
    }
    return newIn;
}

void Reverb2::Param1::parameterCallback(uint16_t val) // reverb time
{
    pData->reverb.decay = (float)val/4095.0f;
    rawValue = val;
}

void Reverb2::Param1::parameterDisplay(char*res)
{
    Int16ToChar((int16_t)(pData->reverb.decay*100.0f), res);
}

void Reverb2::Param2::parameterCallback(uint16_t val) // Mix
{
    pData->reverb.mix=val/4095.0f;
    rawValue = val;
}

void Reverb2::Param2::parameterDisplay(char*res)
{
    Int16ToChar((int16_t)(pData->reverb.mix*100.0f),res);
    appendToString(res,"%");
}

void Reverb2::Param3::parameterCallback(uint16_t val)
{
    pData->presetVolume.gain = ((float)val)/1024.0f; // 0.0f up to 4.0f
    rawValue = val;
}

void Reverb2::Param3::parameterDisplay(char*res)
{
    uint16_t dVal;
    dVal=(uint16_t)(pData->presetVolume.gain*10000.0f);
    decimalUInt16ToChar(dVal,res,2);
    appendToString(res,"%");
}

void Reverb2::Reverb2::setup()
{
    initReverb2(&this->reverb,mallocDelayMemory(24576<<2));
    this->addParameter(new Param1(this));
    this->addParameter(new Param2(this));
    this->addParameter(new Param3(this));
}

Reverb2::Reverb2::~Reverb2()
{
    freeDelayMemory(this->reverb.aps[0].delayLineIn);
}

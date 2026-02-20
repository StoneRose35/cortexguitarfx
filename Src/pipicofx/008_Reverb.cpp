
#include "pipicofx/008_Reverb.hpp"
extern "C" {
#include "stringFunctions.h"
#include "pipicofx/delayMemoryHandler.h"
#include "audio/gainstage.h"
#include "memoryRegions.h"
}
using namespace PiPicoFX;

__ITCM_CODE
 float Reverb::Reverb::processSample(float sampleIn)
{
    float newIn=0.0f;;
    if (this->isOn())
    {
        newIn=sampleIn;
    }
    newIn = gainStageProcessSample(newIn,&this->presetVolume);
    newIn =  reverbProcessSample(newIn,&this->reverb);
    if(!this->isOn())
    {
        return sampleIn + newIn;
    }
    return newIn;
}

 void Reverb::Param1::parameterCallback(uint16_t val) // reverb time
{
    pData->reverbTime = ((float)val)/4095.0f*(2.0f-0.1f) + 0.1f;
    setReverbTime(pData->reverbTime,&pData->reverb);
    rawValue = val;
}

 void Reverb::Param1::parameterDisplay(char*res)
{
    int16_t reverbms = (int16_t)(pData->reverbTime*1000);
    Int16ToChar(reverbms,res);
    appendToString(res," ms");
}

 void Reverb::Param2::parameterCallback(uint16_t val) // Mix
{
    float fval=(float)val/4095.0;
    pData->reverb.mix=fval;
    rawValue = val;
}

 void Reverb::Param2::parameterDisplay(char*res)
{
    int16_t mixpercent = (int16_t)(pData->reverb.mix*100.0);
    Int16ToChar(mixpercent,res);
    appendToString(res,"%");
}

 void Reverb::Param3::parameterCallback(uint16_t val) // Parameter
{
    pData->reverb.paramNr=(val >> 10);
    freeDelayMemory(pData->reverb.delayPointers[0]);
    initReverb(&pData->reverb,pData->reverbTime,mallocDelayMemory(pData->getDelayMemoryUseage()));
    rawValue = val;
}

 void Reverb::Param3::parameterDisplay(char*res)
{
    *res=0;
    appendToString(res,getReverbParameterSetName(&pData->reverb));
}

void Reverb::Param4::parameterCallback(uint16_t val)
{
    pData->presetVolume.gain = ((float)val)/1024.0f; // 0.0f up to 4.0f
    rawValue = val;
}

void Reverb::Param4::parameterDisplay(char*res)
{
    uint16_t dVal;
    dVal=(uint16_t)(pData->presetVolume.gain*10000.0f);
    decimalUInt16ToChar(dVal,res,2);
    appendToString(res,"%");
}

 void Reverb::Reverb::setup()
{
    initReverb(&this->reverb,this->reverbTime,mallocDelayMemory(this->getDelayMemoryUseage()));
    this->addParameter(new Param1(this));
    this->addParameter(new Param2(this));
    this->addParameter(new Param3(this));
    this->addParameter(new Param4(this));
    this->setFreezable(1);
}

Reverb::Reverb::~Reverb()
{
    freeDelayMemory(this->reverb.delayPointers[0]);
}

void Reverb::Reverb::freeze()
{
    FxProgram::freeze();
    reverb.frozen = 1;
}

void Reverb::Reverb::unfreeze()
{
    FxProgram::unfreeze();
    reverb.frozen = 0;
}
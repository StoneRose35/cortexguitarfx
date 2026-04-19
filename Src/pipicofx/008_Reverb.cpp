
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
    FxProgram::processSample(sampleIn);
    float newIn=0.0f;;
    if (this->isOn())
    {
        newIn=sampleIn;
    }
    if (this->getFreezeState()==FXP_FREEZE_STATE_FREEZING || this->getFreezeState() == FXP_FREEZE_STATE_MELTING)
    {
        this->reverb.feedbackValues[0] = (((float)freezeCnt)/(float)(FXP_FREEZE_DURATION_IN_SAMPLES-1)) + (1.0f - ((float)freezeCnt)/(float)(FXP_FREEZE_DURATION_IN_SAMPLES-1))*this->meltedTaus[0];
        this->reverb.feedbackValues[1] = (((float)freezeCnt)/(float)(FXP_FREEZE_DURATION_IN_SAMPLES-1)) + (1.0f - ((float)freezeCnt)/(float)(FXP_FREEZE_DURATION_IN_SAMPLES-1))*this->meltedTaus[1];
        this->reverb.feedbackValues[2] = (((float)freezeCnt)/(float)(FXP_FREEZE_DURATION_IN_SAMPLES-1)) + (1.0f - ((float)freezeCnt)/(float)(FXP_FREEZE_DURATION_IN_SAMPLES-1))*this->meltedTaus[2];
        this->reverb.feedbackValues[3] = (((float)freezeCnt)/(float)(FXP_FREEZE_DURATION_IN_SAMPLES-1)) + (1.0f - ((float)freezeCnt)/(float)(FXP_FREEZE_DURATION_IN_SAMPLES-1))*this->meltedTaus[3];
        this->reverb.gainIn = 1.0f - (((float)freezeCnt)/(float)(FXP_FREEZE_DURATION_IN_SAMPLES-1));
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
    if (this->pData->getFreezeState() == FXP_FREEZE_STATE_MELTED)
    {
        pData->reverbTime = ((float)val)/4095.0f*(2.0f-0.1f) + 0.1f;
        setReverbTime(pData->reverbTime,&pData->reverb);
        rawValue = val;
    }
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
    FxProgram::setup();
}

Reverb::Reverb::~Reverb()
{
    freeDelayMemory(this->reverb.delayPointers[0]);
}

void Reverb::Reverb::freeze()
{
    FxProgram::freeze();
    this->meltedTaus[0] = this->reverb.feedbackValues[0];
    this->meltedTaus[1] = this->reverb.feedbackValues[1];
    this->meltedTaus[2] = this->reverb.feedbackValues[2];
    this->meltedTaus[3] = this->reverb.feedbackValues[3];
    this->freezeCnt = 0;
}

void Reverb::Reverb::onFreeze()
{
    this->reverb.frozen = 1;
}

void Reverb::Reverb::onMelt()
{
    this->reverb.frozen = 0;
    this->reverb.gainIn = 1.0f;
}

void Reverb::Reverb::unfreeze()
{
    FxProgram::unfreeze();
    reverb.frozen = 0;
}

#include "pipicofx/011_Reverb3.hpp"
extern "C" {
#include "math.h"
#include "stringFunctions.h"
#include "pipicofx/delayMemoryHandler.h"
#include "audio/gainstage.h"
#include "memoryRegions.h"
}
using namespace PiPicoFX;

__ITCM_CODE
float Reverb3::Reverb3::processSample(float sampleIn)
{
    FxProgram::processSample(sampleIn);
    float newIn=0.0f;
    if (this->isOn())
    {
        newIn = sampleIn;
    }
    float reverberatedSample;
    if (this->getFreezeState()==FXP_FREEZE_STATE_FREEZING || this->getFreezeState() == FXP_FREEZE_STATE_MELTING)
    {
        this->reverb.delay.feedback = (((float)freezeCnt)/(float)(FXP_FREEZE_DURATION_IN_SAMPLES-1)) + (1.0f - ((float)freezeCnt)/(float)(FXP_FREEZE_DURATION_IN_SAMPLES-1))*this->meltedFeedbackValue;
        this->reverb.delay.gainIn = 1.0f - (((float)freezeCnt)/(float)(FXP_FREEZE_DURATION_IN_SAMPLES-1));
    }
    newIn = gainStageProcessSample(newIn,&this->presetVolume);
    reverberatedSample = reverb3processSample(newIn,&this->reverb);
    newIn = ((1.0f - this->mix)*newIn) + (this->mix*reverberatedSample);
    if (!this->isOn())
    {
        return (sampleIn + newIn);
    }
    return newIn;
}

void Reverb3::Param1::parameterCallback(uint16_t val) // decay / delay feedback
{
    float intermVal;
    intermVal = (float)val/4095.0f;
    if (intermVal > 0x7FFD/32768.0f)
    {
        intermVal = 0x7FFD/32768.0f;
    }
    pData->reverb.delay.feedback = intermVal;
    rawValue = val;
}

void Reverb3::Param1::parameterDisplay(char*res)
{
    float ffbk;
    int16_t t60;
    ffbk = (float)(pData->reverb.delay.feedback);
    if (ffbk < 0.0000305f)
    {
        t60=0;
    }
    else
    {
        t60=(int16_t)(-589.03004f/logf(ffbk)); // t60 in ms
    }
    Int16ToChar(t60,res);
    appendToString(res," ms");
}

void Reverb3::Param2::parameterCallback(uint16_t val) // mix
{
    pData->mix=val/4095.0f;
    rawValue = val;
}

void Reverb3::Param2::parameterDisplay(char*res)
{
    int16_t mixpercent = (int16_t)(pData->mix*100.0f);
    Int16ToChar(mixpercent,res);
    appendToString(res,"%");
}

void Reverb3::Param3::parameterCallback(uint16_t val)
{
    pData->presetVolume.gain = ((float)val)/1024.0f; // 0.0f up to 4.0f
    rawValue = val;
}

void Reverb3::Param3::parameterDisplay(char*res)
{
    uint16_t dVal;
    dVal=(uint16_t)(pData->presetVolume.gain*10000.0f);
    decimalUInt16ToChar(dVal,res,2);
    appendToString(res,"%");
}

void Reverb3::Reverb3::setup()
{
    initReverb3(&this->reverb,mallocDelayMemory((16*DIFFUSOR_SIZE + 4096)<<2));
    this->addParameter(new Param1(this));
    this->addParameter(new Param2(this));
    this->addParameter(new Param3(this));
    this->setFreezable(1);
}

Reverb3::Reverb3::~Reverb3()
{
    freeDelayMemory(this->reverb.diffusers[0].delayPointers[0]);
}


void Reverb3::Reverb3::freeze()
{
    FxProgram::freeze();
    this->meltedFeedbackValue = this->reverb.delay.feedback;
    this->freezeCnt=0;
}

void Reverb3::Reverb3::unfreeze()
{
    FxProgram::unfreeze();
}

void Reverb3::Reverb3::onFreeze()
{
    this->reverb.delay.frozen = 1;
}

void Reverb3::Reverb3::onMelt()
{
    this->reverb.delay.frozen = 0;
}

#include "pipicofx/005_Delay.hpp"
extern "C" {
#include "stringFunctions.h"
#include "audio/gainstage.h"
#include "pipicofx/delayMemoryHandler.h"
#include "memoryRegions.h"
#include "globalConfig.h"
}
using namespace PiPicoFX;
__ITCM_CODE
float Delay::Delay::processSample(float sampleIn)
{
    float newIn=0.0f;
    if (isOn())
    {
        newIn = sampleIn;
    }
    if (this->interpCnt < UI_LATENCY_IN_SAMPLES)
    {
        int32_t interpValue = ((((this->delayInSamplesTargetValue<< 4) - (this->getParameter(0)->rawValue << 4))*interpCnt) >> 12) + (this->getParameter(0)->rawValue << 4);
        this->delay.delayInSamples = interpValue;
        this->interpCnt++;
        if (this->interpCnt >= UI_LATENCY_IN_SAMPLES)
        {
            this->getParameter(0)->rawValue = this->delayInSamplesTargetValue;
        }
    }
    FxProgram::processSample(sampleIn);
    if (this->getFreezeState()==FXP_FREEZE_STATE_FREEZING || this->getFreezeState() == FXP_FREEZE_STATE_MELTING)
    {
        this->delay.feedback = (((float)freezeCnt)/(float)FXP_FREEZE_DURATION_IN_SAMPLES) + (1.0f - ((float)freezeCnt)/(float)FXP_FREEZE_DURATION_IN_SAMPLES)*this->meltedFeedbackValue;
    }
    //this->delay.delayInSamples = this->delay.delayInSamples + ((FXPROGRAM_DELAY_DELAY_TIME_LOWPASS_T*(delayInSamplesTargetValue - this->delay.delayInSamples)) >> 8);
    newIn = delayLineProcessSample(newIn, &this->delay);
    newIn = gainStageProcessSample(newIn,&this->presetVolume);
    if (!isOn())
    {
        return sampleIn + newIn;
    }
    return newIn;
}

void Delay::Param1::parameterCallback(uint16_t val) // Delay Time
{

    if (val != this->rawValue && this->pData->interpCnt>=UI_LATENCY_IN_SAMPLES)
    {
        this->pData->delayInSamplesTargetValue = val;
        this->pData->interpCnt=0;
    }
}

void Delay::Param1::parameterDisplay(char*res)
{
    int16_t dval;
    dval = pData->delay.delayInSamples/48; // in ms
    Int16ToChar(dval,res);
    for (uint8_t c=0;c<PARAMETER_NAME_MAXLEN-2;c++)
    {
        if(*(res+c)==0)
        {
            *(res+c)='m';
            *(res+c+1)='s';
            *(res+c+2)=(char)0;
            break;
        }
    }
}

void Delay::Param2::parameterCallback(uint16_t val) // Feedback
{
    if (this->pData->getFreezeState() == FXP_FREEZE_STATE_MELTED)
    {
        rawValue = val;
        pData->delay.feedback=((float)val)/4096.0f;
    }
}


void Delay::Param2::parameterDisplay(char*res)
{
    Int16ToChar((int16_t)(pData->delay.feedback*100.0f),res);
    for (uint8_t c=0;c<PARAMETER_NAME_MAXLEN-1;c++)
    {
        if(*(res+c)==0)
        {
            *(res+c)='%';
            *(res+c+1)=(char)0;
            break;
        }
    }
}

void Delay::Param3::parameterCallback(uint16_t val) // Mix
{
    pData->delay.mix = ((float)val)/4096.0f;
    this->rawValue = val; 
}

void Delay::Param3::parameterDisplay(char*res)
{
    Int16ToChar(pData->delay.mix*100.0f,res);
    for (uint8_t c=0;c<PARAMETER_NAME_MAXLEN-1;c++)
    {
        if(*(res+c)==0)
        {
            *(res+c)='%';
            *(res+c+1)=(char)0;
            break;
        }
    }
}

void Delay::Param4::parameterCallback(uint16_t val)
{
    pData->presetVolume.gain = ((float)val)/1024.0f; // 0 to 1024
    this->rawValue = val;
}

void Delay::Param4::parameterDisplay(char*res)
{
    uint16_t dVal;
    dVal=(uint16_t)(pData->presetVolume.gain*10000.0f);
    decimalUInt16ToChar(dVal,res,2);
    appendToString(res,"%");
}

void Delay::Delay::setup()
{
    initDelay(&this->delay,mallocDelayMemory(MAX_DELAY_SINGLEBUFFER<<2),MAX_DELAY_SINGLEBUFFER);
    this->interpCnt = 0;
    this->addParameter(new Param1(this));
    this->addParameter(new Param2(this));
    this->addParameter(new Param3(this));
    this->addParameter(new Param4(this));
    this->setFreezable(1);

}

Delay::Delay::~Delay()
{
    freeDelayMemory(this->delay.delayLine);
}

void Delay::Delay::freeze()
{
    FxProgram::freeze();
    this->meltedFeedbackValue = this->delay.feedback;
    this->freezeCnt=0;
}

void Delay::Delay::unfreeze()
{
    FxProgram::unfreeze();
}

void Delay::Delay::onFreeze()
{
    this->delay.frozen = 1;
}

void Delay::Delay::onMelt()
{
    this->delay.frozen = 0;
}
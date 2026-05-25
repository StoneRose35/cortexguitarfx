
#include "pipicofx/017_FreeVerb.hpp"
extern "C" {
#include "stringFunctions.h"
#include "audio/gainstage.h"
#include "audio/delay.h"
#include "audio/reverbUtils.h"
#include "pipicofx/delayMemoryHandler.h"
#include "audio/firstOrderIirFilter.h"
#include "pipicofx/delayMemoryHandler.h"
#include "memoryRegions.h"
#include "math.h"

__ITCM_CODE
float freeVerbLowpass(float sampleIn,void * filterData)
{
    return firstOrderIirLowpassProcessSample(sampleIn,(FirstOrderIirType*)filterData);
}

}
using namespace PiPicoFX;

__ITCM_CODE
float FreeVerb::FreeVerb::processSample(float sampleIn)
{
    float newIn=0.0f;
    if (this->isOn())
    {
        newIn = sampleIn;
    }
    float sampleOut;
    float delaySum=0.0f;
    FxProgram::processSample(sampleIn);
    if (this->getFreezeState()==FXP_FREEZE_STATE_FREEZING || this->getFreezeState() == FXP_FREEZE_STATE_MELTING)
    {
        float frzVal = (((float)freezeCnt)/(float)(FXP_FREEZE_DURATION_IN_SAMPLES-1));
        for (uint8_t c=0;c<8;c++)
        {
            this->delays[c].feedback = frzVal + (1.0f - frzVal)*this->meltedFeedbackValue;
            this->delays[c].gainIn = 1.0f - frzVal;
        }
    }
    for (uint8_t c=0;c<8;c++)
    {
        delaySum += delayLineWetProcessSample(newIn,this->delays+c);
    }
    sampleOut = delaySum;
    for (uint8_t c=0;c<4;c++)
    {
        sampleOut = allpassProcessSample(sampleOut,this->allpasses+c);
    }
    sampleOut = (1.0f - this->mix)*newIn + this->mix*sampleOut;
    sampleOut = gainStageProcessSample(sampleOut,&this->presetVolume);
    if (!this->isOn())
    {
        return (sampleIn + sampleOut);
    }
    return sampleOut;
}

void FreeVerb::FreeVerb::setup(uint8_t allocateMemory)
{
    if (allocateMemory)
    {
        float* delayMemPtr = mallocDelayMemory(24576<<2);

        for (uint8_t c=0;c<8;c++)
        {
            initDelay(this->delays+c,delayMemPtr+c*2048,2048);
            this->delays[c].feebackData = (void*)(this->feedbackFilters+c);
        }
        for (uint8_t c=0;c<4;c++)
        {
            this->allpasses[c].delayLineIn=delayMemPtr+8*2048+c*2048;
            this->allpasses[c].delayLineOut=delayMemPtr+8*2048+c*2048+1024;
        }
    }
    this->addParameter(new Param1(this));
    this->addParameter(new Param2(this));
    this->addParameter(new Param3(this));
    this->addParameter(new Param4(this));
    this->setFreezable(1);
    FxProgram::setup(allocateMemory);
}

FreeVerb::FreeVerb::~FreeVerb()
{
    freeDelayMemory(this->delays[0].delayLine);
}

void FreeVerb::Param1::parameterCallback(uint16_t val)
{
    float feedback = ((float)val)/4095.0f*0.28f+0.7f; // val*0.28+0.7;
    for (uint8_t c=0;c<8;c++)
    {
        (this->pData->delays+c)->feedback = feedback;
    }
    this->rawValue = val; 
}

void FreeVerb::Param1::parameterDisplay(char*res)
{
    float ffbk;
    int16_t t60;
    ffbk = this->pData->delays[0].feedback;
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

void FreeVerb::Param2::parameterCallback(uint16_t val)
{
    for (uint8_t c=0;c<8;c++)
    {
        (this->pData->feedbackFilters+c)->alpha = 1.0f - ((float)val)/4096.0f;
    }
    this->rawValue = val; 
}

void FreeVerb::Param2::parameterDisplay(char*res)
{
    int16_t damping = this->pData->feedbackFilters[0].alpha;
    Int16ToChar(damping/328,res);
    appendToString(res,"%");
}

void FreeVerb::Param3::parameterCallback(uint16_t val)
{
    this->pData->mix=((float)val)/4096.0f;
    this->rawValue = val; 
}

void FreeVerb::Param3::parameterDisplay(char*res)
{
    Int16ToChar(pData->mix*100.0f,res);
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

void FreeVerb::Param4::parameterCallback(uint16_t val)
{
    pData->presetVolume.gain = ((float)val)/1024.0f; // 0.0f up to 4.0f
    rawValue = val;
}

void FreeVerb::Param4::parameterDisplay(char*res)
{
    uint16_t dVal;
    dVal=(uint16_t)(pData->presetVolume.gain*10000.0f);
    decimalUInt16ToChar(dVal,res,2);
    appendToString(res,"%");
}

void FreeVerb::FreeVerb::freeze()
{
    FxProgram::freeze();
    this->meltedFeedbackValue = this->delays[0].feedback;
    this->freezeCnt=0;
}

void FreeVerb::FreeVerb::unfreeze()
{
    FxProgram::unfreeze();
}

void FreeVerb::FreeVerb::onFreeze()
{
    for(uint8_t c=0;c<8;c++)
    {
        this->delays[c].frozen = 1;
    }
}

void FreeVerb::FreeVerb::onMelt()
{
    for(uint8_t c=0;c<8;c++)
    {
        this->delays[c].frozen = 0;
    }
}
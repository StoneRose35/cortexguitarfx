
#include "pipicofx/017_FreeVerb.hpp"
extern "C" {
#include "stringFunctions.h"
#include "audio/gainstage.h"
#include "audio/delay.h"
#include "audio/reverbUtils.h"
#include "pipicofx/delayMemoryHandler.h"
#include "audio/firstOrderIirFilter.h"
#include "romfunc.h"

__attribute__ ((section (".ramfunc"))) 
int16_t freeVerbLowpass(int16_t sampleIn,void * filterData,volatile uint32_t * audioStatePtr)
{
    return firstOrderIirLowpassProcessSample(sampleIn,(FirstOrderIirType*)filterData);
}

}
using namespace PiPicoFX;

int16_t FreeVerb::FreeVerb::processSample(int16_t sampleIn)
{
    int32_t sampleOut;
    volatile uint32_t * audioStatePtr = getAudioStatePtr();
    int32_t delaySum=0;
    
    for (uint8_t c=0;c<8;c++)
    {
        delaySum += delayLineWetProcessSample(sampleIn,this->delays+c);
    }
    sampleOut = delaySum >> 3;
    for (uint8_t c=0;c<4;c++)
    {
        sampleOut = allpassProcessSample(sampleOut,this->allpasses+c,audioStatePtr);
    }
    sampleOut = clip(((((1 << 15) - this->mix)*sampleIn) >> 15) + ((this->mix*sampleOut) >> 15),audioStatePtr);
    return gainStageProcessSample(sampleOut,&this->presetVolume);
}

void FreeVerb::FreeVerb::setup()
{
    int16_t* delayMemPtr = mallocDelayMemory(24576<<1);

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

    this->addParameter(new Param1(this));
    this->addParameter(new Param2(this));
    this->addParameter(new Param3(this));
    this->addParameter(new Param4(this));

}

FreeVerb::FreeVerb::~FreeVerb()
{
    freeDelayMemory(this->delays[0].delayLine);
}

void FreeVerb::Param1::parameterCallback(uint16_t val)
{
    int32_t feedback = (((val << 3)*9175)>>15) + 22938;// val*0.28+0.7;
    for (uint8_t c=0;c<8;c++)
    {
        (this->pData->delays+c)->feedback = (int16_t)feedback;
    }
    this->rawValue = val; 
}

void FreeVerb::Param1::parameterDisplay(char*res)
{
    float ffbk;
    int16_t t60;
    ffbk = int2float(this->pData->delays[0].feedback)/32767.0f;
    if (ffbk < 0.0000305)
    {
        t60=0;
    }
    else
    {
        t60=(int16_t)float2int(-589.03004f/fln(ffbk)); // t60 in ms
    }
    Int16ToChar(t60,res);
    appendToString(res," ms");
}
void FreeVerb::Param2::parameterCallback(uint16_t val)
{
    for (uint8_t c=0;c<8;c++)
    {
        (this->pData->feedbackFilters+c)->alpha = 32767 - (val << 3);
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
    this->pData->mix=(val << 3);
    this->rawValue = val; 
}

void FreeVerb::Param3::parameterDisplay(char*res)
{
    int16_t mixpercent = (int16_t)(this->pData->mix/328);
    Int16ToChar(mixpercent,res);
    appendToString(res,"%");
}
void FreeVerb::Param4::parameterCallback(uint16_t val)
{
    this->pData->presetVolume.gain = val >> 2; // 0 to 1024
    this->rawValue = val; 
}

void FreeVerb::Param4::parameterDisplay(char*res)
{
    int16_t dVal;
    dVal = this->pData->presetVolume.gain*39; // percent with two decimal points
    decimalInt16ToChar(dVal,res,2);
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

#include "pipicofx/008_Reverb.hpp"
extern "C" {
#include "stringFunctions.h"
#include "pipicofx/delayMemoryHandler.h"
#include "audio/gainstage.h"
}
using namespace PiPicoFX;

int16_t Reverb::Reverb::processSample(int16_t sampleIn)
{
    sampleIn = gainStageProcessSample(sampleIn,&this->presetVolume);
    return reverbProcessSample(sampleIn,&this->reverb);
}

void Reverb::Reverb::setup()
{
    initReverb(&this->reverb,this->reverbTime,mallocDelayMemory(20480<<1));
    this->addParameter(new Param1(this));
    this->addParameter(new Param2(this));
    this->addParameter(new Param3(this));
    this->addParameter(new Param4(this));

}

void Reverb::Param1::parameterCallback(uint16_t val)
{
    this->pData->reverbTime = (((uint32_t)val*1900)>>12) + 100;
    setReverbTime(this->pData->reverbTime,&this->pData->reverb);
    this->rawValue = val; 
}

void Reverb::Param1::parameterDisplay(char*res)
{
    Int16ToChar(this->pData->reverbTime,res);
    appendToString(res," ms");
}
void Reverb::Param2::parameterCallback(uint16_t val)
{
    this->pData->reverb.mix=(val << 3);
    this->rawValue = val; 
}

void Reverb::Param2::parameterDisplay(char*res)
{
    int16_t mixpercent = (int16_t)(this->pData->reverb.mix/328);
    Int16ToChar(mixpercent,res);
    appendToString(res,"%");
}
void Reverb::Param3::parameterCallback(uint16_t val)
{
    this->pData->reverb.paramNr=(val >> 10);
    initReverb(&this->pData->reverb,this->pData->reverbTime,this->pData->reverb.delayPointers[0]);
    this->rawValue = val; 
}

void Reverb::Param3::parameterDisplay(char*res)
{
    *res=0;
    appendToString(res,getReverbParameterSetName(&this->pData->reverb));
}
void Reverb::Param4::parameterCallback(uint16_t val)
{
    this->pData->presetVolume.gain = val >> 2; // 0 to 1024
    this->rawValue = val; 
}

void Reverb::Param4::parameterDisplay(char*res)
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
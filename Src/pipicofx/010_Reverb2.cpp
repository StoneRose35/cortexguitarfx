
#include "pipicofx/010_Reverb2.hpp"
extern "C" {
#include "stringFunctions.h"
#include "audio/gainstage.h"
#include "pipicofx/delayMemoryHandler.h"
}
using namespace PiPicoFX;

int16_t Reverb2::Reverb2::processSample(int16_t sampleIn)
{
    sampleIn = gainStageProcessSample(sampleIn,&this->presetVolume);
    return reverb2ProcessSample(sampleIn,&this->reverb);
}

void Reverb2::Reverb2::setup()
{
    initReverb2(&this->reverb,mallocDelayMemory(24576<<1));
    this->addParameter(new Param1(this));
    this->addParameter(new Param2(this));
    this->addParameter(new Param3(this));
}

Reverb2::Reverb2::~Reverb2()
{
    freeDelayMemory(this->reverb.aps[0].delayLineIn);
}

void Reverb2::Param1::parameterCallback(uint16_t val)
{
    this->pData->reverb.decay = val << 3;
    this->rawValue = val; 
}

void Reverb2::Param1::parameterDisplay(char*res)
{
    Int16ToChar(this->pData->reverb.decay, res);
}
void Reverb2::Param2::parameterCallback(uint16_t val)
{
    this->pData->reverb.mix=(val << 3);
    this->rawValue = val; 
}

void Reverb2::Param2::parameterDisplay(char*res)
{
    int16_t mixpercent = (int16_t)(this->pData->reverb.mix/328);
    Int16ToChar(mixpercent,res);
    appendToString(res,"%");
}
void Reverb2::Param3::parameterCallback(uint16_t val)
{
    this->pData->presetVolume.gain = val >> 2; // 0 to 1024
    this->rawValue = val; 
}

void Reverb2::Param3::parameterDisplay(char*res)
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
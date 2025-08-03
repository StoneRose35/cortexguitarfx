
#include "pipicofx/005_Delay.hpp"
extern "C" {
#include "stringFunctions.h"
#include "audio/gainstage.h"
#include "pipicofx/delayMemoryHandler.h"
}
using namespace PiPicoFX;

int16_t Delay::Delay::processSample(int16_t sampleIn)
{
    sampleIn = delayLineProcessSample(sampleIn, &this->delay);
    sampleIn = gainStageProcessSample(sampleIn,&this->presetVolume);
    return sampleIn;
}

void Delay::Delay::setup()
{
    initDelay(&this->delay,mallocDelayMemory(DELAY_LINE_LENGTH<<1),DELAY_LINE_LENGTH);
    this->addParameter(new Param1(this));
    this->addParameter(new Param2(this));
    this->addParameter(new Param3(this));
    this->addParameter(new Param4(this));

}

void Delay::Param1::parameterCallback(uint16_t val)
{
    int32_t wVal;
    wVal = val;
    wVal <<= 4;
    this->pData->delay.delayInSamples = wVal;
    this->rawValue = val; 
}

void Delay::Param1::parameterDisplay(char*res)
{
    int16_t dval;
    dval = this->pData->delay.delayInSamples/48; // in ms
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
void Delay::Param2::parameterCallback(uint16_t val)
{
    uint32_t wVal;
    wVal = val;
    wVal <<= 3;
    this->pData->delay.feedback=(int16_t)wVal;
    this->rawValue = val; 
}

void Delay::Param2::parameterDisplay(char*res)
{
    Int16ToChar(pData->delay.feedback/328,res);
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
void Delay::Param3::parameterCallback(uint16_t val)
{
    int16_t wVal;
    wVal = val;
    wVal <<= 3;
    this->pData->delay.mix = wVal;
    this->rawValue = val; 
}

void Delay::Param3::parameterDisplay(char*res)
{
    Int16ToChar(pData->delay.mix/328,res);
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
    pData->presetVolume.gain = val >> 2; // 0 to 1024
    this->rawValue = val; 
}

void Delay::Param4::parameterDisplay(char*res)
{
    int16_t dVal;
    dVal = pData->presetVolume.gain*39; // percent with two decimal points
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
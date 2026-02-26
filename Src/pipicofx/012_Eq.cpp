
#include "pipicofx/012_Eq.hpp"
extern "C" {
#include "stringFunctions.h"
#include "audio/gainstage.h"
#include "romfunc.h"
}
using namespace PiPicoFX;

int16_t Eq::Eq::processSample(int16_t sampleIn)
{
    int16_t newIn=0;
    if (this->isOn())
    {
        newIn = sampleIn;
    }
    newIn = gainStageProcessSample(newIn,&this->presetVolume);
    newIn = threeBandEqProcessSample(newIn,&this->eq);
    if(!this->isOn())
    {
        return (sampleIn + newIn);
    }
    return newIn;
}

void Eq::Eq::setup()
{
    initThreeBandEq(&this->eq);
    this->addParameter(new Param1(this));
    this->addParameter(new Param2(this));
    this->addParameter(new Param3(this));
    this->addParameter(new Param4(this));

}

void Eq::Param1::parameterCallback(uint16_t val)
{
    this->pData->eq.lowFactor = (-(1 << 12)) + (val << 3);
    this->rawValue = val; 
}

void Eq::Param1::parameterDisplay(char*res)
{
    float fFactor;
    fFactor = int2float((int32_t)this->pData->eq.lowFactor);
    fFactor /= 40.96f;
    decimalInt16ToChar((int16_t)float2int(fFactor),res,2);
}
void Eq::Param2::parameterCallback(uint16_t val)
{
    this->pData->eq.midFactor = (-(1 << 12)) + (val << 3);
    this->rawValue = val; 
}

void Eq::Param2::parameterDisplay(char*res)
{
    float fFactor;
    fFactor = int2float((int32_t)this->pData->eq.midFactor);
    fFactor /= 40.96f;
    decimalInt16ToChar((int16_t)float2int(fFactor),res,2);
}
void Eq::Param3::parameterCallback(uint16_t val)
{
    this->pData->eq.highFactor = (-(1 << 12)) + (val << 3);
    this->rawValue = val; 
}

void Eq::Param3::parameterDisplay(char*res)
{
    float fFactor;
    fFactor = int2float((int32_t)this->pData->eq.highFactor);
    fFactor /= 40.96f;
    decimalInt16ToChar((int16_t)float2int(fFactor),res,2);
}
void Eq::Param4::parameterCallback(uint16_t val)
{
    this->pData->presetVolume.gain = val >> 2; // 0 to 1024
    this->rawValue = val; 
}

void Eq::Param4::parameterDisplay(char*res)
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
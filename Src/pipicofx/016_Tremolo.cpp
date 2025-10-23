
#include "pipicofx/016_Tremolo.hpp"
extern "C" {
#include "stringFunctions.h"
#include "audio/gainstage.h"
#include "memoryRegions.h"
}
using namespace PiPicoFX;

__ITCM_CODE
float Tremolo::Tremolo::processSample(float sampleIn)
{
    sampleIn = gainStageProcessSample(sampleIn,&this->presetVolume);
    return tremoloProcessSample(sampleIn,&this->tremolo);
}

void Tremolo::Tremolo::setup()
{
    initTremolo(&this->tremolo);
    this->addParameter(new Param1(this));
    this->addParameter(new Param2(this));
    this->addParameter(new Param3(this));
    this->addParameter(new Param4(this));
    this->addParameter(new Param5(this));

}

void Tremolo::Param1::parameterCallback(uint16_t val)
{
    float rateVal = ((float)val)/200.f + 0.2f;
    phaseDistortedSineSquareSetFrequency(rateVal,&this->pData->tremolo.modulator);
    this->rawValue = val; 
}

void Tremolo::Param1::parameterDisplay(char*res)
{
    float f = phaseDistortedSineSquareGetFrequency(&this->pData->tremolo.modulator)*8.0f;
    uint16_t intf = (uint16_t)f;
    fixedPointInt16ToChar(res,intf,3);
    appendToString(res," Hz");
}
void Tremolo::Param2::parameterCallback(uint16_t val)
{
    this->pData->tremolo.depth = val << 3;
    this->rawValue = val; 
}

void Tremolo::Param2::parameterDisplay(char*res)
{
    int16_t depth = this->pData->tremolo.depth;
    Int16ToChar(depth/328,res);
    appendToString(res,"%");
}
void Tremolo::Param3::parameterCallback(uint16_t val)
{
    this->pData->tremolo.modulator.squareRatio = val >> 4;
    this->rawValue = val; 
}

void Tremolo::Param3::parameterDisplay(char*res)
{
    Int16ToChar(this->pData->tremolo.modulator.squareRatio,res);
}
void Tremolo::Param4::parameterCallback(uint16_t val)
{
    this->pData->tremolo.modulator.pulseWidth = ((int16_t)val - 2048) << 4;
    phaseDistortedSineSquarePulseWidth(this->pData->tremolo.modulator.pulseWidth,&this->pData->tremolo.modulator);
    this->rawValue = val; 
}

void Tremolo::Param4::parameterDisplay(char*res)
{
    Int16ToChar(this->pData->tremolo.modulator.pulseWidth,res);
}
void Tremolo::Param5::parameterCallback(uint16_t val)
{
    pData->presetVolume.gain = ((float)val)/1024.0f; // 0.0f up to 4.0f
    rawValue = val;
}

void Tremolo::Param5::parameterDisplay(char*res)
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
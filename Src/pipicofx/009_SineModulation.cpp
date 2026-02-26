
#include "pipicofx/009_SineModulation.hpp"
extern "C" {
#include "stringFunctions.h"
#include "audio/gainstage.h"
#include "pipicofx/delayMemoryHandler.h"
}
using namespace PiPicoFX;

int16_t SineModulation::SineModulation::processSample(int16_t sampleIn)
{
    int16_t newIn=0;
    if (this->isOn())
    {
        newIn = sampleIn;
    }
    newIn >>= 1;
    newIn = gainStageProcessSample(newIn,&this->presetVolume);
    newIn = sineChorusInterpolatedProcessSample(newIn,&this->sineChorus);
    if(!this->isOn())
    {
        return (sampleIn + newIn);
    }
    return newIn;
}

void SineModulation::SineModulation::setup()
{
    initSineChorus(&this->sineChorus,mallocDelayMemory(SINE_CHORUS_DELAY_SIZE<<1));
    this->addParameter(new Param1(this));
    this->addParameter(new Param2(this));
    this->addParameter(new Param3(this));
    this->addParameter(new Param4(this));
    this->addParameter(new Param5(this));
    this->addParameter(new Param6(this));
}

SineModulation::SineModulation::~SineModulation()
{
    freeDelayMemory(this->sineChorus.delayBuffer);
}

void SineModulation::Param1::parameterCallback(uint16_t val)
{
    // map 0 - 4095 to 1 1000
    this->rawValue = val; 
    val = ((val*250) >> 10) + 1;
    sineChorusSetFrequency(val,&this->pData->sineChorus);
}

void SineModulation::Param1::parameterDisplay(char*res)
{
    decimalInt16ToChar(this->pData->sineChorus.frequency,res,2);
    appendToString(res," Hz");
}
void SineModulation::Param2::parameterCallback(uint16_t val)
{
    this->rawValue = val; 
        // map to 0 to 255
    val >>= 4;
    pData->sineChorus.depth = (uint8_t)val;
}

void SineModulation::Param2::parameterDisplay(char*res)
{
    int16_t dVal;
    dVal = this->pData->sineChorus.depth; 
    Int16ToChar(dVal,res);
}
void SineModulation::Param3::parameterCallback(uint16_t val)
{
    this->pData->sineChorus.mix = val << 3;
    this->rawValue = val; 
}

void SineModulation::Param3::parameterDisplay(char*res)
{
    Int16ToChar(this->pData->sineChorus.mix/328,res);
    appendToString(res,"%");
}
void SineModulation::Param4::parameterCallback(uint16_t val)
{
    this->pData->sineChorus.offset = 4 + (val >> 2);
    this->rawValue = val; 
}

void SineModulation::Param4::parameterDisplay(char*res)
{
    uint16_t msValue;
    msValue = (uint16_t)(((uint32_t)this->pData->sineChorus.offset*213) >> 10);
    decimalUInt16ToChar(msValue,res,1);
    appendToString(res, " ms");
}
void SineModulation::Param5::parameterCallback(uint16_t val)
{
    this->pData->sineChorus.feedback = val << 3;
    this->rawValue = val; 
}

void SineModulation::Param5::parameterDisplay(char*res)
{
    Int16ToChar(this->pData->sineChorus.feedback/328,res);
    appendToString(res,"%");
}
void SineModulation::Param6::parameterCallback(uint16_t val)
{
    this->pData->presetVolume.gain = val >> 2; // 0 to 1024
    this->rawValue = val; 
}

void SineModulation::Param6::parameterDisplay(char*res)
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

#include "pipicofx/012_Eq.hpp"
extern "C" {
#include "stringFunctions.h"
#include "audio/gainstage.h"
#include "memoryRegions.h"
}
using namespace PiPicoFX;

__ITCM_CODE
float Eq::Eq::processSample(float sampleIn)
{
    float newIn=0.0f;
    if (this->isOn())
    {
        newIn = sampleIn;
    }
    newIn = gainStageProcessSample(newIn,&this->presetVolume);
    newIn = threeBandEqProcessSample(newIn,&this->eq);
    if (!this->isOn())
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

/*
__QSPI_CODE
void fxProgramReset(void*data)
{
    FxProgram14DataType* pData= (FxProgram14DataType*)data;
    threeBandEqReset(&pData->eq);   
}*/

void Eq::Param1::parameterCallback(uint16_t val) // low
{
    pData->eq.lowFactor = (float)val/512.0f-1.0f;
    rawValue = val;
}

void Eq::Param1::parameterDisplay(char*res)
{
    decimalInt16ToChar((int16_t)(pData->eq.lowFactor*100.f),res,2);
}

void Eq::Param2::parameterCallback(uint16_t val) // mid
{
    pData->eq.midFactor = (float)val/512.0f-1.0f;
    rawValue = val;
}

void Eq::Param2::parameterDisplay(char*res)
{
    decimalInt16ToChar((int16_t)(pData->eq.midFactor*100.f),res,2);
}

void Eq::Param3::parameterCallback(uint16_t val) // high
{
    pData->eq.highFactor = (float)val/512.0f-1.0f;
    rawValue = val;
}

void Eq::Param3::parameterDisplay(char*res)
{
    decimalInt16ToChar((int16_t)(pData->eq.highFactor*100.f),res,2);
}

void Eq::Param4::parameterCallback(uint16_t val)
{
    pData->presetVolume.gain = ((float)val)/1024.0f; // 0.0f up to 4.0f
    rawValue = val;
}

void Eq::Param4::parameterDisplay(char*res)
{
    uint16_t dVal;
    dVal=(uint16_t)(pData->presetVolume.gain*10000.0f);
    decimalUInt16ToChar(dVal,res,2);
    appendToString(res,"%");
}

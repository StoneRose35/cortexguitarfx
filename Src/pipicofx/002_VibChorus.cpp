#include "pipicofx/002_VibChorus.hpp"
extern "C" {
#include "stringFunctions.h"
#include "audio/gainstage.h"
#include "pipicofx/delayMemoryHandler.h"
#include "memoryRegions.h"
}
using namespace PiPicoFX;

__ITCM_CODE
float VibChorus::VibChorus::processSample(float sampleIn)
{
    float newIn=0.0f;
    if (this->isOn())
    {
        newIn = sampleIn;
    }
    newIn = simpleChorusProcessSample(newIn,&chorusData);
    newIn =  gainStageProcessSample(newIn,&presetVolume);
    if(!this->isOn())
    {
        return (sampleIn + newIn);
    }
    return newIn;
}

void VibChorus::Param1::parameterCallback(uint16_t val) // frequency
{
    // map 0 - 4095 to 1 1000
    val = ((val*250) >> 10) + 1;
    simpleChorusSetFrequency(val,&pData->chorusData);
    rawValue = val;
}

void VibChorus::Param1::parameterDisplay(char*res)
{
    decimalInt16ToChar(pData->chorusData.frequency,res,2);
    appendToString(res," Hz");
}

void VibChorus::Param2::parameterCallback(uint16_t val) // depth
{
    rawValue = val;
    // map to 0 to 255
    val >>= 4;
    pData->chorusData.depth = (uint8_t)val;

}

void VibChorus::Param2::parameterDisplay(char*res)
{
    int16_t dVal;
    dVal = (pData->chorusData.depth*100) >> 8; ///164;
    Int16ToChar(dVal,res);
    appendToString(res,"%");
}

void VibChorus::Param3::parameterCallback(uint16_t val) // mix
{
    pData->chorusData.mix = ((float)val)/4095.0f;
    rawValue = val;
}

void VibChorus::Param3::parameterDisplay(char*res)
{
    int16_t dVal;
    dVal = (int16_t)(pData->chorusData.mix*100.0f);
    Int16ToChar(dVal,res);
    appendToString(res,"%");
}

void VibChorus::Param4::parameterCallback(uint16_t val)
{
    pData->presetVolume.gain = ((float)val)/1024.0f; // 0.0f up to 4.0f
    rawValue = val;
};


void VibChorus::Param4::parameterDisplay(char* res)
{
    uint16_t dVal;
    dVal=(uint16_t)(pData->presetVolume.gain*10000.0f);
    decimalUInt16ToChar(dVal,res,2);
    appendToString(res,"%");
};

void VibChorus::VibChorus::setup()
{
    initSimpleChorus(&chorusData);
    this->addParameter(new Param1(this));
    this->addParameter(new Param2(this));
    this->addParameter(new Param3(this));
    this->addParameter(new Param4(this));
}

VibChorus::VibChorus::~VibChorus()
{
    freeDelayMemory(this->chorusData.delayBuffer);
}


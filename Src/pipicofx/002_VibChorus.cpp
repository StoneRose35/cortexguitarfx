#include "pipicofx/002_VibChorus.hpp"
extern "C" {
#include "stringFunctions.h"
#include "audio/gainstage.h"
}
using namespace PiPicoFX;

int16_t VibChorus::VibChorus::processSample(int16_t sampleIn)
{
    sampleIn >>= 1;
    sampleIn = simpleChorusProcessSample(sampleIn,&chorusData);
    return gainStageProcessSample(sampleIn,&presetVolume);
}

void VibChorus::Param1::parameterCallback(uint16_t val)
{
    val = ((val*250) >> 10) + 1;
    simpleChorusSetFrequency(val,&pData->chorusData);
    this->rawValue = val;
}

void VibChorus::Param2::parameterCallback(uint16_t val)
{
    // map to 0 to 255
    val >>= 4;
    pData->chorusData.depth = (uint8_t)val;
    rawValue = val;
}

void VibChorus::Param3::parameterCallback(uint16_t val)
{
    // map to 0 to 255
    val >>= 4;
    pData->chorusData.mix = (uint8_t)val;
    rawValue = val;
}

void VibChorus::Param4::parameterCallback(uint16_t val)
{
    pData->presetVolume.gain = val >> 2; // 0 to 1024
    rawValue=val;
}

void VibChorus::Param1::parameterDisplay(char*res)
{
    decimalInt16ToChar(pData->chorusData.frequency,res,2);
    appendToString(res," Hz");
}

void VibChorus::Param2::parameterDisplay(char*res)
{
    int16_t dVal;
    dVal = (pData->chorusData.depth*100) >> 8;
    Int16ToChar(dVal,res);
    appendToString(res,"%");
}

void VibChorus::Param3::parameterDisplay(char*res)
{
    int16_t dVal;
    dVal = (pData->chorusData.mix*100) >> 8;
    Int16ToChar(dVal,res);
    appendToString(res,"%");
}

void VibChorus::Param4::parameterDisplay(char*res)
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


void VibChorus::VibChorus::setup()
{
    initSimpleChorus(&chorusData);
    this->addParameter(new Param1(this));
    this->addParameter(new Param2(this));
    this->addParameter(new Param3(this));
    this->addParameter(new Param4(this));
}
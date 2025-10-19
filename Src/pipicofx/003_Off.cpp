#include "pipicofx/003_Off.hpp"
extern "C" {
#include "stringFunctions.h"
#include "audio/gainstage.h"
#include "memoryRegions.h"
}
using namespace PiPicoFX;


float Off::Off::processSample(float sampleIn)
{
    return gainStageProcessSample(sampleIn,&presetVolume);
}


void Off::Param1::parameterCallback(uint16_t val)
{
    pData->presetVolume.gain = val >> 2; // 0 to 1024
    this->rawValue = val;
}


void Off::Param1::parameterDisplay(char*res)
{
    int16_t dVal;
    dVal=(int16_t)(pData->presetVolume.gain*100.0f);
    Int16ToChar(dVal,res);
    appendToString(res,"%");
}


void Off::Off::setup()
{
    this->addParameter(new Param1(this));
}

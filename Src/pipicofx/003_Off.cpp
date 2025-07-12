#include "pipicofx/003_Off.hpp"
extern "C" {
#include "stringFunctions.h"
#include "audio/gainstage.h"
}
using namespace PiPicoFX;

int16_t Off::Off::processSample(int16_t sampleIn)
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
    dVal = pData->presetVolume.gain*39; // percent with two decimal points
    decimalUInt16ToChar(dVal,res,2);
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


void Off::Off::setup()
{
    this->addParameter(new Param1(this));
}
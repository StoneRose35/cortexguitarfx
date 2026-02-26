
#include "pipicofx/004_MonsterCrusher.hpp"
extern "C" {
#include "stringFunctions.h"
#include "audio/gainstage.h"
}
using namespace PiPicoFX;

int16_t MonsterCrusher::MonsterCrusher::processSample(int16_t sampleIn)
{
    int16_t newIn=0;
    if (this->isOn())
    {
        newIn = sampleIn;
    }
    newIn = bitCrusherProcessSample(newIn,&this->bitcrusher);
    newIn = gainStageProcessSample(newIn,&this->presetVolume);
    if(!this->isOn())
    {
        return (sampleIn + newIn);
    }
    return newIn;
}

void MonsterCrusher::MonsterCrusher::setup()
{
    this->addParameter(new Param1(this));
    this->addParameter(new Param2(this));
}

void MonsterCrusher::Param1::parameterCallback(uint16_t val)
{
    uint16_t resolution;
    resolution = (4096 - val) >> 8;
    if (resolution==0)
    {
        resolution = 1;
    }
    setBitMask((uint8_t)resolution,&this->pData->bitcrusher);
    this->rawValue = val; 
}

void MonsterCrusher::Param1::parameterDisplay(char*res)
{
    uint8_t nbits = (4096-this->rawValue) >> 8;
    if (nbits==0)
    {
        nbits = 1;
    }
    UInt8ToChar(nbits,res);
    appendToString(res,"-bits");
}
void MonsterCrusher::Param2::parameterCallback(uint16_t val)
{
    pData->presetVolume.gain = val >> 2; // 0 to 1024
    this->rawValue = val; 
}

void MonsterCrusher::Param2::parameterDisplay(char*res)
{
    uint16_t dVal;
    dVal = this->pData->presetVolume.gain*39; // percent with two decimal points
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

#include "pipicofx/004_MonsterCrusher.hpp"
extern "C" {
#include "stringFunctions.h"
#include "audio/gainstage.h"
#include "memoryRegions.h"
}
using namespace PiPicoFX;

__ITCM_CODE
float MonsterCrusher::MonsterCrusher::processSample(float sampleIn)
{
    float newIn=0.0f;
    if (this->isOn())
    {
        newIn = sampleIn;
    }
    newIn = bitCrusherProcessSample(newIn,&this->bitcrusher);
    newIn = gainStageProcessSample(newIn,&this->presetVolume);
    if (!this->isOn())
    {
        return (sampleIn + newIn);
    }
    return newIn;
}

void MonsterCrusher::Param1::parameterCallback(uint16_t val) // set bit mask
{
    uint32_t resolution;
    resolution = (4096 - val)*24;
    resolution >>= 12;
    if (resolution==0)
    {
        resolution = 1;
    }
    rawValue = val;
    setBitMask((uint8_t)resolution,&pData->bitcrusher);
}

void MonsterCrusher::Param1::parameterDisplay(char*res)
{  
    uint8_t nbits = ((4096 - rawValue)*24) >> 12;
    if (nbits==0)
    {
        nbits = 1;
    }
    UInt8ToChar(nbits,res);
    appendToString(res,"-bits");
}

void MonsterCrusher::Param2::parameterCallback(uint16_t val)
{
    pData->presetVolume.gain = ((float)val)/1024.0f; // 0.0f up to 4.0f
    rawValue = val;
}

void MonsterCrusher::Param2::parameterDisplay(char*res)
{
    uint16_t dVal;
    dVal=(uint16_t)(pData->presetVolume.gain*10000.0f);
    decimalUInt16ToChar(dVal,res,2);
    appendToString(res,"%");
}

void MonsterCrusher::MonsterCrusher::setup()
{
    this->addParameter(new Param1(this));
    this->addParameter(new Param2(this));
    FxProgram::setup();
}

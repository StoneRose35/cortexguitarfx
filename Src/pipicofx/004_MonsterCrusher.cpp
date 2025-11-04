
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
    sampleIn = bitCrusherProcessSample(sampleIn,&this->bitcrusher);
    sampleIn = gainStageProcessSample(sampleIn,&this->presetVolume);
    return sampleIn;
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
}

/*
FxProgram5DataType fxProgram5data = {
    .bitcrusher = {
        .bitmask = 0x800000
    }
};

FxProgramType fxProgram5 = {
    .name = "Monstercrusher       ",
    .nParameters=1,
    .parameters = {
        {
            .name = "Bit Reduction  ",
            .control=0,
            .increment=64,
            .rawValue=0,
            .getParameterDisplay=&fxProgram5Param1Display,
            .getParameterValue=0,
            .setParameter=&fxProgram5Param1Callback
        }
    },
    .processSample = &fxProgram5processSample,
    .setup = &fxProgram5Setup,
    .reset = 0,
    .data = (void*)&fxProgram5data
};
*/
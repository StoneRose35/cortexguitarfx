
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

__QSPI_CODE
void MonsterCrusher::Param1::parameterCallback(uint16_t val) // set bit mask
{
    uint32_t resolution;
    resolution = (4096 - val)*24;
    resolution >>= 12;
    pData->resolution = (uint8_t)resolution;
    rawValue = val;
    setBitMask((uint8_t)resolution,&pData->bitcrusher);
}

__QSPI_CODE
void MonsterCrusher::Param1::parameterDisplay(char*res)
{
    uint8_t resolution;    
    resolution = pData->resolution;
    UInt8ToChar(resolution,res);
}

__QSPI_CODE
void MonsterCrusher::Param2::parameterCallback(uint16_t val)
{
    pData->presetVolume.gain = val >> 2; // 0 to 1024
    this->rawValue = val; 
}

__QSPI_CODE
void MonsterCrusher::Param2::parameterDisplay(char*res)
{
    int16_t dVal;
    dVal=(int16_t)(pData->presetVolume.gain*100.0f);
    Int16ToChar(dVal,res);
    appendToString(res,"%");
}

__QSPI_CODE
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
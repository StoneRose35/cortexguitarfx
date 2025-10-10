
#include "pipicofx/010_Reverb2.hpp"
extern "C" {
#include "stringFunctions.h"
#include "audio/gainstage.h"
#include "pipicofx/delayMemoryHandler.h"
#include "memoryRegions.h"
}
using namespace PiPicoFX;

__ITCM_CODE
float Reverb2::Reverb2::processSample(float sampleIn)
{
    sampleIn = gainStageProcessSample(sampleIn,&this->presetVolume);
    return reverb2ProcessSample(sampleIn,&this->reverb);
}

__QSPI_CODE
void Reverb2::Param1::parameterCallback(uint16_t val) // reverb time
{
    pData->reverb.decay = (float)val/4095.0f;
    rawValue = val;
}

__QSPI_CODE
void Reverb2::Param1::parameterDisplay(char*res)
{
    Int16ToChar((int16_t)(pData->reverb.decay*100.0f), res);
}

__QSPI_CODE
void Reverb2::Param2::parameterCallback(uint16_t val) // Mix
{
    pData->reverb.mix=val/4095.0f;
    rawValue = val;
}

__QSPI_CODE
void Reverb2::Param2::parameterDisplay(char*res)
{
    Int16ToChar((int16_t)(pData->reverb.mix*100.0f),res);
    appendToString(res,"%");
}

__QSPI_CODE
void Reverb2::Param3::parameterCallback(uint16_t val)
{
    pData->presetVolume.gain = val >> 2; // 0 to 1024
    this->rawValue = val;
}

__QSPI_CODE
void Reverb2::Param3::parameterDisplay(char*res)
{
    int16_t dVal;
    dVal=(int16_t)(pData->presetVolume.gain*100.0f);
    Int16ToChar(dVal,res);
    appendToString(res,"%");
}

__QSPI_CODE
void Reverb2::Reverb2::setup()
{
    initReverb2(&this->reverb,mallocDelayMemory(24576<<1));
    this->addParameter(new Param1(this));
    this->addParameter(new Param2(this));
    this->addParameter(new Param3(this));
}

__QSPI_CODE
Reverb2::Reverb2::~Reverb2()
{
    freeDelayMemory(this->reverb.aps[0].delayLineIn);
}
/*
FxProgram12DataType fxProgram12data;

FxProgramType fxProgram12 = {
    .name = "Allpass Reverb",
    .nParameters=2,
    .parameters = {
        {
            .name = "Decay          ",
            .control=0,
            .increment=32,
            .rawValue=0,
            .getParameterDisplay=&fxProgramParam1Display,
            .getParameterValue=0,
            .setParameter=&fxProgramParam1Callback
        },
        {
            .name = "Mix            ",
            .control=1,
            .increment=32,
            .rawValue=0,
            .getParameterDisplay=&fxProgramParam2Display,
            .getParameterValue=0,
            .setParameter=&fxProgramParam2Callback
        }
    },
    .processSample = &fxProgramprocessSample,
    .setup = &fxProgramSetup,
    .reset = 0,
    .data = (void*)&fxProgram12data
};
*/
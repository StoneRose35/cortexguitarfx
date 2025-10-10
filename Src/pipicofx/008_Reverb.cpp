
#include "pipicofx/008_Reverb.hpp"
extern "C" {
#include "stringFunctions.h"
#include "pipicofx/delayMemoryHandler.h"
#include "audio/gainstage.h"
#include "memoryRegions.h"
}
using namespace PiPicoFX;

__ITCM_CODE
 float Reverb::Reverb::processSample(float sampleIn)
{
    sampleIn = gainStageProcessSample(sampleIn,&this->presetVolume);
    return reverbProcessSample(sampleIn,&this->reverb);
}

__QSPI_CODE
 void Reverb::Param1::parameterCallback(uint16_t val) // reverb time
{
    pData->reverbTime = ((float)val)/4095.0f*(2.0f-0.1f) + 0.1f;
    setReverbTime(pData->reverbTime,&pData->reverb);
    rawValue = val;
}

__QSPI_CODE
 void Reverb::Param1::parameterDisplay(char*res)
{
    int16_t reverbms = (int16_t)(pData->reverbTime*1000.0f);
    Int16ToChar(reverbms,res);
    appendToString(res," ms");
}

__QSPI_CODE
 void Reverb::Param2::parameterCallback(uint16_t val) // Mix
{
    float fval=(float)val/4095.0;
    pData->reverb.mix=fval;
    rawValue = val;
}

__QSPI_CODE
 void Reverb::Param2::parameterDisplay(char*res)
{
    int16_t mixpercent = (int16_t)(pData->reverb.mix*100.0);
    Int16ToChar(mixpercent,res);
    appendToString(res,"%");
}

__QSPI_CODE
 void Reverb::Param3::parameterCallback(uint16_t val) // Parameter
{
    pData->reverb.paramNr=(val >> 10);
    freeDelayMemory(pData->reverb.delayPointers[0]);
    initReverb(&pData->reverb,pData->reverbTime,mallocDelayMemory(pData->getDelayMemoryUseage()));
    rawValue = val;
}

__QSPI_CODE
 void Reverb::Param3::parameterDisplay(char*res)
{
    *res=0;
    appendToString(res,getReverbParameterSetName(&pData->reverb));
}

__QSPI_CODE
void Reverb::Param4::parameterCallback(uint16_t val)
{
    pData->presetVolume.gain = val >> 2; // 0 to 1024
    this->rawValue = val;
}

__QSPI_CODE
void Reverb::Param4::parameterDisplay(char*res)
{
    int16_t dVal;
    dVal=(int16_t)(pData->presetVolume.gain*100.0f);
    Int16ToChar(dVal,res);
    appendToString(res,"%");
}

__QSPI_CODE
 void Reverb::Reverb::setup()
{
    initReverb(&this->reverb,this->reverbTime,mallocDelayMemory(this->getDelayMemoryUseage()));
    this->addParameter(new Param1(this));
    this->addParameter(new Param2(this));
    this->addParameter(new Param3(this));
    this->addParameter(new Param4(this));
}

__QSPI_CODE
Reverb::Reverb::~Reverb()
{
    freeDelayMemory(this->reverb.delayPointers[0]);
}


/*
FxProgram10DataType fxProgram10data=
{
    .reverbTime=0.3f,
    .reverb.paramNr=0
};


FxProgramType fxProgram10 = {
    .name = "Reverb",
    .nParameters=3,
    .parameters = {
        {
            .name = "Time           ",
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
        },
        {
            .name = "ParameterSet   ",
            .control=2,
            .increment=1024,
            .rawValue=0,
            .getParameterDisplay=&fxProgramParam3Display,
            .getParameterValue=0,
            .setParameter=&fxProgramParam3Callback
        }
    },
    .processSample = &fxProgramprocessSample,
    .setup = &fxProgramSetup,
    .reset = 0,
    .data = (void*)&fxProgram10data
};

*/
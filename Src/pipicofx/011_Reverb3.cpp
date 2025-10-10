
#include "pipicofx/011_Reverb3.hpp"
extern "C" {
#include "math.h"
#include "stringFunctions.h"
#include "pipicofx/delayMemoryHandler.h"
#include "audio/gainstage.h"
#include "memoryRegions.h"
}
using namespace PiPicoFX;

__ITCM_CODE
float Reverb3::Reverb3::processSample(float sampleIn)
{
    float reverberatedSample;
    sampleIn = gainStageProcessSample(sampleIn,&this->presetVolume);
    reverberatedSample = reverb3processSample(sampleIn,&this->reverb);
    return ((1.0f - this->mix)*sampleIn) + (this->mix*reverberatedSample);
}

__QSPI_CODE
void Reverb3::Param1::parameterCallback(uint16_t val) // decay / delay feedback
{
    float intermVal;
    intermVal = (float)val/4095.0f;
    if (intermVal > 0x7FFD/32768.0f)
    {
        intermVal = 0x7FFD/32768.0f;
    }
    pData->reverb.delay.feedback = intermVal;
    rawValue = val;
}

__QSPI_CODE
void Reverb3::Param1::parameterDisplay(char*res)
{
    float ffbk;
    int16_t t60;
    ffbk = (float)(pData->reverb.delay.feedback);
    if (ffbk < 0.0000305f)
    {
        t60=0;
    }
    else
    {
        t60=(int16_t)(-589.03004f/logf(ffbk)); // t60 in ms
    }
    Int16ToChar(t60,res);
    appendToString(res," ms");
}

__QSPI_CODE
void Reverb3::Param2::parameterCallback(uint16_t val) // mix
{
    pData->mix=val/4095.0f;
    rawValue = val;
}

__QSPI_CODE
void Reverb3::Param2::parameterDisplay(char*res)
{
    int16_t mixpercent = (int16_t)(pData->mix*100.0f);
    Int16ToChar(mixpercent,res);
    appendToString(res,"%");
}

__QSPI_CODE
void Reverb3::Param3::parameterCallback(uint16_t val)
{
    pData->presetVolume.gain = val >> 2; // 0 to 1024
    this->rawValue = val;
}

__QSPI_CODE
void Reverb3::Param3::parameterDisplay(char*res)
{
    int16_t dVal;
    dVal=(int16_t)(pData->presetVolume.gain*100.0f);
    Int16ToChar(dVal,res);
    appendToString(res,"%");
}

__QSPI_CODE
void Reverb3::Reverb3::setup()
{
    initReverb3(&this->reverb,mallocDelayMemory((16*DIFFUSOR_SIZE + 4096)<<1));
    this->addParameter(new Param1(this));
    this->addParameter(new Param2(this));
    this->addParameter(new Param3(this));
}

Reverb3::Reverb3::~Reverb3()
{
    freeDelayMemory(this->reverb.diffusers[0].delayPointers[0]);
}

/*
FxProgram13DataType fxProgram13data;

FxProgramType fxProgram13 = {
    .name = "Hadamard Reverb",
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
    .data = (void*)&fxProgram13data
};
*/
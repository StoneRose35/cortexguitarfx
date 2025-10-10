
#include "pipicofx/012_Eq.hpp"
extern "C" {
#include "stringFunctions.h"
#include "audio/gainstage.h"
#include "memoryRegions.h"
}
using namespace PiPicoFX;

__ITCM_CODE
float Eq::Eq::processSample(float sampleIn)
{
    sampleIn = gainStageProcessSample(sampleIn,&this->presetVolume);
    return threeBandEqProcessSample(sampleIn,&this->eq);
}

__QSPI_CODE
void Eq::Eq::setup()
{
    initThreeBandEq(&this->eq);
    this->addParameter(new Param1(this));
    this->addParameter(new Param2(this));
    this->addParameter(new Param3(this));
    this->addParameter(new Param4(this));
}

/*
__QSPI_CODE
void fxProgramReset(void*data)
{
    FxProgram14DataType* pData= (FxProgram14DataType*)data;
    threeBandEqReset(&pData->eq);   
}*/


__QSPI_CODE
void Eq::Param1::parameterCallback(uint16_t val) // low
{
    pData->eq.lowFactor = (float)val/512.0f-1.0f;
    rawValue = val;
}

__QSPI_CODE
void Eq::Param1::parameterDisplay(char*res)
{
    decimalInt16ToChar((int16_t)(pData->eq.lowFactor*100.f),res,2);
}

__QSPI_CODE
void Eq::Param2::parameterCallback(uint16_t val) // mid
{
    pData->eq.midFactor = (float)val/512.0f-1.0f;
    rawValue = val;
}

__QSPI_CODE
void Eq::Param2::parameterDisplay(char*res)
{
    decimalInt16ToChar((int16_t)(pData->eq.midFactor*100.f),res,2);
}

__QSPI_CODE
void Eq::Param3::parameterCallback(uint16_t val) // high
{
    pData->eq.highFactor = (float)val/512.0f-1.0f;
    rawValue = val;
}

__QSPI_CODE
void Eq::Param3::parameterDisplay(char*res)
{
    decimalInt16ToChar((int16_t)(pData->eq.highFactor*100.f),res,2);
}

__QSPI_CODE
void Eq::Param4::parameterCallback(uint16_t val)
{
    pData->presetVolume.gain = val >> 2; // 0 to 1024
    this->rawValue = val;
}

__QSPI_CODE
void Eq::Param4::parameterDisplay(char*res)
{
    int16_t dVal;
    dVal=(int16_t)(pData->presetVolume.gain*100.0f);
    Int16ToChar(dVal,res);
    appendToString(res,"%");
}
/*
FxProgram14DataType fxProgram14data;

FxProgramType fxProgram14 = {
    .name = "3-Band Equalizer",
    .nParameters=3,
    .parameters = {
        {
            .name = "Low           ",
            .control=0,
            .increment=32,
            .rawValue=0,
            .getParameterDisplay=&fxProgramParam1Display,
            .getParameterValue=0,
            .setParameter=&fxProgramParam1Callback
        },
        {
            .name = "Mid            ",
            .control=1,
            .increment=32,
            .rawValue=0,
            .getParameterDisplay=&fxProgramParam2Display,
            .getParameterValue=0,
            .setParameter=&fxProgramParam2Callback
        },
        {
            .name = "High           ",
            .control=2,
            .increment=32,
            .rawValue=0,
            .getParameterDisplay=&fxProgramParam3Display,
            .getParameterValue=0,
            .setParameter=&fxProgramParam3Callback
        }
    },
    .processSample = &fxProgramProcessSample,
    .setup = &fxProgramSetup,
    .reset = &fxProgramReset,
    .data = (void*)&fxProgram14data
};
*/
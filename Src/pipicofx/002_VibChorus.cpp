#include "pipicofx/002_VibChorus.hpp"
extern "C" {
#include "stringFunctions.h"
#include "audio/gainstage.h"
#include "pipicofx/delayMemoryHandler.h"
#include "memoryRegions.h"
}
using namespace PiPicoFX;

__ITCM_CODE
float VibChorus::VibChorus::processSample(float sampleIn)
{
    sampleIn = simpleChorusProcessSample(sampleIn,&chorusData);
    return gainStageProcessSample(sampleIn,&presetVolume);
}

__QSPI_CODE
void VibChorus::Param1::parameterCallback(uint16_t val) // frequency
{
    // map 0 - 4095 to 1 1000
    val = ((val*250) >> 10) + 1;
    simpleChorusSetFrequency(val,&pData->chorusData);
    rawValue = val;
}

__QSPI_CODE
void VibChorus::Param1::parameterDisplay(char*res)
{
    decimalInt16ToChar(pData->chorusData.frequency,res,2);
    appendToString(res," Hz");
}

__QSPI_CODE
void VibChorus::Param2::parameterCallback(uint16_t val) // depth
{
    // map to 0 to 255
    val >>= 4;
    pData->chorusData.depth = (uint8_t)val;
    rawValue = val;
}

__QSPI_CODE
void VibChorus::Param2::parameterDisplay(char*res)
{
    int16_t dVal;
    dVal = (pData->chorusData.depth*100) >> 8; ///164;
    Int16ToChar(dVal,res);
    appendToString(res,"%");
}

__QSPI_CODE
void VibChorus::Param3::parameterCallback(uint16_t val) // mix
{
    pData->chorusData.mix = ((float)val)/4095.0f;
    rawValue = val;
}

__QSPI_CODE
void VibChorus::Param3::parameterDisplay(char*res)
{
    int16_t dVal;
    dVal = (int16_t)(pData->chorusData.mix*100.0f);
    Int16ToChar(dVal,res);
    appendToString(res,"%");
}

__QSPI_CODE
void VibChorus::Param4::parameterCallback(uint16_t val)
{
    pData->presetVolume.gain = ((float)val)/1024.0f; // 0.0f up to 4.0f
};

__QSPI_CODE
void VibChorus::Param4::parameterDisplay(char* res)
{
    int16_t dVal;
    dVal=(int16_t)(pData->presetVolume.gain*100.0f);
    Int16ToChar(dVal,res);
    appendToString(res,"%");
};

__QSPI_CODE
void VibChorus::VibChorus::setup()
{
    initSimpleChorus(&chorusData);
    this->addParameter(new Param1(this));
    this->addParameter(new Param2(this));
    this->addParameter(new Param3(this));
    this->addParameter(new Param4(this));
}


__QSPI_CODE
VibChorus::VibChorus::~VibChorus()
{
    freeDelayMemory(this->chorusData.delayBuffer);
}

/*
FxProgram2DataType fxProgram2data = {
    .chorusData = {
        .mix = 0.5f,
        .frequency = 500,
        .depth = 10
    }
};

FxProgramType fxProgram2 = {
    .name = "Vibrato/Chorus",
    .nParameters=3,
    .processSample = &fxProgram2processSample,
    .parameters = {
        {
            .name = "Frequency      ",
            .control=0,
            .increment = 32,
            .rawValue=0,
            .getParameterDisplay=&fxProgram2Param1Display,
            .getParameterValue=0,
            .setParameter=&fxProgram2Param1Callback
        },
        {
            .name = "Depth          ",
            .control=1,
            .increment = 32,
            .rawValue=0,
            .getParameterDisplay=&fxProgram2Param2Display,
            .getParameterValue=0,
            .setParameter=&fxProgram2Param2Callback
        },
        {
            .name = "Mix            ",
            .control=2,
            .increment=32,
            .rawValue=0,
            .getParameterDisplay=&fxProgram2Param3Display,
            .getParameterValue=0,
            .setParameter=&fxProgram2Param3Callback
        }
    },
    .setup = &fxProgram2Setup,
    .reset=0,
    .data = (void*)&fxProgram2data
};
*/
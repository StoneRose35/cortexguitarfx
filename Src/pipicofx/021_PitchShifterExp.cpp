#include "pipicofx/021_PitchShifterExp.hpp"
extern "C" {
#include "stringFunctions.h"
#include "audio/pitchshifter.h"
#include "audio/gainstage.h"
#include "pipicofx/delayMemoryHandler.h"
#include "memoryRegions.h"
}
using namespace PiPicoFX;

__ITCM_CODE
float PitchShifterExp::PitchShifterExp::processSample(float sampleIn)
{
    float newIn=0.0f;
    float proc=0.0f;
    if (this->isOn())
    {
        newIn = sampleIn;
    }
    proc = firstOrderIirHighpassProcessSample(newIn,&dcRemoval);
    proc = pitchShifter3ProcessSample(proc,&pitchShifter3);
    proc = newIn + mix*(proc - newIn); //(1.0f - mix)*newIn + mix*proc;
    proc = gainStageProcessSample(proc,&presetVolume);
    if (!this->isOn())
    {
        return (sampleIn + proc);
    }
    return proc;
}
void PitchShifterExp::Param1::parameterCallback(uint16_t val) // tune
{
    pData->pitchShifter3.pointerIncrement = (val >> 9) + 1;
    rawValue = val;
}

void PitchShifterExp::Param1::parameterDisplay(char*res)
{
    *res=0;
    switch (this->pData->pitchShifter3.pointerIncrement)
    {
    case 1:
        appendToString(res,"2OctDown");
        break;
    case 2:
        appendToString(res,"OctDown");
        break;
    case 3:
        appendToString(res,"FourthDown");
        break;
    case 4:
        appendToString(res,"NoShift");
        break;
    case 5:
        appendToString(res,"ThirdUp");
        break;
    case 6:
        appendToString(res,"FifthUp");
        break;
    case 7:
        appendToString(res,"Devil666");
        break;
    case 8:
        appendToString(res,"OctUp");
        break;
    default:
        appendToString(res,"ERROR");
        break;
    }
}

void PitchShifterExp::Param2::parameterCallback(uint16_t val) // mix
{
    pData->mix = ((float)val)/4095.0f;
    rawValue = val;
}

void PitchShifterExp::Param2::parameterDisplay(char*res)
{
    int16_t dVal;
    dVal = (int16_t)(pData->mix*100.0f);
    Int16ToChar(dVal,res);
    appendToString(res,"%");
}


void PitchShifterExp::Param3::parameterCallback(uint16_t val)
{
    pData->presetVolume.gain = ((float)val)/1024.0f; // 0.0f up to 4.0f
    rawValue = val;
}


void PitchShifterExp::Param3::parameterDisplay(char*res)
{
    uint16_t dVal;
    dVal=(uint16_t)(pData->presetVolume.gain*10000.0f);
    decimalUInt16ToChar(dVal,res,2);
    appendToString(res,"%");
}


void PitchShifterExp::PitchShifterExp::setup(uint8_t allocateMemory)
{
    this->addParameter(new Param1(this));
    this->addParameter(new Param2(this));
    this->addParameter(new Param3(this));
    FxProgram::setup(allocateMemory);
    iniPitchShifter3(&pitchShifter3);
}

PitchShifterExp::PitchShifterExp::~PitchShifterExp()
{
    freeDelayMemory(this->pitchShifter3.delayMemoryPtr);
}
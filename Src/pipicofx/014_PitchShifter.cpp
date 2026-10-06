
#include "pipicofx/014_PitchShifter.hpp"
extern "C" {
#include "drivers/adc.h"
#include "stringFunctions.h"
#include "audio/gainstage.h"
#include "audio/audiotools.h"
#include "globalConfig.h"
#include "memoryRegions.h"
#include "pipicofx/delayMemoryHandler.h"
}
using namespace PiPicoFX;

__ITCM_CODE
float PitchShifter::PitchShifter::processSample(float sampleIn)
{
    float newIn=0.0f;
    if (this->isOn())
    {
        newIn = sampleIn;
    }
    float processedSample = pitchShifterProcessSample(newIn,&this->pitchShifter);
    float sampleOut= (newIn*(1.0f - this->mix)) + (processedSample*this->mix);
    sampleOut = gainStageProcessSample(sampleOut,&this->presetVolume);
    if (!this->isOn())
    {
        return (sampleIn + sampleOut);
    }
    return sampleOut;
}

void PitchShifter::Param1::parameterCallback(uint16_t val) // low
{
    pData->pitchShifter.pointerIncrement = (val >> 9) + 1;
    rawValue = val;
}

void PitchShifter::Param1::parameterDisplay(char*res)
{
    *res=0;
    switch (this->pData->pitchShifter.pointerIncrement)
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

void PitchShifter::Param2::parameterCallback(uint16_t val) // Mix
{
    pData->mix=val/4095.0f;
    rawValue = val;
}

void PitchShifter::Param2::parameterDisplay(char*res)
{
    Int16ToChar(pData->mix*100.0f,res);
    appendToString(res,"%");
}

void PitchShifter::Param3::parameterCallback(uint16_t val) // BufferSize
{
    uint16_t newVal = 2880 + val;
    this->pData->pitchShifter.grainSize = newVal;
    for (uint8_t c=0;c<this->pData->pitchShifter.readPointerCount;c++)
    {
        this->pData->pitchShifter.readPointers[c] = (-((c*(this->pData->pitchShifter.grainSize/this->pData->pitchShifter.readPointerCount)) << 2))  & ((this->pData->pitchShifter.bufferSize << 2)-1);;
    }
    rawValue = val;
}

void PitchShifter::Param3::parameterDisplay(char*res)
{
    int16_t avgDelayMs=((pData->pitchShifter.grainSize) / (AUDIO_SAMPLING_RATE/1000));
    Int16ToChar(avgDelayMs,res);
    appendToString(res, "ms");
}

void PitchShifter::Param4::parameterCallback(uint16_t val)
{
    pData->pitchShifter.readPointerCount = (val >> 10) + 1;
    deinitPitchshifter(&pData->pitchShifter);
    initPitchshifter(&pData->pitchShifter);
    rawValue = val;
}

void PitchShifter::Param4::parameterDisplay(char*res)
{
    *res  = pData->pitchShifter.readPointerCount + 0x30;
    *(res+1) = 0;
}

void PitchShifter::Param5::parameterCallback(uint16_t val)
{
    pData->presetVolume.gain = ((float)val)/1024.0f; // 0.0f up to 4.0f
    rawValue = val;
}

void PitchShifter::Param5::parameterDisplay(char*res)
{
    uint16_t dVal;
    dVal=(uint16_t)(pData->presetVolume.gain*10000.0f);
    decimalUInt16ToChar(dVal,res,2);
    appendToString(res,"%");
}

void PitchShifter::PitchShifter::setup(uint8_t allocateMemory)
{
    if (allocateMemory)
    {
        initPitchshifter(&this->pitchShifter);
    }
    this->addParameter(new Param1(this));
    this->addParameter(new Param2(this));
    this->addParameter(new Param3(this));
    this->addParameter(new Param4(this));
    this->addParameter(new Param5(this));
    FxProgram::setup(allocateMemory);
}

PitchShifter::PitchShifter::~PitchShifter()
{
    deinitPitchshifter(&this->pitchShifter);
}

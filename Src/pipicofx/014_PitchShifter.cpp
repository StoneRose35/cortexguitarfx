
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
    float processedSample = pitchShifter2ProcessSample(newIn,&this->pitchShifter);
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
    pData->pitchShifter.delayIncrement = (val >> 9) + 1;
    rawValue = val;
}

void PitchShifter::Param1::parameterDisplay(char*res)
{
    *res=0;
    switch (this->pData->pitchShifter.delayIncrement)
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
    uint16_t newVal = (val >> 10)+9;
    if (newVal != pData->pitchShifter.buffersizePowerTwo)
    {
        this->pData->pitchShifter.buffersizePowerTwo=newVal;
        this->pData->pitchShifter.crossFadeWidthPwr2 = newVal-2;
        pData->pitchShifter.buffersizePowerTwo=newVal;
        freeDelayMemory(pData->pitchShifter.delayMemoryPtr);
        initPitchshifter2(&pData->pitchShifter);
    }
    rawValue = val;
}

void PitchShifter::Param3::parameterDisplay(char*res)
{
    int16_t avgDelayMs=((pData->pitchShifter.buffersize >> 1) / (AUDIO_SAMPLING_RATE/1000));
    Int16ToChar(avgDelayMs,res);
    appendToString(res, "ms");
}

void PitchShifter::Param4::parameterCallback(uint16_t val)
{
    pData->presetVolume.gain = ((float)val)/1024.0f; // 0.0f up to 4.0f
    rawValue = val;
}

void PitchShifter::Param4::parameterDisplay(char*res)
{
    uint16_t dVal;
    dVal=(uint16_t)(pData->presetVolume.gain*10000.0f);
    decimalUInt16ToChar(dVal,res,2);
    appendToString(res,"%");
}

void PitchShifter::PitchShifter::setup()
{
    this->addParameter(new Param1(this));
    this->addParameter(new Param2(this));
    this->addParameter(new Param3(this));
    this->addParameter(new Param4(this));

    initPitchshifter2(&this->pitchShifter);

}

PitchShifter::PitchShifter::~PitchShifter()
{
    freeDelayMemory(this->pitchShifter.delayMemoryPtr);
}

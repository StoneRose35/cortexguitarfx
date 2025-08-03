
#include "pipicofx/013_PitchShifter.hpp"
extern "C" {
#include "stringFunctions.h"
#include "audio/gainstage.h"
#include "audio/audiotools.h"
#include "globalConfig.h"
}
using namespace PiPicoFX;

int16_t PitchShifter::PitchShifter::processSample(int16_t sampleIn)
{
    int16_t processedSample = pitchShifter2ProcessSample(sampleIn,&this->pitchShifter,getAudioStatePtr());
    int16_t sampleOut= ((sampleIn)*((1 << 15) - this->mix) >> 15) + ((processedSample)*this->mix >> 15);
    sampleOut = gainStageProcessSample(sampleOut,&this->presetVolume);
    return sampleOut;
}

void PitchShifter::PitchShifter::setup()
{
    this->addParameter(new Param1(this));
    this->addParameter(new Param2(this));
    this->addParameter(new Param3(this));
    this->addParameter(new Param4(this));

    initPitchshifter2(&this->pitchShifter);

}

void PitchShifter::Param1::parameterCallback(uint16_t val)
{
    this->pData->pitchShifter.delayIncrement = (val >> 9) + 1;
    this->rawValue = val; 
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
void PitchShifter::Param2::parameterCallback(uint16_t val)
{
    this->pData->mix=(val << 3);
    this->rawValue = val; 
}

void PitchShifter::Param2::parameterDisplay(char*res)
{
    int16_t mixpercent = (int16_t)(this->pData->mix/328);
    Int16ToChar(mixpercent,res);
    appendToString(res,"%");
}
void PitchShifter::Param3::parameterCallback(uint16_t val)
{
    uint16_t newVal = (val >> 10)+9;
    if (newVal != this->pData->pitchShifter.buffersizePowerTwo)
    {
        this->pData->pitchShifter.buffersizePowerTwo=newVal;
        this->pData->pitchShifter.crossFadeWidthPwr2 = newVal-2;
        deinitPitchshifter2(&pData->pitchShifter);
        initPitchshifter2(&pData->pitchShifter);
    }
    this->rawValue = val; 
}

void PitchShifter::Param3::parameterDisplay(char*res)
{
    int16_t avgDelayMs=((this->pData->pitchShifter.buffersize >> 1) / (AUDIO_SAMPLING_RATE/1000));
    Int16ToChar(avgDelayMs,res);
    appendToString(res, "ms");
}
void PitchShifter::Param4::parameterCallback(uint16_t val)
{
    this->pData->presetVolume.gain = val >> 2; // 0 to 1024
    this->rawValue = val; 
}

void PitchShifter::Param4::parameterDisplay(char*res)
{
    int16_t dVal;
    dVal = this->pData->presetVolume.gain*39; // percent with two decimal points
    decimalInt16ToChar(dVal,res,2);
    for (uint8_t c=0;c<PARAMETER_NAME_MAXLEN-1;c++)
    {
        if(*(res+c)==0)
        {
            *(res+c)='%';
            *(res+c+1)=(char)0;
            break;
        }
    }
}
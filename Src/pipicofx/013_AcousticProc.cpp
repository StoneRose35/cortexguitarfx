
#include "pipicofx/013_AcousticProc.hpp"
extern "C" {
#include "stringFunctions.h"
#include "pipicofx/delayMemoryHandler.h"
#include "audio/gainstage.h"
#include "romfunc.h"
}
using namespace PiPicoFX;

int16_t AcousticProc::AcousticProc::processSample(int16_t sampleIn)
{
    int16_t newIn=0;
    if (this->isOn())
    {
        newIn = sampleIn;
    }
    newIn = threeBandEqProcessSample(newIn,&this->eq);
    newIn = compressor2ProcessSample(newIn,&this->comp);
    newIn = gainStageProcessSample(newIn,&this->postGain);
    newIn = gainStageProcessSample(newIn,&this->presetVolume);
    newIn = reverbProcessSample(newIn,&this->reverb);
    if(!this->isOn())
    {
        return (sampleIn + newIn);
    }
    return newIn;
}

void AcousticProc::AcousticProc::setup()
{
    initThreeBandEq(&this->eq);
    initReverb(&this->reverb,500,mallocDelayMemory(20480<<1));
    this->addParameter(new Param1(this));
    this->addParameter(new Param2(this));
    this->addParameter(new Param3(this));
    this->addParameter(new Param4(this));
    this->addParameter(new Param5(this));
    this->addParameter(new Param6(this));
    this->addParameter(new Param7(this));

}

AcousticProc::AcousticProc::~AcousticProc()
{
    freeDelayMemory(this->reverb.delayPointers[0]);
}

void AcousticProc::Param1::parameterCallback(uint16_t val)
{
    this->pData->eq.lowFactor = (-(1 << 12)) + (val << 3);
    this->rawValue = val; 
}

void AcousticProc::Param1::parameterDisplay(char*res)
{
    float fFactor;
    fFactor = int2float((int32_t)this->pData->eq.lowFactor);
    fFactor /= 40.96f;
    decimalInt16ToChar((int16_t)float2int(fFactor),res,2);
}
void AcousticProc::Param2::parameterCallback(uint16_t val)
{
    this->pData->eq.midFactor = (-(1 << 12)) + (val << 3);
    this->rawValue = val; 
}

void AcousticProc::Param2::parameterDisplay(char*res)
{
    float fFactor;
    fFactor = int2float((int32_t)this->pData->eq.midFactor);
    fFactor /= 40.96f;
    decimalInt16ToChar((int16_t)float2int(fFactor),res,2);
}
void AcousticProc::Param3::parameterCallback(uint16_t val)
{
    this->pData->eq.highFactor = (-(1 << 12)) + (val << 3);
    this->rawValue = val; 
}

void AcousticProc::Param3::parameterDisplay(char*res)
{
    float fFactor;
    fFactor = int2float((int32_t)this->pData->eq.highFactor);
    fFactor /= 40.96f;
    decimalInt16ToChar((int16_t)float2int(fFactor),res,2);
}
void AcousticProc::Param4::parameterCallback(uint16_t val)
{
    int32_t maxGain;
    // map val to a thresshold range of 0 to ~ -60dB /-8599
    this->pData->comp.gainFunction.threshhold = 0x7FFF - (val << 1);
    // compute post Gain according to threshhold set
    maxGain = getMaxGain(&this->pData->comp);
    maxGain = 32767/maxGain; // 0.8 as q15
    maxGain <<= 8;
    this->pData->postGain.gain = (int16_t)maxGain;
    this->rawValue = val; 
}

void AcousticProc::Param4::parameterDisplay(char*res)
{
    int16_t comppercent = (int16_t)(this->rawValue << 3);
    Int16ToChar(comppercent/328,res);
    appendToString(res,"%");
}
void AcousticProc::Param5::parameterCallback(uint16_t val)
{
    this->pData->reverbTime = (((uint32_t)val*1900)>>12) + 100;
    setReverbTime(this->pData->reverbTime,&this->pData->reverb);
    this->rawValue = val; 
}

void AcousticProc::Param5::parameterDisplay(char*res)
{
    Int16ToChar(this->pData->reverbTime,res);
    appendToString(res," ms");
}
void AcousticProc::Param6::parameterCallback(uint16_t val)
{
    this->pData->reverb.mix=(val << 3);
    this->rawValue = val; 
}

void AcousticProc::Param6::parameterDisplay(char*res)
{
    int16_t mixpercent = (int16_t)(this->pData->reverb.mix/328);
    Int16ToChar(mixpercent,res);
    appendToString(res,"%");
}
void AcousticProc::Param7::parameterCallback(uint16_t val)
{
    pData->presetVolume.gain = val >> 2; // 0 to 1024
    this->rawValue = val; 
}

void AcousticProc::Param7::parameterDisplay(char*res)
{
    int16_t dVal;
    dVal = pData->presetVolume.gain*39; // percent with two decimal points
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
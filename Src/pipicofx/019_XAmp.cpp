#include "pipicofx/019_XAmp.hpp"

extern "C" {
#include <stdint.h>
#include "stringFunctions.h"
#include "ln.h"
#include "audio/gainstage.h"
#include "audio/multimodefilter.h"
#include "audio/delay.h"
#include "audio/genericDistortion.h"
#include "pipicofx/delayMemoryHandler.h"
#include "memoryRegions.h"
#include "globalConfig.h"
}

using namespace PiPicoFX;

__ITCM_CODE
float XAmp::XAmp::processSample(float sampleIn)
{
    sampleIn = gainStageProcessSample(sampleIn,&gain);
    sampleIn = gdGetValue(sampleIn,&distortion)*0.5f;
    sampleIn = MMFilterProcessSample(sampleIn,&lowpass);
    sampleIn = delayLineProcessSample(sampleIn,&delay);
    return sampleIn;
}

void XAmp::XAmp::setup()
{
    float  points[8];
    points[0]=0.1f;
    points[1]=0.1f;
    points[2]=0.2f;
    points[3]=0.2f;
    points[4]=0.6f;
    points[5]=0.6f;
    points[6]=0.7f;
    points[7]=0.7f;
    initDelay(&delay,mallocDelayMemory(MAX_DELAY_SINGLEBUFFER << 2),MAX_DELAY_SINGLEBUFFER);
    MMFilterSetResonance(0.7f,&this->lowpass);
    gdSetAllPoints(points,&distortion);
    this->addParameter(new Param1(this));
    this->addParameter(new Param2(this));
    this->addParameter(new Param3(this));
    this->addParameter(new Param4(this));
}

XAmp::XAmp::~XAmp()
{
    freeDelayMemory(this->delay.delayLine);
}

void XAmp::Param1::parameterCallback(uint16_t val)
{
    pData->gain.gain = 1.0f + ((float)val)/64.0f; // 1.0f up to 64.0f
    rawValue = val;
}

void XAmp::Param1::parameterDisplay(char*res)
{
    uint16_t dVal;
    dVal=(uint16_t)(pData->gain.gain*1000.0f);
    decimalUInt16ToChar(dVal,res,1);
    appendToString(res,"%");
}


void XAmp::Param2::parameterCallback(uint16_t val)// lowpass cut
{
    MMFilterSetCutoff(0.001f*toExp((float)(val)*0.0016868f),&this->pData->lowpass);
    rawValue=val;
}

void XAmp::Param2::parameterDisplay(char* res)
{
    decimalUInt16ToChar(rawValue/41,res,2);
}

void XAmp::Param3::parameterCallback(uint16_t val)// delay intensity
{
    
    pData->delay.delayInSamples = 2400 + (val << 3);
    pData->delay.mix = ((float)val)/8192.0f; // up to 100%
    pData->delay.feedback = 0.25f;
    rawValue=val;
}

void XAmp::Param3::parameterDisplay(char* res)
{
    int16_t dVal;
    dVal=(int16_t)(pData->delay.mix*100.0f);
    Int16ToChar(dVal,res);
    appendToString(res,"%");
}


void XAmp::Param4::parameterCallback(uint16_t val)
{
    pData->presetVolume.gain = ((float)val)/1024.0f; // 0.0f up to 4.0f
    rawValue = val;
}

void XAmp::Param4::parameterDisplay(char*res)
{
    uint16_t dVal;
    dVal=(uint16_t)(pData->presetVolume.gain*10000.0f);
    decimalUInt16ToChar(dVal,res,2);
    appendToString(res,"%");
}


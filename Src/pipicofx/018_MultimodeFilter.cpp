#include "pipicofx/018_MultimodeFilter.hpp"

extern "C" {
#include "stringFunctions.h"
#include "audio/gainstage.h"
#include "audio/multimodefilter.h"
#include "ln.h"
#include "memoryRegions.h"
#include "globalConfig.h"
}


using namespace PiPicoFX;


__ITCM_CODE
float MultimodeFilter::MultimodeFilter::processSample(float sampleIn)
{
    
    if ( this->interpCnt < UI_LATENCY_IN_SAMPLES)
    {
        uint16_t interpValue = (((this->newCutoff - this->getParameter(0)->rawValue)*interpCnt)>>12) + this->getParameter(0)->rawValue;
        MMFilterSetCutoff(0.001f*toExp((float)(interpValue)*0.0016868f),&this->mmfilter);
        this->interpCnt++;
        if (this->interpCnt == UI_LATENCY_IN_SAMPLES)
        {
            this->getParameter(0)->rawValue = this->newCutoff;
        }
    }
    
    sampleIn = MMFilterProcessSample(sampleIn,&this->mmfilter);
    return gainStageProcessSample(sampleIn,&this->presetVolume);
}

void MultimodeFilter::MultimodeFilter::setup()
{
    this->addParameter(new Param1(this));
    this->addParameter(new Param2(this));
    this->addParameter(new Param3(this));
    this->addParameter(new Param4(this));
}

MultimodeFilter::MultimodeFilter::~MultimodeFilter()
{

}

void MultimodeFilter::Param1::parameterCallback(uint16_t val)
{
    if (val != this->rawValue && this->pData->interpCnt == 4096)
    {
        this->pData->newCutoff =val;
        this->pData->interpCnt = 0;
    }
}

void MultimodeFilter::Param1::parameterDisplay(char*res)
{
    float freqHz=pData->mmfilter.cutoff*24000.0f;
    int16_t freqHzInt = (int16_t)freqHz;
    Int16ToChar(freqHzInt,res);
    appendToString(res, " Hz");
}

void MultimodeFilter::Param2::parameterCallback(uint16_t val)
{
    MMFilterSetResonance((float)val/4096.0f,&this->pData->mmfilter);
    this->rawValue = val;
}

void MultimodeFilter::Param2::parameterDisplay(char*res)
{
    decimalUInt16ToChar(rawValue/41,res,2);
}

void MultimodeFilter::Param3::parameterCallback(uint16_t val)
{
    MMFilterSetType(val/1365,&this->pData->mmfilter);
    this->rawValue = val;
}

void MultimodeFilter::Param3::parameterDisplay(char*res)
{
    if (this->rawValue < 1365)
    {
        *res=0;
        appendToString(res,"Lowpass");
    }
    else if (this->rawValue < 2730)
    {
        *res=0;
        appendToString(res,"Highpass");
    }
    else
    {
        *res=0;
        appendToString(res,"Bandpass");
    }
}

void MultimodeFilter::Param4::parameterCallback(uint16_t val)
{
    pData->presetVolume.gain = ((float)val)/1024.0f; // 0.0f up to 4.0f
    rawValue = val;
}

void MultimodeFilter::Param4::parameterDisplay(char*res)
{
    uint16_t dVal;
    dVal=(uint16_t)(pData->presetVolume.gain*10000.0f);
    decimalUInt16ToChar(dVal,res,2);
    appendToString(res,"%");
}



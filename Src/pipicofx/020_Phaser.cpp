#include "pipicofx/020_Phaser.hpp"

extern "C" {
#include <stdint.h>
#include "stringFunctions.h"
#include "ln.h"
#include "audio/gainstage.h"
#include "audio/multimodefilter.h"
#include "audio/delay.h"
#include "audio/genericDistortionSimple.h"
#include "pipicofx/delayMemoryHandler.h"
#include "memoryRegions.h"
#include "globalConfig.h"
#include "math.h"
}

using namespace PiPicoFX;

__ITCM_CODE
float Phaser::Phaser::processSample(float sampleIn)
{
    float newIn=0.0f;
    float proc;
    if (this->isOn())
    {
        newIn = sampleIn;
    }
    proc = newIn + feedback*oldVal;
    for (uint8_t c=0;c<8;c++)
    {
        proc = firstOrderIirAllpassProcessSample(proc,aps+c);
    }
    oldVal = proc;
    lfoPhase += lfoPhaseInc;
    float newalpha = sinf(lfoPhase)*0.99f*depth;
    if (lfoPhase > (6.28318f) )
    {
        lfoPhase -= (6.28318f);
    }
    for (uint8_t c=0;c<8;c++)
    {
        aps[c].alpha = newalpha;
    }
    newIn = newIn + mix*(proc - newIn); 

    if (!this->isOn())
    {
        return (sampleIn + newIn);
    }
    return newIn;
}

void Phaser::Phaser::setup(uint8_t allocateMemory)
{

    this->addParameter(new Param1(this));
    this->addParameter(new Param2(this));
    this->addParameter(new Param3(this));
    this->addParameter(new Param4(this));
    this->addParameter(new Param5(this));
    FxProgram::setup(allocateMemory);
}


void Phaser::Param1::parameterCallback(uint16_t val) //speed
{
    this->pData->lfoPhaseInc = 0.0000130f + (0.0013f - 0.0000130f)*((float)val)/4095.0f;
    this->pData->frequency = 1 + ((val*250) >> 10);
    rawValue = val;
}

void Phaser::Param1::parameterDisplay(char*res)
{
    decimalInt16ToChar(pData->frequency,res,2);
    appendToString(res," Hz");
}

void Phaser::Param2::parameterCallback(uint16_t val) // depth
{
    pData->depth = (float)val/4096.0f+0.0002f;
    rawValue = val;
}

void Phaser::Param2::parameterDisplay(char*res)
{
    int16_t intalpha = (int16_t)(pData->depth*100.0f);
    decimalInt16ToChar(intalpha,res,2);
}


void Phaser::Param3::parameterCallback(uint16_t val) // mix
{
    pData->mix = ((float)val)/4095.0f;
    rawValue = val;
}

void Phaser::Param3::parameterDisplay(char*res)
{
    int16_t dVal;
    dVal = (int16_t)(pData->mix*100.0f);
    Int16ToChar(dVal,res);
    appendToString(res,"%");
}

void Phaser::Param4::parameterCallback(uint16_t val) // feedback
{
    pData->feedback = (float)val/4095.0f; 
    rawValue = val;
}

void Phaser::Param4::parameterDisplay(char*res)
{
    uint16_t dVal;
    dVal=(uint16_t)(pData->feedback*10000.0f);
    decimalUInt16ToChar(dVal,res,2);
    appendToString(res,"%");
}

void Phaser::Param5::parameterCallback(uint16_t val) // volume
{
    pData->presetVolume.gain = ((float)val)/1024.0f; // 0.0f up to 4.0f
    rawValue = val;
}

void Phaser::Param5::parameterDisplay(char*res)
{
    uint16_t dVal;
    dVal=(uint16_t)(pData->presetVolume.gain*10000.0f);
    decimalUInt16ToChar(dVal,res,2);
    appendToString(res,"%");
}
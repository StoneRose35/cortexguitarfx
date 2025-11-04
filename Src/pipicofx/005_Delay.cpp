
#include "pipicofx/005_Delay.hpp"
extern "C" {
#include "stringFunctions.h"
#include "audio/gainstage.h"
#include "pipicofx/delayMemoryHandler.h"
#include "memoryRegions.h"
}
using namespace PiPicoFX;
__ITCM_CODE
float Delay::Delay::processSample(float sampleIn)
{
    sampleIn = delayLineProcessSample(sampleIn, &this->delay);
    sampleIn = gainStageProcessSample(sampleIn,&this->presetVolume);
    return sampleIn;
}

void Delay::Param1::parameterCallback(uint16_t val) // Delay Time
{

    int32_t wVal;
    wVal = val;
    wVal <<= 4;
    pData->delay.delayInSamples = wVal; //pData->delay->delayInSamples + ((FXPROGRAM6_DELAY_TIME_LOWPASS_T*(wVal - pData->delay->delayInSamples)) >> 8);
    rawValue = val;
}

void Delay::Param1::parameterDisplay(char*res)
{
    int16_t dval;
    dval = pData->delay.delayInSamples/48; // in ms
    Int16ToChar(dval,res);
    for (uint8_t c=0;c<PARAMETER_NAME_MAXLEN-2;c++)
    {
        if(*(res+c)==0)
        {
            *(res+c)='m';
            *(res+c+1)='s';
            *(res+c+2)=(char)0;
            break;
        }
    }
}

void Delay::Param2::parameterCallback(uint16_t val) // Feedback
{
    rawValue = val;
    pData->delay.feedback=((float)val)/4096.0f;
}


void Delay::Param2::parameterDisplay(char*res)
{
    Int16ToChar((int16_t)(pData->delay.feedback*100.0f),res);
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

void Delay::Param3::parameterCallback(uint16_t val) // Mix
{
    pData->delay.mix = ((float)val)/4096.0f;
    this->rawValue = val; 
}

void Delay::Param3::parameterDisplay(char*res)
{
    Int16ToChar(pData->delay.mix*100.0f,res);
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

void Delay::Param4::parameterCallback(uint16_t val)
{
    pData->presetVolume.gain = ((float)val)/1024.0f; // 0 to 1024
    this->rawValue = val;
}

void Delay::Param4::parameterDisplay(char*res)
{
    uint16_t dVal;
    dVal=(uint16_t)(pData->presetVolume.gain*10000.0f);
    decimalUInt16ToChar(dVal,res,2);
    appendToString(res,"%");
}

void Delay::Delay::setup()
{
    initDelay(&this->delay,mallocDelayMemory(MAX_DELAY_SINGLEBUFFER<<2),MAX_DELAY_SINGLEBUFFER);
    this->addParameter(new Param1(this));
    this->addParameter(new Param2(this));
    this->addParameter(new Param3(this));
    this->addParameter(new Param4(this));

}

Delay::Delay::~Delay()
{
    freeDelayMemory(this->delay.delayLine);
}

/*
FxProgram6DataType fxProgram6data;

FxProgramType fxProgram6 = {
    .name = "Delay                ",
    .nParameters=3,
    .parameters = {
        {
            .name = "Time           ",
            .control=0,
            .increment=64,
            .rawValue=0,
            .getParameterDisplay=&fxProgram6Param1Display,
            .getParameterValue=0,
            .setParameter=&fxProgram6Param1Callback
        },
        {
            .name = "Feedback       ",
            .control=1,
            .increment=64,
            .rawValue=0,
            .getParameterDisplay=&fxProgram6Param2Display,
            .getParameterValue=0,
            .setParameter=&fxProgram6Param2Callback
        },
        {
            .name = "Mix            ",
            .control=2,
            .increment=64,
            .rawValue=0,
            .getParameterDisplay=&fxProgram6Param3Display,
            .getParameterValue=0,
            .setParameter=&fxProgram6Param3Callback
        }
    },
    .processSample = &fxProgram6processSample,
    .setup = &fxProgram6Setup,
    .reset = 0,
    .data = (void*)&fxProgram6data
};

*/
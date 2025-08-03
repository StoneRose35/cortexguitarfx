
#include "pipicofx/011_Reverb3.hpp"
extern "C" {
#include "stringFunctions.h"
#include "pipicofx/delayMemoryHandler.h"
#include "audio/gainstage.h"
#include "romfunc.h"
}
using namespace PiPicoFX;

int16_t Reverb3::Reverb3::processSample(int16_t sampleIn)
{
    int16_t reverberatedSample;
    sampleIn = gainStageProcessSample(sampleIn,&this->presetVolume);
    reverberatedSample = reverb3processSample(sampleIn,&this->reverb);
    return (((0x7FFF - this->mix)*sampleIn) >> 15) + ((this->mix*reverberatedSample) >> 15);
}

void Reverb3::Reverb3::setup()
{
    initReverb3(&this->reverb,mallocDelayMemory((16*DIFFUSOR_SIZE + 4096)<<1));
    this->addParameter(new Param1(this));
    this->addParameter(new Param2(this));
    this->addParameter(new Param3(this));

}

Reverb3::Reverb3::~Reverb3()
{
    freeDelayMemory(this->reverb.diffusers[0].delayPointers[0]);
}

void Reverb3::Param1::parameterCallback(uint16_t val)
{
    int16_t intermVal;
    intermVal = val << 3;
    if (intermVal > 0x7FFD)
    {
        intermVal = 0x7FFD;
    }
    this->pData->reverb.delay.feedback = intermVal;
    this->rawValue = val; 
}

void Reverb3::Param1::parameterDisplay(char*res)
{
    float ffbk;
    int16_t t60;
    ffbk = int2float(this->pData->reverb.delay.feedback)/32767.0f;
    if (ffbk < 0.0000305)
    {
        t60=0;
    }
    else
    {
        t60=(int16_t)float2int(-589.03004f/fln(ffbk)); // t60 in ms
    }
    Int16ToChar(t60,res);
    appendToString(res," ms");
}

void Reverb3::Param2::parameterCallback(uint16_t val)
{
    this->pData->mix=(val << 3);
    this->rawValue = val; 
}

void Reverb3::Param2::parameterDisplay(char*res)
{
    int16_t mixpercent = (int16_t)(this->pData->mix/328);
    Int16ToChar(mixpercent,res);
    appendToString(res,"%");
}
void Reverb3::Param3::parameterCallback(uint16_t val)
{
    this->pData->presetVolume.gain = val >> 2; // 0 to 1024
    this->rawValue = val; 
}

void Reverb3::Param3::parameterDisplay(char*res)
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
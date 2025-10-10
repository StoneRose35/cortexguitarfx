
#include "pipicofx/015_ShimmerVerb.hpp"
extern "C" {
#include "stringFunctions.h"
#include "pipicofx/delayMemoryHandler.h"
#include "audio/gainstage.h"
#include "audio/audiotools.h"
#include "pipicofx/delayMemoryHandler.h"
#include "memoryRegions.h"
#include "math.h"

__ITCM_CODE
float unicornGlitter(float sampleIn,void*data,volatile uint32_t * audioState)
{
    UnicornGlitterDataType* pData=(UnicornGlitterDataType*)data;
    sampleIn = pitchShifter2ProcessSample(sampleIn,&pData->pitchShifter);
    return firstOrderIirLowpassProcessSample(sampleIn,&pData->glitterTamer);
}
}
using namespace PiPicoFX;

__ITCM_CODE
float ShimmerVerb::ShimmerVerb::processSample(float sampleIn)
{
    float sampleProc = gainStageProcessSample(sampleIn,&this->presetVolume);
    
    float summedDelay=0;
    volatile uint32_t * audioStatePtr = getAudioStatePtr();
    summedDelay += delayLineProcessSample(sampleProc,this->delays);
    summedDelay += delayLineProcessSample(sampleProc,this->delays+1);
    summedDelay += delayLineProcessSample(sampleProc,this->delays+2);
    sampleProc = summedDelay;
    sampleProc = allpassProcessSample(sampleProc,this->allpasses,audioStatePtr);
    sampleProc = allpassProcessSample(sampleProc,this->allpasses+1,audioStatePtr);
    sampleProc = delayLineProcessSample(sampleProc,this->delays+3);
    this->oldVal = sampleProc;
    return (1.0f - this->mix)*sampleIn  + this->mix*sampleProc;
}

__QSPI_CODE
void ShimmerVerb::ShimmerVerb::setup()
{
    float * delayMemoryPointer = mallocDelayMemory(11008<<1); 
    initPitchshifter2(&this->unicornGlitterData.pitchShifter);

    initDelay(this->delays,delayMemoryPointer,256);
    this->delays[0].delayInSamples = 149;
    this->delays[0].mix = ((1 << 15) -1) ;
    this->delays[0].feedbackFunction=0;
    initDelay(this->delays+1,delayMemoryPointer+256,512);
    this->delays[1].delayInSamples = 337;
    this->delays[1].mix = ((1 << 15) -1) ;
    this->delays[1].feedbackFunction=0;
    initDelay(this->delays+2,delayMemoryPointer+512+256,2048);
    this->delays[2].delayInSamples = 1597;
    this->delays[2].mix = ((1 << 15) -1);
    this->delays[2].feedbackFunction=0;
    initDelay(this->delays+3,delayMemoryPointer+2048+512+256,4096);
    this->delays[3].delayInSamples = 3989;
    this->delays[3].mix = ((1 << 15) -1) ;
    this->delays[3].feedbackFunction = (AudioProcessorFunc)unicornGlitter;
    this->delays[3].feebackData = &this->unicornGlitterData;

    this->allpasses[0].delayLineIn = delayMemoryPointer + 4096+2048+512+256;
    this->allpasses[0].delayLineOut = delayMemoryPointer + 1024+4096+2048+512+256;
    this->allpasses[0].coefficient = 22936;
    this->allpasses[0].delayPtr = 0;
    this->allpasses[0].oldValues = 0;
    this->allpasses[0].delayInSamples=617;
    this->allpasses[0].bufferSize = 0x3FF;

    this->allpasses[1].delayLineIn = delayMemoryPointer + 1024+1024+4096+2048+512+256;
    this->allpasses[1].delayLineOut = delayMemoryPointer + 1024+1024+1024+4096+2048+512+256;
    this->allpasses[1].coefficient = 22936;
    this->allpasses[1].delayPtr = 0;
    this->allpasses[1].oldValues = 0;
    this->allpasses[1].delayInSamples=907;
    this->allpasses[1].bufferSize = 0x3FF;
    this->addParameter(new Param1(this));
    this->addParameter(new Param2(this));
    this->addParameter(new Param3(this));
    this->addParameter(new Param4(this));

}

__QSPI_CODE
ShimmerVerb::ShimmerVerb::~ShimmerVerb()
{
    freeDelayMemory(this->unicornGlitterData.pitchShifter.delayMemoryPtr);
    freeDelayMemory(this->delays->delayLine);
}

__QSPI_CODE
void ShimmerVerb::Param1::parameterCallback(uint16_t val)
{
    this->pData->unicornGlitterData.pitchShifter.delayIncrement = (val >> 9) + 1;
    this->rawValue = val; 
}

__QSPI_CODE
void ShimmerVerb::Param1::parameterDisplay(char*res)
{
    *res=0;
    switch (pData->unicornGlitterData.pitchShifter.delayIncrement)
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

__QSPI_CODE
void ShimmerVerb::Param2::parameterCallback(uint16_t val)
{
    (this->pData->delays+0)->feedback = val << 3;
    (this->pData->delays+1)->feedback = val << 3;
    (this->pData->delays+2)->feedback = val << 3;
    (this->pData->delays+3)->feedback = val << 3;
    this->pData->feedback = val << 3;
    this->rawValue = val; 
}

__QSPI_CODE
void ShimmerVerb::Param2::parameterDisplay(char*res)
{
    float ffbk;
    int16_t t60;
    ffbk = this->pData->delays->feedback;
    if (ffbk < 0.0000305f)
    {
        t60=0;
    }
    else
    {
        t60=(int16_t)(-589.03004f/logf(ffbk)); // t60 in ms
    }
    Int16ToChar(t60,res);
    appendToString(res," ms");
}

__QSPI_CODE
void ShimmerVerb::Param3::parameterCallback(uint16_t val)
{
    this->pData->mix=(val << 3);
    this->rawValue = val; 
}

__QSPI_CODE
void ShimmerVerb::Param3::parameterDisplay(char*res)
{
    int16_t mixpercent = (int16_t)(this->pData->mix/328);
    Int16ToChar(mixpercent,res);
    appendToString(res,"%");
}
void ShimmerVerb::Param4::parameterCallback(uint16_t val)
{
    this->pData->presetVolume.gain = val >> 2; // 0 to 1024
    this->rawValue = val; 
}

__QSPI_CODE
void ShimmerVerb::Param4::parameterDisplay(char*res)
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
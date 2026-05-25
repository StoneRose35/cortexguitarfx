#include "pipicofx/006_Compressor.hpp"
extern "C" {
#include "stringFunctions.h"
#include "audio/gainstage.h"
#include "fastExpLog.h"
#include "memoryRegions.h"
#include "math.h"
}
using namespace PiPicoFX;

__ITCM_CODE
 float Compressor::Compressor::processSample(float sampleIn)
{

    float newIn=0.0f;
    if (this->isOn())
    {
        newIn = sampleIn;
    }
    switch (this->compressorType)
    {
        case 0:
            newIn = compressorProcessSample(newIn,&this->compressor);
            break;
        case 1:
            newIn = compressor2ProcessSample(newIn,&this->compressor);
            break;
        case 2:
            newIn = compressor3ProcessSample(newIn,&this->compressor);
            break;
    }
    newIn = gainStageProcessSample(newIn,&this->presetVolume);
    if (!this->isOn())
    {
        return (sampleIn + newIn);
    }
    return newIn;
}

 void Compressor::Param1::parameterCallback(uint16_t val) 
{
    pData->compressor.gainFunction.threshhold = -60.0f + (float)val/4095.f*60.0f;
    rawValue = val;
}

 void Compressor::Param1::parameterDisplay(char*res)
{
    decimalInt16ToChar((int16_t)(pData->compressor.gainFunction.threshhold*10.0f),res,1);
    appendToString(res," dB");
}

 void Compressor::Param2::parameterCallback(uint16_t val) 
{
    uint16_t enumVal = (val >> 9) + 1;
    if (enumVal > 5)
    {
        enumVal=5;
    }
    rawValue = val;
    switch (enumVal)
    {
        case 1:
            pData->compressor.gainFunction.gainReduction = 2.0f;
            break;
        case 2:
            pData->compressor.gainFunction.gainReduction = 4.0f;
            break;
        case 3:
            pData->compressor.gainFunction.gainReduction = 8.0f;
            break;
        case 4:
            pData->compressor.gainFunction.gainReduction = 16.0f;
            break;
        default:
            pData->compressor.gainFunction.gainReduction = 1000.0f;
            break;
    }
}

 void Compressor::Param2::parameterDisplay(char*res)
{
    uint16_t c=0,cres=0;
    const char* infDisplay="Inf          ";
    int32_t gainReductionInt = (int32_t)pData->compressor.gainFunction.gainReduction;
    *(res+cres++)='1';
    *(res+cres++)=':';
    if (gainReductionInt > 16)
    {
        while(*(infDisplay+c) !=0)
        {
            *(res+cres++)=*(infDisplay+c++);
        }
        *(res+cres)=0;
    }
    else
    {
        cres +=  decimalInt16ToChar((int16_t)(gainReductionInt*10),res+cres,1);
        for (c=0;c<15-cres;c++)
        {
            *(res+cres++)=' ';
        }
        *(res+cres)=0;
    }
}

 void Compressor::Param3::parameterCallback(uint16_t val) 
{
    pData->presetVolume.gain = (float)val/256.0f + 1.0f;
    rawValue = val;
}

 void Compressor::Param3::parameterDisplay(char*res)
{
    uint32_t dval;
    dval = pData->presetVolume.gain*100.0f;
    decimalInt16ToChar((int16_t)dval,res,2);
}

 void Compressor::Param4::parameterCallback(uint16_t val) 
{
    pData->compressor.avgLowpass.alphaRising = 1.0f - 2.0f/32768.0f -val/64.0f/32768.0;
    rawValue = val;
}

 void Compressor::Param4::parameterDisplay(char*res)
{
    float attackFloat;
    float t60;
    int32_t ival;
    int16_t i16val;
    attackFloat = pData->compressor.avgLowpass.alphaRising;
    t60 = -0.143911568f/logf(attackFloat);

    ival = (int)t60;
    i16val = (int16_t)ival;
    Int16ToChar(i16val,res);
    appendToString(res, " ms");
}

 void Compressor::Param5::parameterCallback(uint16_t val) 
{
    pData->compressor.avgLowpass.alphaFalling = 1.0f - 2.0f/32768.0f - val/64.0f/32768.0f;
    rawValue = val;
}

 void Compressor::Param5::parameterDisplay(char*res)
{
    float releaseFloat;
    float t60;
    int32_t ival;
    int16_t i16val;
    releaseFloat = pData->compressor.avgLowpass.alphaFalling;
    t60 = -0.143911568f/logf(releaseFloat);

    ival = (int)t60;
    i16val = (int16_t)ival;
    Int16ToChar(i16val,res);
    appendToString(res, " ms");
}

 void Compressor::Param6::parameterCallback(uint16_t val) 
{
    uint8_t intermVal;
    intermVal = val >> 10;
    if (intermVal >2 )
    {
        intermVal = 2;
    }
    pData->compressorType = intermVal;
    rawValue = val;
}

 void Compressor::Param6::parameterDisplay(char*res)
{
    *res=0;
    switch (pData->compressorType)
    {
        case 0:
            appendToString(res,"Dirty");
            break;
        case 1:
            appendToString(res,"Snappy");
            break;
        case 2:
            appendToString(res,"Pumpy");
            break;
    }
}

void Compressor::Compressor::setup(uint8_t allocateMemory)
{
    this->addParameter(new Param1(this));
    this->addParameter(new Param2(this));
    this->addParameter(new Param3(this));
    this->addParameter(new Param4(this));
    this->addParameter(new Param5(this));
    this->addParameter(new Param6(this));
    FxProgram::setup(allocateMemory);
}

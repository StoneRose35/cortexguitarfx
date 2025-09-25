
#include "pipicofx/006_Compressor.hpp"
extern "C" {
#include "stringFunctions.h"
#include "audio/gainstage.h"
#include "romfunc.h"
#include "fastExpLog.h"
}
using namespace PiPicoFX;

int16_t Compressor::Compressor::processSample(int16_t sampleIn)
{
    switch(this->compressorType)
    {
        case 0:
            sampleIn = compressorProcessSample(sampleIn,&this->compressor);
            break;
        case 1:
            sampleIn = compressor2ProcessSample(sampleIn,&this->compressor);
            break;
        case 2:
            sampleIn = compressor3ProcessSample(sampleIn,&this->compressor);
            break;
    }
    sampleIn = gainStageProcessSample(sampleIn,&this->presetVolume);
    return sampleIn;
}

void Compressor::Compressor::setup()
{
    this->addParameter(new Param1(this));
    this->addParameter(new Param2(this));
    this->addParameter(new Param3(this));
    this->addParameter(new Param4(this));
    this->addParameter(new Param5(this));
    this->addParameter(new Param6(this));

}

void Compressor::Param1::parameterCallback(uint16_t val)
{
    pData->compressor.gainFunction.threshhold = val << 3;
    this->rawValue = val; 
}

void Compressor::Param1::parameterDisplay(char*res)
{
    int16_t dbval;
    dbval = asDb(this->pData->compressor.gainFunction.threshhold);
    decimalInt16ToChar(dbval,res,1);
    appendToString(res," dB");
}
void Compressor::Param2::parameterCallback(uint16_t val)
{
    uint16_t enumVal = (val >> 9) + 1;
    if (enumVal > 5)
    {
        enumVal=5;
    }
    this->pData->compressor.gainFunction.gainReduction = enumVal;
    this->rawValue = val; 
}

void Compressor::Param2::parameterDisplay(char*res)
{
    const char* dstrings[5];
    dstrings[0]="1:2            ";
    dstrings[1]="1:4            ";
    dstrings[2]="1:8            ";
    dstrings[3]="1:16           ";
    dstrings[4]="1:Inf          ";

    for(uint8_t c=0;c<16;c++)
    {
        *(res+c)=*(dstrings[this->pData->compressor.gainFunction.gainReduction-1] + c);
    }
}
void Compressor::Param3::parameterCallback(uint16_t val)
{
    this->pData->presetVolume.gain = val;
    this->rawValue = val; 
}

void Compressor::Param3::parameterDisplay(char*res)
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

void Compressor::Param4::parameterCallback(uint16_t val)
{
    this->pData->compressor.avgLowpass.alphaRising = (1 << 15) - 2 - (val >> 6);
    this->rawValue = val; 
}

void Compressor::Param4::parameterDisplay(char*res)
{
    float attackFloat;
    float t60;
    int32_t ival;
    int16_t i16val;
    attackFloat = int2float(this->pData->compressor.avgLowpass.alphaRising)/32767.0f;
    t60=-0.143911568f/fln(attackFloat); // -3*ln(10)/(ln(attack)*f_sample)*1000., result in t60 in ms
    ival = float2int(t60);
    i16val = (int16_t)ival;
    Int16ToChar(i16val,res);
    appendToString(res," ms");
}
void Compressor::Param5::parameterCallback(uint16_t val)
{
    this->pData->compressor.avgLowpass.alphaFalling = (1 << 15) - 2 - (val >> 6);
    this->rawValue = val; 
}

void Compressor::Param5::parameterDisplay(char*res)
{
    float releaseFloat;
    float t60;
    int32_t ival;
    int16_t i16val;
    releaseFloat = int2float(this->pData->compressor.avgLowpass.alphaFalling)/32767.0f;
    t60=-0.143911568f/fln(releaseFloat); // -3*ln(10)/(ln(release)*f_sample)*1000., result in t60 in ms
    ival = float2int(t60);
    i16val = (int16_t)ival;
    Int16ToChar(i16val,res);
    appendToString(res," ms");
}
void Compressor::Param6::parameterCallback(uint16_t val)
{
    uint8_t intermVal;
    intermVal = val >> 10;
    if (intermVal == 3)
    {
        intermVal = 2;
    }
    this->pData->compressorType = intermVal;
    this->rawValue = val; 
}

void Compressor::Param6::parameterDisplay(char*res)
{
    *res=0;
    switch (this->pData->compressorType)
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
#include "pipicofx/001_AmpModel.hpp"

extern "C" {
#include "audio/gainstage.h"
#include "pipicofx/delayMemoryHandler.h"
#include "memoryRegions.h"
#include "pipicofx/001_AmpModel.hpp"

__ITCM_CODE
float analogDelayFeedbackFunction(float sampleIn,void*fbkFilterData)
{
    FirstOrderIirType* tData = (FirstOrderIirType*)fbkFilterData;
    return firstOrderIirLowpassProcessSample(sampleIn,tData);
}
}

using namespace PiPicoFX;

__ITCM_CODE
float AmpModel::AmpModel::processSample(float sampleIn)
{
    float out;
    float newIn=0.0f;
    if (this->isOn())
    {
        newIn = sampleIn;
    }

    //this->highpass_out =  (1.0f + this->highpassCutoff)/2.0f*(newIn - this->highpass_old_in) + this->highpassCutoff*this->highpass_old_out; //(((((1 << 15) + this->highpassCutoff) >> 1)*(sampleIn - this->highpass_old_in))>>15) + ((this->highpassCutoff *this->highpass_old_out) >> 15);
    //this->highpass_old_in = newIn;
    //this->highpass_old_out = this->highpass_out;

    //out = this->highpass_out;
    out = threeBandEqProcessSample(newIn,&preEq);
    for (uint8_t c=0;c<this->nWaveshapers;c++)
    {
        out = waveShaperProcessSample(out,&this->waveshaper1);
    }
    
    out = threeBandEqProcessSample(out,&postEq);

    out = gainStageProcessSample(out,&presetVolume);

    //out = out/2.0f;
    out = secondOrderIirFilterProcessSample(out,&this->filter1);
    
    out = firFilterProcessSample(out, &this->filter3);
    
    //out = delayLineProcessSample(out, &this->delay);
    
    if (!this->isOn())
    {
        return (sampleIn + out);
    }
    return out;
}

void AmpModel::Param1::parameterCallback(uint16_t val) // pre-Eq as single parameter setting
{
    //float fval;

    //fval = ((FXPROGRAM1_HIGHCUT_DELTA*(float)val)/4096.0f);
    //pData->highpassCutoff = FXPROGRAM1_HIGHCUT_VAL1 + fval;

    setSingleParam(val,&pData->preEq);
    rawValue=val;
}

void AmpModel::Param1::parameterDisplay(char* chrbfr)
{
    uint32_t dval;
    dval=(uint32_t)((float)rawValue*0.2439560f); // map to 0-999
    Int16ToChar(dval,chrbfr);
}

void AmpModel::Param2::parameterCallback(uint16_t val) // number of waveshaper (more means more distortion)
{
    // map 0-4095 to 1-8
    rawValue=val;
    val >>= 9;
    val += 1; 
    pData->nWaveshapers = val;
}

void AmpModel::Param2::parameterDisplay(char* res)
{
    UInt8ToChar(pData->nWaveshapers,res);
};

void AmpModel::Param3::parameterCallback(uint16_t val)// post-EQ
{
    
    setSingleParam(val,&pData->postEq);
    //pData->delay.delayInSamples = 2400 + (val << 3);
    //pData->delay.mix = ((float)val)/8192.0f; // up to 100%
    //pData->delay.feedback = 0.25f;
    rawValue=val;
}

void AmpModel::Param3::parameterDisplay(char* res)
{
    uint32_t dval;
    dval=(uint32_t)((float)rawValue*0.2439560f); // map to 0-999
    Int16ToChar(dval,res);
}

void AmpModel::Param4::parameterCallback(uint16_t val)
{
    pData->presetVolume.gain = ((float)val)/1024.0f; // 0.0f up to 4.0f
    rawValue = val;
};

void AmpModel::Param4::parameterDisplay(char* res)
{
    uint16_t dVal;
    dVal=(uint16_t)(pData->presetVolume.gain*10000.0f);
    decimalUInt16ToChar(dVal,res,2);
    appendToString(res,"%");
};

AmpModel::AmpModel::~AmpModel()
{
    //freeDelayMemory(this->delay.delayLine);
}

void AmpModel::AmpModel::setup(uint8_t allocateMemory)
{
    if (allocateMemory)
    {
        initfirFilter(&filter3);
        initWaveShaper(&waveshaper1,&waveShaperDefaultOverdrive);
        initThreeBandEq(&preEq);
        initThreeBandEq(&postEq);
    }
    this->addParameter(new Param1(this));
    this->addParameter(new Param2(this));
    this->addParameter(new Param3(this));
    this->addParameter(new Param4(this));
    this->setFreezable(0);
    FxProgram::setup(allocateMemory);
}

/*
void AmpModel::AmpModel::freeze()
{
    FxProgram::freeze();
    delay.frozen = 1;
}

void AmpModel::AmpModel::unfreeze()
{
    FxProgram::unfreeze();
    delay.frozen=0;
}
    */
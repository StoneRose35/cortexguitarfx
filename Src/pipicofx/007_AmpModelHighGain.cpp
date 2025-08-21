
#include "pipicofx/007_AmpModelHighGain.hpp"
extern "C" {
#include "stringFunctions.h"
#include "audio/gainstage.h"
#include "pipicofx/delayMemoryHandler.h"
}
using namespace PiPicoFX;

int16_t AmpModelHighGain::AmpModelHighGain::processSample(int16_t sampleIn)
{
    int16_t out;
    this->highpass_out = (((((1 << 15) + this->highpassCutoff) >> 1)*(sampleIn - this->highpass_old_in))>>15) + ((this->highpassCutoff *this->highpass_old_out) >> 15);
    this->highpass_old_in = sampleIn;
    this->highpass_old_out = this->highpass_out;

    out = this->highpass_out;
    out =multiWaveShaperProcessSample(out,&this->waveshaper1);
    out =multiWaveShaperProcessSample(out,&this->waveshaper1);
    out = out >> 1;

    out = gainStageProcessSample(out,&this->presetVolume);
    
    switch (this->cabSimType)
    {
        case 0:
            out = secondOrderIirFilterProcessSample(out,&this->customCabIir);
            out >>= 1;
            out = firFilterProcessSample(out,&this->customCabFir);
            break;
        case 1:
            //out >>= 1;
            out = secondOrderIirFilterProcessSample(out,&this->customIir1);
            out = secondOrderIirFilterProcessSample(out,&this->customIir2);
            out = secondOrderIirFilterProcessSample(out,&this->customIir3);
            out <<= 2;
            out = secondOrderIirFilterProcessSample(out,&this->customIir4);
            break;
        case 2:
            out >>= 2;
            out = firFilterProcessSample(out,&this->hiwattFir);
            break;
        case 3:
            out = secondOrderIirFilterProcessSample(out,&this->hiwattIir1);
            out = secondOrderIirFilterProcessSample(out,&this->hiwattIir2);
            out = secondOrderIirFilterProcessSample(out,&this->hiwattIir3);
            break;
        case 4:
            out >>= 2;
            out = firFilterProcessSample(out,&this->frontmanFir);
            break;
        case 5:
            out = secondOrderIirFilterProcessSample(out,&this->frontmanIir1);
            out = secondOrderIirFilterProcessSample(out,&this->frontmanIir2);
            out = secondOrderIirFilterProcessSample(out,&this->frontmanIir3);
            break;
        case 6:
            out >>= 2;
            out = firFilterProcessSample(out,&this->voxAC15Fir);
            break;
        case 7:
            out = secondOrderIirFilterProcessSample(out,&this->voxAC15Iir1);
            out = secondOrderIirFilterProcessSample(out,&this->voxAC15Iir2);
            out = secondOrderIirFilterProcessSample(out,&this->voxAC15Iir3);
            break;
        default:
            break;

    }
    if (this->modType==0)
    {
        out = delayLineProcessSample(out, &this->delay);
    }
    else
    {
        out = reverbProcessSample(out,&this->reverb);
    }
    return out;

}

void AmpModelHighGain::AmpModelHighGain::setup()
{
    initfirFilter(&this->customCabFir);
    initfirFilter(&this->frontmanFir);
    initfirFilter(&this->hiwattFir);
    initfirFilter(&this->voxAC15Fir);
    initMultiWaveShaper(&this->waveshaper1,&multiWaveshaper1);
    initDelay(&this->delay,mallocDelayMemory(MAX_DELAY_SINGLEBUFFER<<1),MAX_DELAY_SINGLEBUFFER);
    initReverb(&this->reverb,500,this->delay.delayLine); // putting delay and reverb onto the same memory since they are used mutually exclusively
    this->addParameter(new Param1(this));
    this->addParameter(new Param2(this));
    this->addParameter(new Param3(this));
    this->addParameter(new Param4(this));
    this->addParameter(new Param5(this));
    this->addParameter(new Param6(this));

}

AmpModelHighGain::AmpModelHighGain::~AmpModelHighGain()
{
    freeDelayMemory(this->delay.delayLine);
}

void AmpModelHighGain::Param1::parameterCallback(uint16_t val)
{
    uint32_t dval;
    dval = ((AMPMODEL_HIGHGAIN_HIGHCUT_DELTA*val) >> 12);
    this->pData->highpassCutoff = AMPMODEL_HIGHGAIN_HIGHCUT_VAL1 + (int16_t)dval;
    this->rawValue = val; 
}

void AmpModelHighGain::Param1::parameterDisplay(char*res)
{
    Int16ToChar(pData->highpassCutoff,res);
}
void AmpModelHighGain::Param2::parameterCallback(uint16_t val)
{
    this->rawValue = val; 
    val >>= 6;
    pData->waveshaper1.functionIndex = val;
}

void AmpModelHighGain::Param2::parameterDisplay(char*res)
{
    UInt8ToChar(this->pData->waveshaper1.functionIndex,res);
}
void AmpModelHighGain::Param3::parameterCallback(uint16_t val)
{
    this->pData->delay.delayInSamples = 2400 + (val << 3);
    this->pData->delay.mix = val << 2; // up to 100%
    this->pData->delay.feedback = (1<< 14);

    setReverbTime(500 + (val>>2),&this->pData->reverb);
    this->pData->reverb.mix = val << 2;
    this->rawValue = val; 
}

void AmpModelHighGain::Param3::parameterDisplay(char*res)
{
    int16_t dVal;
    dVal=pData->delay.mix/164;
    Int16ToChar(dVal,res);
    appendToString(res,"%");
}
void AmpModelHighGain::Param4::parameterCallback(uint16_t val)
{
    this->pData->modType = val >> 11;
    this->rawValue = val; 
}

void AmpModelHighGain::Param4::parameterDisplay(char*res)
{
    *res=0;
    if (this->pData->modType==0)
    {
        appendToString(res,"Delay");
    }
    else
    {
        appendToString(res,"Reverb");
    }
}
void AmpModelHighGain::Param5::parameterCallback(uint16_t val)
{
    this->pData->cabSimType = val >> 8;
    if (this->pData->cabSimType > 8)
    {
        this->pData->cabSimType=8;
    }
    this->rawValue = val; 
}

void AmpModelHighGain::Param5::parameterDisplay(char*res)
{
    for(uint8_t c=0;c<24;c++)
    {
        *(res+c)=this->pData->cabNames[pData->cabSimType][c];
    }
}
void AmpModelHighGain::Param6::parameterCallback(uint16_t val)
{
    this->pData->presetVolume.gain = val >> 2; // 0 to 1024
    this->rawValue = val; 
}

void AmpModelHighGain::Param6::parameterDisplay(char*res)
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
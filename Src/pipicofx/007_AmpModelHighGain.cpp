#include "pipicofx/007_AmpModelHighGain.hpp"
extern "C" {
#include "audio/gainstage.h"
#include "stringFunctions.h"
#include "pipicofx/delayMemoryHandler.h"
#include "memoryRegions.h"
}

#define FXPROGRAM_HIGHCUT_VAL1 0.61035f
#define FXPROGRAM_HIGHCUT_VAL2 0.9613037f

#define FXPROGRAM1_HIGHCUT_DELTA (FXPROGRAM_HIGHCUT_VAL2-FXPROGRAM_HIGHCUT_VAL1)
__ITCM_CODE
 float AmpModelHighGain::AmpModelHighGain::processSample(float sampleIn)
{
    float out;

    float newIn=0.0f;
    if (this->isOn())
    {
        newIn = sampleIn;
    }
    this->highpass_out =  (1.0f + this->highpassCutoff)/2.0f*(newIn - this->highpass_old_in) + this->highpassCutoff*this->highpass_old_out; 
    this->highpass_old_in = newIn;
    this->highpass_old_out = this->highpass_out;

    out = this->highpass_out;

    // twice to overcome gain limitations due to interpolation
    out = multiWaveShaperProcessSample(out,&this->waveshaper1);
    out = multiWaveShaperProcessSample(out,&this->waveshaper1);


    out = out/2.0f;
    
switch (this->cabSimType)
    {
        case 0:
            out = secondOrderIirFilterProcessSample(out,&this->customCabIir);
            out = out/ 2.0f;
            out = firFilterProcessSample(out,&this->customCabFir);
            break;
        case 1:
            //out >>= 1;
            out = secondOrderIirFilterProcessSample(out,&this->customIir1);
            out = secondOrderIirFilterProcessSample(out,&this->customIir2);
            out = secondOrderIirFilterProcessSample(out,&this->customIir3);
            out = out*4.0f;
            out = secondOrderIirFilterProcessSample(out,&this->customIir4);
            break;
        case 2:
            out = out/4.0f;
            out = firFilterProcessSample(out,&this->hiwattFir);
            break;
        case 3:
            out = secondOrderIirFilterProcessSample(out,&this->hiwattIir1);
            out = secondOrderIirFilterProcessSample(out,&this->hiwattIir2);
            out = secondOrderIirFilterProcessSample(out,&this->hiwattIir3);
            break;
        case 4:
            out = out / 4.0f;
            out = firFilterProcessSample(out,&this->frontmanFir);
            break;
        case 5:
            out = secondOrderIirFilterProcessSample(out,&this->frontmanIir1);
            out = secondOrderIirFilterProcessSample(out,&this->frontmanIir2);
            out = secondOrderIirFilterProcessSample(out,&this->frontmanIir3);
            break;
        case 6:
            out = out / 4.0f;
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
    out = gainStageProcessSample(out,&this->presetVolume);
    if (!this->isOn())
    {
        return (sampleIn + out);
    }
    return out;
}

 void AmpModelHighGain::Param1::parameterCallback(uint16_t val) // highpass cutoff before the nonlinear stage
{
    float fval;

    fval = ((FXPROGRAM1_HIGHCUT_DELTA*(float)val)/4096.0f);
    pData->highpassCutoff = FXPROGRAM_HIGHCUT_VAL1 + fval;
    rawValue = val;
}

 void AmpModelHighGain::Param1::parameterDisplay(char*res)
{
    uint32_t dval;
    dval=(uint32_t)(pData->highpassCutoff*100.0f);
    Int16ToChar(dval,res);
}

 void AmpModelHighGain::Param2::parameterCallback(uint16_t val) // number of waveshaper (more means more distortion)
{
    // map 0-4095 to 0-63
    rawValue = val;
    val >>= 6;
    pData->waveshaper1.functionIndex=(uint8_t)val;
}

 void AmpModelHighGain::Param2::parameterDisplay(char*res)
{
    UInt8ToChar(pData->waveshaper1.functionIndex,res);
}

 void AmpModelHighGain::Param3::parameterCallback(uint16_t val) // delay/reverb intensity
{
    pData->delay.delayInSamples = 2400 + (val << 3);
    pData->delay.mix = ((float)val)/8192.0f; // up to 100%
    pData->delay.feedback = 0.25f;

    setReverbTime(0.5f + (float)val/4000.0f,&pData->reverb);
    pData->reverb.mix = ((float)val)/8192.0f; // up to 100%
    rawValue = val;
}

 void AmpModelHighGain::Param3::parameterDisplay(char*res)
{
    int16_t dVal;
    dVal=rawValue/41;
    Int16ToChar(dVal,res);
    appendToString(res,"%");
}

 void AmpModelHighGain::Param4::parameterCallback(uint16_t val) // modulation type
{
    pData->modType = val >> 11;
    rawValue = val;
}

 void AmpModelHighGain::Param4::parameterDisplay(char*res) // modulation type
{
    *res=0;
    if (pData->modType==0)
    {
        appendToString(res,"Delay");
    }
    else
    {
        appendToString(res,"Reverb");
    }
}

 void AmpModelHighGain::Param5::parameterCallback(uint16_t val) // cab type
{
    pData->cabSimType = val >> 8;
    if (pData->cabSimType > 8)
    {
        pData->cabSimType=8;
    }
    rawValue = val;
}

 void AmpModelHighGain::Param5::parameterDisplay(char*res) // cab type
{

    for(uint8_t c=0;c<24;c++)
    {
        *(res+c)=pData->cabNames[pData->cabSimType][c];
    }
}

 void AmpModelHighGain::AmpModelHighGain::setup()
{
    initfirFilter(&this->customCabFir);
    initfirFilter(&this->frontmanFir);
    initfirFilter(&this->hiwattFir);
    initfirFilter(&this->voxAC15Fir);
    initMultiWaveShaper(&this->waveshaper1,&multiWaveshaper1);
    initDelay(&this->delay,mallocDelayMemory(MAX_DELAY_SINGLEBUFFER<<2),MAX_DELAY_SINGLEBUFFER);
    initReverb(&this->reverb,500,this->delay.delayLine); // putting delay and reverb onto the same memory since they are used mutually exclusively
    this->addParameter(new Param1(this));
    this->addParameter(new Param2(this));
    this->addParameter(new Param3(this));
    this->addParameter(new Param4(this));
    this->addParameter(new Param5(this));
    this->addParameter(new Param6(this));

}

void AmpModelHighGain::Param6::parameterCallback(uint16_t val)
{
    pData->presetVolume.gain = ((float)val)/1024.0f; // 0.0f up to 4.0f
    rawValue = val;
}

void AmpModelHighGain::Param6::parameterDisplay(char*res)
{
    uint16_t dVal;
    dVal=(uint16_t)(pData->presetVolume.gain*10000.0f);
    decimalUInt16ToChar(dVal,res,2);
    appendToString(res,"%");
}

AmpModelHighGain::AmpModelHighGain::~AmpModelHighGain()
{
    freeDelayMemory(this->delay.delayLine);
}


#include "pipicofx/FXProgram_AmpModel.hpp"
extern "C" {
#include "audio/gainstage.h"

int16_t analogDelayFeedbackFunction(int16_t sampleIn,void*fbkFilterData,volatile uint32_t*audioStatePtr)
{
    FirstOrderIirType* tData = (FirstOrderIirType*)fbkFilterData;
    return firstOrderIirLowpassProcessSample(sampleIn,tData);
}
}
using namespace PiPicoFX;


int16_t AmpModel::AmpModel::processSample(int16_t sampleIn) 
{
    int16_t out;
    this->dataImpl.highpass_out = (((((1 << 15) + this->dataImpl.highpassCutoff) >> 1)*(sampleIn - dataImpl.highpass_old_in))>>15) + ((this->dataImpl.highpassCutoff *this->dataImpl.highpass_old_out) >> 15);
    this->dataImpl.highpass_old_in = sampleIn;
    this->dataImpl.highpass_old_out = dataImpl.highpass_out;

    out = dataImpl.highpass_out;
    for (uint8_t c=0;c<dataImpl.nWaveshapers;c++)
    {
        out = waveShaperProcessSample(out,&dataImpl.waveshaper1);
    }

    out = gainStageProcessSample(out,&dataImpl.presetVolume);

    out = out >> 1;

    out = secondOrderIirFilterProcessSample(out,&dataImpl.filter1);
    out >>= 2;
    out = firFilterProcessSample(out,&dataImpl.filter3);
    out = delayLineProcessSample(out, &dataImpl.delay);
    return out;
}

void AmpModel::AmpModel::setup()
{
    this->data = &this->dataImpl;
    initfirFilter(&dataImpl.filter3);
    initWaveShaper(&dataImpl.waveshaper1,&waveShaperDefaultOverdrive);
    initDelay(&dataImpl.delay,getDelayMemoryPointer(),DELAY_LINE_LENGTH);
    dataImpl.delay.feebackData = (void*)&dataImpl.feedbackFilter;
    this->addParameter(new Param1(this->data));
    this->addParameter(new Param2(this->data));
    this->addParameter(new Param3(this->data));
    this->addParameter(new Param4(this->data));
    
}

void AmpModel::Param1::parameterCallback(uint16_t val)
{
    uint32_t dval;
    dval = ((FXPROGRAM1_HIGHCUT_DELTA*val) >> 12);
    pData->highpassCutoff = FXPROGRAM1_HIGHCUT_VAL1 + (int16_t)dval;
    rawValue = val;
};

void AmpModel::Param1::parameterDisplay(char* chrbfr)
{
    Int16ToChar(pData->highpassCutoff,chrbfr);
};

void AmpModel::Param2::parameterCallback(uint16_t val)
{
    rawValue=val;
    // map 0-4095 to 1-8
    val >>= 9;
    val += 1; 
    pData->nWaveshapers = val;
};

void AmpModel::Param2::parameterDisplay(char* res)
{
    UInt8ToChar(pData->nWaveshapers,res);
};


void AmpModel::Param3::parameterCallback(uint16_t val)
{
    pData->delay.delayInSamples = 9600 + (val << 2);
    pData->delay.mix = val << 2; // up to 100%
    pData->delay.feedback = (1<< 13);
    rawValue=val;
};

void AmpModel::Param3::parameterDisplay(char* res)
{
    int16_t dVal;
    dVal=pData->delay.mix/164;
    Int16ToChar(dVal,res);
    uint8_t c=0;

    while(*(res+c)!=0)
    {
        c++;
    }
    *(res+c)='%';
    *(res+c+1)=(char)0;
};

void AmpModel::Param4::parameterCallback(uint16_t val)
{
    pData->presetVolume.gain = val >> 2; // 0 to 1024
    rawValue=val;
};

void AmpModel::Param4::parameterDisplay(char* res)
{
    uint16_t dVal;
    dVal = pData->presetVolume.gain*39; // percent with two decimal points
    decimalUInt16ToChar(dVal,res,2);
    uint8_t c=0;

    while(*(res+c)!=0)
    {
        c++;
    }
    *(res+c)='%';
    *(res+c+1)=(char)0;
};


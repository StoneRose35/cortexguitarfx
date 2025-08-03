
#include "pipicofx/001_AmpModel.hpp"
extern "C" {
#include "audio/gainstage.h"
#include "pipicofx/delayMemoryHandler.h"
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
    this->highpass_out = (((((1 << 15) + this->highpassCutoff) >> 1)*(sampleIn - highpass_old_in))>>15) + ((this->highpassCutoff *this->highpass_old_out) >> 15);
    this->highpass_old_in = sampleIn;
    this->highpass_old_out = highpass_out;

    out = highpass_out;
    for (uint8_t c=0;c<nWaveshapers;c++)
    {
        out = waveShaperProcessSample(out,&waveshaper1);
    }

    out = gainStageProcessSample(out,&presetVolume);

    out = out >> 1;

    out = secondOrderIirFilterProcessSample(out,&filter1);
    out >>= 2;
    out = firFilterProcessSample(out,&filter3);
    out = delayLineProcessSample(out, &delay);
    return out;
}

AmpModel::AmpModel::~AmpModel()
{
    freeDelayMemory(this->delay.delayLine);
}
void AmpModel::AmpModel::setup()
{
    initfirFilter(&filter3);
    initWaveShaper(&waveshaper1,&waveShaperDefaultOverdrive);
    initDelay(&delay,mallocDelayMemory(DELAY_LINE_LENGTH << 1),DELAY_LINE_LENGTH);
    delay.feebackData = (void*)&feedbackFilter;
    this->addParameter(new Param1(this));
    this->addParameter(new Param2(this));
    this->addParameter(new Param3(this));
    this->addParameter(new Param4(this));
    
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


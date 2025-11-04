#include "pipicofx/001_AmpModel.hpp"

extern "C" {
#include "audio/gainstage.h"
#include "pipicofx/delayMemoryHandler.h"
#include "memoryRegions.h"

__ITCM_CODE
float analogDelayFeedbackFunction(float sampleIn,void*fbkFilterData,volatile uint32_t*audioStatePtr)
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

    this->highpass_out =  (1.0f + this->highpassCutoff)/2.0f*(sampleIn - this->highpass_old_in) + this->highpassCutoff*this->highpass_old_out; //(((((1 << 15) + this->highpassCutoff) >> 1)*(sampleIn - this->highpass_old_in))>>15) + ((this->highpassCutoff *this->highpass_old_out) >> 15);
    this->highpass_old_in = sampleIn;
    this->highpass_old_out = this->highpass_out;

    out = this->highpass_out;
    
    for (uint8_t c=0;c<this->nWaveshapers;c++)
    {
        out = waveShaperProcessSample(out,&this->waveshaper1);
    }
    

    out = gainStageProcessSample(out,&presetVolume);

    out = out/2.0f;
    out = secondOrderIirFilterProcessSample(out,&this->filter1);
    
    out = firFilterProcessSample(out, &this->filter3);
    
    out = delayLineProcessSample(out, &this->delay);
    
    return out;
}

void AmpModel::Param1::parameterCallback(uint16_t val) // highpass cutoff before the nonlinear stage
{
    float fval;

    fval = ((FXPROGRAM1_HIGHCUT_DELTA*(float)val)/4096.0f);
    pData->highpassCutoff = FXPROGRAM1_HIGHCUT_VAL1 + fval;
    rawValue=val;
}

void AmpModel::Param1::parameterDisplay(char* chrbfr)
{
    uint32_t dval;
    dval=(uint32_t)(pData->highpassCutoff*100.0f);
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

void AmpModel::Param3::parameterCallback(uint16_t val)// delay intensity
{
    
    pData->delay.delayInSamples = 2400 + (val << 3);
    pData->delay.mix = ((float)val)/8192.0f; // up to 100%
    pData->delay.feedback = 0.25f;
    rawValue=val;
}

void AmpModel::Param3::parameterDisplay(char* res)
{
    int16_t dVal;
    dVal=(int16_t)(pData->delay.mix*100.0f);
    Int16ToChar(dVal,res);
    appendToString(res,"%");
}

void AmpModel::Param4::parameterCallback(uint16_t val)
{
    pData->presetVolume.gain = ((float)val)/1024.0f; // 0.0f up to 4.0f
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
    freeDelayMemory(this->delay.delayLine);
}

void AmpModel::AmpModel::setup()
{
    initfirFilter(&filter3);
    initWaveShaper(&waveshaper1,&waveShaperDefaultOverdrive);
    initDelay(&delay,mallocDelayMemory(MAX_DELAY_SINGLEBUFFER << 2),MAX_DELAY_SINGLEBUFFER);
    delay.feebackData = (void*)&feedbackFilter;
    this->addParameter(new Param1(this));
    this->addParameter(new Param2(this));
    this->addParameter(new Param3(this));
    this->addParameter(new Param4(this));
}

/*
FxProgram1DataType fxProgram1data = {
    // butterworth lowpass @ 6000Hz 
    .filter1 = {
        	.coeffB = {0.09763107f, 0.19526215f, 0.09763107f},
            .coeffA = {-0.94280904f, 0.33333333f},
            .x1=0.0f,
            .x2=0.0f,
            .y1=0.0f,
            .y2=0.0f,
            .acc=0.0f
    },
    .filter3 = {
        .coefficients = {0.016731f, 0.017496f, 0.021249f, 0.031896f, 0.051696f, 0.083098f, 0.125237f, 0.160554f, 0.158897f, 0.113163f, 0.059528f, 0.022957f, -0.017582f, -0.054411f, -0.064864f, -0.061696f, -0.053666f, -0.033009f, -0.007943f, 0.008333f, 0.015584f, 0.018759f, 0.015862f, 0.012281f, 0.016486f, 0.020179f, 0.022272f, 0.020375f, 0.007048f, -0.009343f, -0.016356f, -0.011307f, 0.000459f, 0.011650f, 0.015174f, 0.007995f, 0.000715f, 0.000025f, -0.004465f, -0.010208f, -0.007323f, 0.001944f, 0.012667f, 0.022059f, 0.027738f, 0.028239f, 0.024619f, 0.019782f, 0.017925f, 0.018693f, 0.019640f, 0.018431f, 0.012868f, 0.005646f, -0.000878f, -0.006494f, -0.005713f, -0.000488f, 0.005790f, 0.010304f, 0.013693f, 0.016206f, 0.017209f, 0.016596f, }    },
    .highpass_old_in=0.0f,
    .highpass_old_out=0.0f,
    .highpass_out=0.0f,
    .highpassCutoff = 0.9460737f,
    .nWaveshapers = 1,
    .feedbackFilter.alpha = 14000.0f/32768.0f,
    .feedbackFilter.oldVal=0.0f,
    .feedbackFilter.oldXVal=0.0f,
    .delay.feedbackFunction=&analogDelayFeedbackFunction
    
};


__attribute__((section (".qspi_code")))
static void fxProgram1Reset(void*data)
{
    FxProgram1DataType* pData = (FxProgram1DataType*)data;
    pData->highpass_old_in=0.0f;
    pData->highpass_old_out=0.0f;
    secondOrderIirFilterReset(&pData->filter1);
    firFilterReset(&pData->filter3);
}

FxProgramType fxProgram1 = {
    .name = "Amp-Simulator",
    .nParameters = 3,
    .parameters = {
        {
            .name="Hi-Cut         ",
            .control=0,
            .increment=100,
            .rawValue=31500,
            .setParameter=&fxProgram1Param1Callback,
            .getParameterValue=0,
            .getParameterDisplay=&fxProgram1Param1Display
        },
        {
            .name="Gain/Stages    ",
            .control=1,
            .increment = 512, // 4096/8
            .rawValue=0,
            .setParameter=&fxProgram1Param2Callback,
            .getParameterValue=0,
            .getParameterDisplay=&fxProgram1Param2Display
        },
        {
            .name="Delay Intensity",
            .control=2,
            .increment=64,
            .rawValue=0,
            .setParameter=&fxProgram1Param3Callback,
            .getParameterValue=0,
            .getParameterDisplay=&fxProgram1Param3Display
        },
    },
    .processSample = &fxProgram1processSample,
    .setup = &fxProgram1Setup,
    .reset = &fxProgram1Reset,
    .data = (void*)&fxProgram1data
} ;
 */
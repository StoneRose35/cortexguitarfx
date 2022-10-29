#include "audio/fxprogram/fxProgram.h"
#include "stringFunctions.h"
#include "romfunc.h"

#define FXPROGRAM1_HIGHCUT_VAL1 0.61035f
#define FXPROGRAM1_HIGHCUT_VAL2 0.9613037f

#define FXPROGRAM1_HIGHCUT_DELTA (FXPROGRAM1_HIGHCUT_VAL2-FXPROGRAM1_HIGHCUT_VAL1)
static float fxProgram1processSample(float sampleIn,void*data)
{
    float out;
    FxProgram1DataType* pData = (FxProgram1DataType*)data;

    pData->highpass_out =  (1.0f + pData->highpassCutoff)/2.0f*(sampleIn - pData->highpass_old_in) + pData->highpassCutoff*pData->highpass_old_out; //(((((1 << 15) + pData->highpassCutoff) >> 1)*(sampleIn - pData->highpass_old_in))>>15) + ((pData->highpassCutoff *pData->highpass_old_out) >> 15);
    pData->highpass_old_in = sampleIn;
    pData->highpass_old_out = pData->highpass_out;

    out = pData->highpass_out;
    for (uint8_t c=0;c<pData->nWaveshapers;c++)
    {
        out = waveShaperProcessSample(out,&pData->waveshaper1);
    }

    out = out/2.0f;

    out = secondOrderIirFilterProcessSample(out,&pData->filter1);
    out = out/4.0f;
    out = firFilterProcessSample(out,&pData->filter3);
    out = delayLineProcessSample(out, pData->delay);
    return out;
}


static void fxProgram1Param1Callback(uint16_t val,void*data) // highpass cutoff before the nonlinear stage
{
    float fval;
    FxProgram1DataType* pData = (FxProgram1DataType*)data;
    fval = ((FXPROGRAM1_HIGHCUT_DELTA*(float)val)/4096.0f);
    pData->highpassCutoff = FXPROGRAM1_HIGHCUT_VAL1 + fval;
}

static void fxProgram1Param1Display(void*data,char*res)
{
    uint32_t dval;
    FxProgram1DataType* pData = (FxProgram1DataType*)data;
    dval=(uint32_t)(pData->highpassCutoff*100.0f);
    Int16ToChar(dval,res);
}

static void fxProgram1Param2Callback(uint16_t val,void*data) // number of waveshaper (more means more distortion)
{
    FxProgram1DataType* pData = (FxProgram1DataType*)data;
    // map 0-4095 to 1-8
    val >>= 9;
    val += 1; 
    pData->nWaveshapers = val;
}

static void fxProgram1Param2Display(void*data,char*res)
{
    FxProgram1DataType* pData = (FxProgram1DataType*)data;
    UInt8ToChar(pData->nWaveshapers,res);
}


static void fxProgram1Param3Callback(uint16_t val,void*data) // delay intensity
{
    FxProgram1DataType* pData = (FxProgram1DataType*)data;
    pData->delay->delayInSamples = 2400 + (val << 3);
    pData->delay->mix = val << 2; // up to 100%
    pData->delay->feedback = (1<< 14);
}

static void fxProgram1Param3Display(void*data,char*res)
{
    int16_t dVal;
    FxProgram1DataType* pData = (FxProgram1DataType*)data;
    dVal=pData->delay->mix/164;
    Int16ToChar(dVal,res);
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

static void fxProgram1Setup(void*data)
{
    FxProgram1DataType* pData = (FxProgram1DataType*)data;
    initfirFilter(&pData->filter3);
    initWaveShaper(&pData->waveshaper1,&waveShaperDefaultOverdrive);
    pData->delay = getDelayData();
    initDelay(pData->delay);
}


FxProgram1DataType fxProgram1data = {
    /* butterworth lowpass @ 6000Hz */
    .filter1 = {
        	.coeffB = {0.09763107f, 0.19526215f, 0.09763107f},
            .coeffA = {-30893, 10922},
            .w= {0.0f,0.0f,0.0f}
    },
    .filter3 = {
        .coefficients = {0.016731f, 0.017496f, 0.021249f, 0.031896f, 0.051696f, 0.083098f, 0.125237f, 0.160554f, 0.158897f, 0.113163f, 0.059528f, 0.022957f, -0.017582f, -0.054411f, -0.064864f, -0.061696f, -0.053666f, -0.033009f, -0.007943f, 0.008333f, 0.015584f, 0.018759f, 0.015862f, 0.012281f, 0.016486f, 0.020179f, 0.022272f, 0.020375f, 0.007048f, -0.009343f, -0.016356f, -0.011307f, 0.000459f, 0.011650f, 0.015174f, 0.007995f, 0.000715f, 0.000025f, -0.004465f, -0.010208f, -0.007323f, 0.001944f, 0.012667f, 0.022059f, 0.027738f, 0.028239f, 0.024619f, 0.019782f, 0.017925f, 0.018693f, 0.019640f, 0.018431f, 0.012868f, 0.005646f, -0.000878f, -0.006494f, -0.005713f, -0.000488f, 0.005790f, 0.010304f, 0.013693f, 0.016206f, 0.017209f, 0.016596f, }    },
    .highpass_old_in=0.0f,
    .highpass_old_out=0.0f,
    .highpass_out=0.0f,
    .highpassCutoff = 0.9460737f,
    .nWaveshapers = 1
};
FxProgramType fxProgram1 = {
    .name = "Amp-Simulator        ",
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
    .data = (void*)&fxProgram1data
};
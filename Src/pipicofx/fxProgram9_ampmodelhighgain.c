#include "audio/fxprogram/fxProgram.h"
#include "stringFunctions.h"
#include "romfunc.h"

#define FXPROGRAM_HIGHCUT_VAL1 0.61035f
#define FXPROGRAM_HIGHCUT_VAL2 0.9613037f

#define FXPROGRAM1_HIGHCUT_DELTA (FXPROGRAM_HIGHCUT_VAL2-FXPROGRAM_HIGHCUT_VAL1)
static float fxProgramprocessSample(float sampleIn,void*data)
{
    float out;
    FxProgram9DataType* pData = (FxProgram9DataType*)data;

    pData->highpass_out =  (1.0f + pData->highpassCutoff)/2.0f*(sampleIn - pData->highpass_old_in) + pData->highpassCutoff*pData->highpass_old_out; //(((((1 << 15) + pData->highpassCutoff) >> 1)*(sampleIn - pData->highpass_old_in))>>15) + ((pData->highpassCutoff *pData->highpass_old_out) >> 15);
    pData->highpass_old_in = sampleIn;
    pData->highpass_old_out = pData->highpass_out;

    out = pData->highpass_out;

    // twice to overcome gain limitations due to interpolation
    out = multiWaveShaperProcessSample(out,&pData->waveshaper);
    out = multiWaveShaperProcessSample(out,&pData->waveshaper);

    out = out/2.0f;
    out = secondOrderIirFilterProcessSample(out,&pData->filter1);
    out = out/2.0f;
    out = firFilterProcessSample(out,&pData->filter3);
    out = delayLineProcessSample(out, pData->delay);
    return out;
}


static void fxProgramParam1Callback(uint16_t val,void*data) // highpass cutoff before the nonlinear stage
{
    float fval;
    FxProgram9DataType* pData = (FxProgram9DataType*)data;
    fval = ((FXPROGRAM1_HIGHCUT_DELTA*(float)val)/4096.0f);
    pData->highpassCutoff = FXPROGRAM_HIGHCUT_VAL1 + fval;
}

static void fxProgramParam1Display(void*data,char*res)
{
    uint32_t dval;
    FxProgram9DataType* pData = (FxProgram9DataType*)data;
    dval=(uint32_t)(pData->highpassCutoff*100.0f);
    Int16ToChar(dval,res);
}

static void fxProgramParam2Callback(uint16_t val,void*data) // number of waveshaper (more means more distortion)
{
    FxProgram9DataType* pData = (FxProgram9DataType*)data;
    // map 0-4095 to 0-63
    val >>= 6;
    pData->waveshaper.functionIndex=(uint8_t)val;
}

static void fxProgramParam2Display(void*data,char*res)
{
    FxProgram9DataType* pData = (FxProgram9DataType*)data;
    UInt8ToChar(pData->waveshaper.functionIndex,res);
}


static void fxProgramParam3Callback(uint16_t val,void*data) // delay intensity
{
    FxProgram9DataType* pData = (FxProgram9DataType*)data;
    pData->delay->delayInSamples = 2400 + (val << 3);
    pData->delay->mix = ((float)val)/4096.0f; // up to 100%
    pData->delay->feedback = 0.25f;
}

static void fxProgramParam3Display(void*data,char*res)
{
    int16_t dVal;
    FxProgram9DataType* pData = (FxProgram9DataType*)data;
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

static void fxProgramSetup(void*data)
{
    FxProgram9DataType* pData = (FxProgram9DataType*)data;
    initfirFilter(&pData->filter3);
    initMultiWaveShaper(&pData->waveshaper,&multiWaveshaper1);
    pData->delay = getDelayData();
    initDelay(pData->delay);
}


FxProgram9DataType fxProgram9data = {
    /* butterworth lowpass @ 6000Hz */
    .filter1 = {
        	.coeffB = {0.09763107f, 0.19526215f, 0.09763107f},
            .coeffA = {-0.94280904f, 0.33333333f},
            .w= {0.0f,0.0f,0.0f}
    },
    .filter3 = {
        .coefficients = {0.016731f, 0.017496f, 0.021249f, 0.031896f, 0.051696f, 0.083098f, 0.125237f, 0.160554f, 0.158897f, 0.113163f, 0.059528f, 0.022957f, -0.017582f, -0.054411f, -0.064864f, -0.061696f, -0.053666f, -0.033009f, -0.007943f, 0.008333f, 0.015584f, 0.018759f, 0.015862f, 0.012281f, 0.016486f, 0.020179f, 0.022272f, 0.020375f, 0.007048f, -0.009343f, -0.016356f, -0.011307f, 0.000459f, 0.011650f, 0.015174f, 0.007995f, 0.000715f, 0.000025f, -0.004465f, -0.010208f, -0.007323f, 0.001944f, 0.012667f, 0.022059f, 0.027738f, 0.028239f, 0.024619f, 0.019782f, 0.017925f, 0.018693f, 0.019640f, 0.018431f, 0.012868f, 0.005646f, -0.000878f, -0.006494f, -0.005713f, -0.000488f, 0.005790f, 0.010304f, 0.013693f, 0.016206f, 0.017209f, 0.016596f, }    },
    .highpass_old_in=0.0f,
    .highpass_old_out=0.0f,
    .highpass_out=0.0f,
    .highpassCutoff = 0.9460737f
};
FxProgramType fxProgram9 = {
    .name = "Amp High Gain       ",
    .nParameters = 3,
    .parameters = {
        {
            .name="Hi-Cut         ",
            .control=0,
            .increment=100,
            .rawValue=31500,
            .setParameter=&fxProgramParam1Callback,
            .getParameterValue=0,
            .getParameterDisplay=&fxProgramParam1Display
        },
        {
            .name="Gain          ",
            .control=1,
            .increment = 64, // 4096/8
            .rawValue=0,
            .setParameter=&fxProgramParam2Callback,
            .getParameterValue=0,
            .getParameterDisplay=&fxProgramParam2Display
        },
        {
            .name="Delay Intensity",
            .control=2,
            .increment=64,
            .rawValue=0,
            .setParameter=&fxProgramParam3Callback,
            .getParameterValue=0,
            .getParameterDisplay=&fxProgramParam3Display
        },
    },
    .processSample = &fxProgramprocessSample,
    .setup = &fxProgramSetup,
    .data = (void*)&fxProgram9data
};
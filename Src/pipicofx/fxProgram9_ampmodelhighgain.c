#include "audio/fxprogram/fxProgram.h"
#include "stringFunctions.h"
#include "romfunc.h"

#define FXPROGRAM9_HIGHCUT_VAL1 20000
#define FXPROGRAM9_HIGHCUT_VAL2 31500

#define FXPROGRAM9_HIGHCUT_DELTA (FXPROGRAM9_HIGHCUT_VAL2-FXPROGRAM9_HIGHCUT_VAL1)

static int16_t fxProgramprocessSample(int16_t sampleIn,void*data)
{
    int16_t out;
    FxProgram9DataType* pData = (FxProgram9DataType*)data;

    pData->highpass_out = (((((1 << 15) + pData->highpassCutoff) >> 1)*(sampleIn - pData->highpass_old_in))>>15) + ((pData->highpassCutoff *pData->highpass_old_out) >> 15);
    pData->highpass_old_in = sampleIn;
    pData->highpass_old_out = pData->highpass_out;

    out = pData->highpass_out;
    out =multiWaveShaperProcessSample(out,&pData->waveshaper);

    out = out >> 1;

    out = secondOrderIirFilterProcessSample(out,&pData->filter1);
    out >>= 2;
    out = firFilterProcessSample(out,&pData->filter3);
    out = delayLineProcessSample(out, pData->delay);
    return out;
}

static void fxProgramParam1Callback(uint16_t val,void*data) // highpass cutoff before the nonlinear stage
{
    uint32_t dval;
    FxProgram9DataType* pData = (FxProgram9DataType*)data;
    dval = ((FXPROGRAM9_HIGHCUT_DELTA*val) >> 12);
    pData->highpassCutoff = FXPROGRAM9_HIGHCUT_VAL1 + (int16_t)dval;
}

static void fxProgramParam1Display(void*data,char*res)
{
    FxProgram9DataType* pData = (FxProgram9DataType*)data;
    Int16ToChar(pData->highpassCutoff,res);
}

static void fxProgramParam2Callback(uint16_t val,void*data) // gain
{
    FxProgram9DataType* pData = (FxProgram9DataType*)data;
    // map 0-4095 to 0-63
    val >>= 6;
    pData->waveshaper.functionIndex = val;
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
    pData->delay->mix = val << 2; // up to 100%
    pData->delay->feedback = (1<< 14);
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
    /* butterworth lowpass @ 6000Hz */
    .filter1 = {
        	.coeffB = {3199, 6398, 3199},
            .coeffA = {-30893, 10922},
            .w= {0,0,0}, 
            .bitRes = 16
    },
    .filter3 = {
        .coefficients = {0x62c, 0x674, 0x7d6, 0xbc4, 0x1312, 0x1ea7, 0x2e33, 0x3b3a, 0x3a9e, 0x29bf, 0x15f5, 0x878, 0xf984, 0xebee, 0xe813, 0xe93e, 0xec34, 0xf3d3, 0xfd12, 0x312, 0x5bf, 0x6eb, 0x5da, 0x487, 0x614, 0x771, 0x837, 0x784, 0x299, 0xfc8e, 0xf9f8, 0xfbd5, 0x2b, 0x44c, 0x599, 0x2f3, 0x43, 0x2, 0xfe5b, 0xfc3c, 0xfd4d, 0xb7, 0x4ac, 0x823, 0xa3b, 0xa6a, 0x915, 0x74c, 0x69c, 0x6e5, 0x73e, 0x6cc, 0x4bf, 0x215, 0xffae, 0xfd9b, 0xfde5, 0xffd2, 0x222, 0x3cd, 0x50d, 0x5fa, 0x659, 0x61f}
    },
    .highpass_old_in=0,
    .highpass_old_out=0,
    .highpass_out=0,
    .highpassCutoff = 31000,
    .nWaveshapers = 1
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
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
    out =multiWaveShaperProcessSample(out,&pData->waveshaper1);
    out =multiWaveShaperProcessSample(out,&pData->waveshaper1);
    out = out >> 1;

    switch (pData->cabSimType)
    {
        case 0:
            out = secondOrderIirFilterProcessSample(out,&pData->filter1);
            out >>= 2;
            out = firFilterProcessSample(out,&pData->filter3);
            break;
        case 1:
            out >>= 2;
            out = firFilterProcessSample(out,&pData->hiwattFir);
            break;
        case 2:
            out = secondOrderIirFilterProcessSample(out,&pData->hiwattIir1);
            out = secondOrderIirFilterProcessSample(out,&pData->hiwattIir2);
            out = secondOrderIirFilterProcessSample(out,&pData->hiwattIir3);
            break;
        case 3:
            out >>= 2;
            out = firFilterProcessSample(out,&pData->frontmanFir);
            break;
        case 4:
            out = secondOrderIirFilterProcessSample(out,&pData->frontmanIir1);
            out = secondOrderIirFilterProcessSample(out,&pData->frontmanIir2);
            out = secondOrderIirFilterProcessSample(out,&pData->frontmanIir3);
            break;
        case 5:
            out >>= 2;
            out = firFilterProcessSample(out,&pData->voxAC15Fir);
            break;
        case 6:
            out = secondOrderIirFilterProcessSample(out,&pData->voxAC15Iir1);
            out = secondOrderIirFilterProcessSample(out,&pData->voxAC15Iir2);
            out = secondOrderIirFilterProcessSample(out,&pData->voxAC15Iir3);
            break;
        default:
            break;

    }
    if (pData->modType==0)
    {
        out = delayLineProcessSample(out, pData->delay);
    }
    else
    {
        out = reverbProcessSample(out,&pData->reverb);
    }
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
    pData->waveshaper1.functionIndex = val;
}


static void fxProgramParam2Display(void*data,char*res)
{
    FxProgram9DataType* pData = (FxProgram9DataType*)data;
    UInt8ToChar(pData->waveshaper1.functionIndex,res);
}


static void fxProgramParam3Callback(uint16_t val,void*data) // delay/reverb intensity
{
    FxProgram9DataType* pData = (FxProgram9DataType*)data;
    pData->delay->delayInSamples = 2400 + (val << 3);
    pData->delay->mix = val << 2; // up to 100%
    pData->delay->feedback = (1<< 14);

    setReverbTime(500 + (val>>2),&pData->reverb);
    pData->reverb.mix = val << 2;
}

static void fxProgramParam3Display(void*data,char*res)
{
    int16_t dVal;
    FxProgram9DataType* pData = (FxProgram9DataType*)data;
    dVal=pData->delay->mix/164;
    Int16ToChar(dVal,res);
    appendToString(res,"%");
}

static void fxProgramParam4Callback(uint16_t val,void*data) // modulation type
{
    FxProgram9DataType* pData = (FxProgram9DataType*)data;
    pData->modType = val >> 11;
}

static void fxProgramParam4Display(void*data,char*res) // modulation type
{
    FxProgram9DataType* pData = (FxProgram9DataType*)data;
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

static void fxProgramParam5Callback(uint16_t val,void*data) // cab type
{
    FxProgram9DataType* pData = (FxProgram9DataType*)data;
    pData->cabSimType = val >> 9;
}

static void fxProgramParam5Display(void*data,char*res) // cab type
{
    FxProgram9DataType* pData = (FxProgram9DataType*)data;
    for(uint8_t c=0;c<24;c++)
    {
        *(res+c)=pData->cabNames[pData->cabSimType][c];
    }
}

static void fxProgramSetup(void*data)
{
    FxProgram9DataType* pData = (FxProgram9DataType*)data;
    initfirFilter(&pData->filter3);
    initfirFilter(&pData->frontmanFir);
    initfirFilter(&pData->hiwattFir);
    initfirFilter(&pData->voxAC15Fir);
    initMultiWaveShaper(&pData->waveshaper1,&multiWaveshaper1);
    pData->delay = getDelayData();
    initDelay(pData->delay);
    initReverb(&pData->reverb,500);
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
    .hiwattFir = {
        .coefficients= {-42, -42, -6, 317, 1371, 3572, 7006, 11147, 14461, 14434, 9110, 4, -7520, -9233, -8098, -6638, -4065, -601, 2424, 3345, 1940, 242, 302, 976, 809, 424, -56, -633, -650, -448, -767, -1168, -598, 952, 2050, 1294, -1130, -3011, -3906, -3785, -2258, -467, 841, 1033, 81, -771, -368, 398, 469, 505, 527, -281, -1373, -2394, -3074, -2888, -2175, -1570, -877, 258, 821, 194, -528, -772}
        },
    .hiwattIir1 = 
    { // gain: 0.516
        .bitRes = 16,
        .coeffA = {264, -7532},
        .coeffB = {16923, 11321, -3276},
        .w = {0,0,0}
    },
    .hiwattIir2 = 
    { // gain: 0.75
        .bitRes = 16,
        .coeffA = {-2243, 967},
        .coeffB = {24688, 4010, 4376},
        .w = {0,0,0}
    },
    .hiwattIir3 = 
    { // gain: 0.11
        .bitRes = 16,
        .coeffA = {-145, -7285},
        .coeffB = {3736, 5240, 3361},
        .w = {0,0,0}
    },
    .frontmanFir = {
        .coefficients = {-513, -1756, -3336, -5308, -7784, -9918, -9872, -6752, -2173, 1032, 1765, 1104, 18, 210, 2189, 4246, 4499, 2834, 138, -2553, -4127, -3345, 241, 4895, 7840, 7047, 3855, 1730, 986, -265, -2714, -4937, -5139, -3073, -59, 2420, 3183, 2875, 2107, -511, -4239, -5117, -1723, 3057, 5753, 5872, 5194, 4838, 5248, 6182, 4671, 1171, -1765, -2505, -1594, -619, -843, -1589, -749, 2533, 6338, 6855, 3017, -236}
    },
    .frontmanIir1 = 
    { // gain: 0.14
        .bitRes = 16,
        .coeffA = {4753, -7562},
        .coeffB = {4649, 893, 5816},
        .w = {0,0,0}
    },
    .frontmanIir2 = 
    { // gain: 1.005
        .bitRes = 16,
        .coeffA = {-7357, -1143},
        .coeffB = {32931, 5701, -17718},
        .w = {0,0,0}
    },
    .frontmanIir3 = 
    { // gain: 0.179
        .bitRes = 16,
        .coeffA = {-6330, 3379},
        .coeffB = {5892, 8922, 4388},
        .w = {0,0,0}
    },
    .voxAC15Fir = {
        .coefficients = {-1110, -3127, -5900, -8409, -10180, -10897, -12238, -13579, -10051, -88, 7780, 6320, 468, -2894, -2063, 35, 2166, 2992, 2402, 1810, 2334, 2426, -195, -2013, 2505, 5798, 2919, 1492, 579, -925, -373, 1467, 2484, 2947, 3693, 3601, 1725, 241, 410, 1402, 2221, 2607, 2216, 776, -954, -1518, -677, 129, 1350, 3138, 130, -1561, -88, 646, 888, 1493, 1431, 811, 1202, 1587, 390, 548, 1266, 56}
    },
    .voxAC15Iir1 = {
        // gain: 0.966
        .bitRes = 16,
        .coeffB = {31668, -922, 6985},
        .coeffA = {2752, -1786},
        .w = {0,0,0}
    },
    .voxAC15Iir2 = {
        // gain: 1.00
        .bitRes = 16,
        .coeffB = {32767, -956, -7750},
        .coeffA = {517, 8698},
        .w = {0,0,0}
    },
    .voxAC15Iir3 = {
        // gain: 0.08
        .bitRes = 16,
        .coeffB = {2674, 5068, 3204},
        .coeffA = {-6259, -14319},
        .w = {0,0,0}
    },
    .cabNames = {
        "Custom             ",
        "Hiwatt M412     (F)", 
        "Hiwatt M412     (I)",
        "Fender Frontman (F)",
        "Fender Frontman (I)",
        "Vox AC15        (F)",
        "Vox AC15        (I)",
        "Off                " 
    },
    .modType=0,
    .highpass_old_in=0,
    .highpass_old_out=0,
    .highpass_out=0,
    .highpassCutoff = 31000,
    .nWaveshapers = 1
};
FxProgramType fxProgram9 = {
    .name = "Amp High Gain       ",
    .nParameters = 5,
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
            .name="Gain           ",
            .control=1,
            .increment = 64, // 4096/8
            .rawValue=0,
            .setParameter=&fxProgramParam2Callback,
            .getParameterValue=0,
            .getParameterDisplay=&fxProgramParam2Display
        },
        {
            .name="Mod Intensity  ",
            .control=2,
            .increment=64,
            .rawValue=0,
            .setParameter=&fxProgramParam3Callback,
            .getParameterValue=0,
            .getParameterDisplay=&fxProgramParam3Display
        },
        {
            .name="Mod Type       ",
            .control=0xFF,
            .increment=2048,
            .rawValue=0,
            .setParameter=&fxProgramParam4Callback,
            .getParameterValue=0,
            .getParameterDisplay=&fxProgramParam4Display
        },
        {
            .name="Cab Type       ",
            .control=0xFF,
            .increment=512,
            .rawValue=0,
            .setParameter=&fxProgramParam5Callback,
            .getParameterValue=0,
            .getParameterDisplay=&fxProgramParam5Display
        }
    },
    .processSample = &fxProgramprocessSample,
    .setup = &fxProgramSetup,
    .data = (void*)&fxProgram9data
};
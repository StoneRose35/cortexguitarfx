#include "pipicofx/fxPrograms.h"
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
            out = secondOrderIirFilterProcessSample(out,&pData->customCabIir);
            out >>= 1;
            out = firFilterProcessSample(out,&pData->customCabFir);
            break;
        case 1:
            //out >>= 1;
            out = secondOrderIirFilterProcessSample(out,&pData->customIir1);
            out = secondOrderIirFilterProcessSample(out,&pData->customIir2);
            out = secondOrderIirFilterProcessSample(out,&pData->customIir3);
            out <<= 2;
            out = secondOrderIirFilterProcessSample(out,&pData->customIir4);
            break;
        case 2:
            out >>= 2;
            out = firFilterProcessSample(out,&pData->hiwattFir);
            break;
        case 3:
            out = secondOrderIirFilterProcessSample(out,&pData->hiwattIir1);
            out = secondOrderIirFilterProcessSample(out,&pData->hiwattIir2);
            out = secondOrderIirFilterProcessSample(out,&pData->hiwattIir3);
            break;
        case 4:
            out >>= 2;
            out = firFilterProcessSample(out,&pData->frontmanFir);
            break;
        case 5:
            out = secondOrderIirFilterProcessSample(out,&pData->frontmanIir1);
            out = secondOrderIirFilterProcessSample(out,&pData->frontmanIir2);
            out = secondOrderIirFilterProcessSample(out,&pData->frontmanIir3);
            break;
        case 6:
            out >>= 2;
            out = firFilterProcessSample(out,&pData->voxAC15Fir);
            break;
        case 7:
            out = secondOrderIirFilterProcessSample(out,&pData->voxAC15Iir1);
            out = secondOrderIirFilterProcessSample(out,&pData->voxAC15Iir2);
            out = secondOrderIirFilterProcessSample(out,&pData->voxAC15Iir3);
            break;
        default:
            break;

    }
    if (pData->modType==0)
    {
        out = delayLineProcessSample(out, &pData->delay);
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
    fxProgram9.parameters[0].rawValue = val;
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
    fxProgram9.parameters[1].rawValue = val;
}


static void fxProgramParam2Display(void*data,char*res)
{
    FxProgram9DataType* pData = (FxProgram9DataType*)data;
    UInt8ToChar(pData->waveshaper1.functionIndex,res);
}


static void fxProgramParam3Callback(uint16_t val,void*data) // delay/reverb intensity
{
    FxProgram9DataType* pData = (FxProgram9DataType*)data;
    pData->delay.delayInSamples = 2400 + (val << 3);
    pData->delay.mix = val << 2; // up to 100%
    pData->delay.feedback = (1<< 14);

    setReverbTime(500 + (val>>2),&pData->reverb);
    pData->reverb.mix = val << 2;
    fxProgram9.parameters[2].rawValue = val;
}

static void fxProgramParam3Display(void*data,char*res)
{
    int16_t dVal;
    FxProgram9DataType* pData = (FxProgram9DataType*)data;
    dVal=pData->delay.mix/164;
    Int16ToChar(dVal,res);
    appendToString(res,"%");
}

static void fxProgramParam4Callback(uint16_t val,void*data) // modulation type
{
    FxProgram9DataType* pData = (FxProgram9DataType*)data;
    pData->modType = val >> 11;
    fxProgram9.parameters[3].rawValue = val;
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
    pData->cabSimType = val >> 8;
    if (pData->cabSimType > 8)
    {
        pData->cabSimType=8;
    }
    fxProgram9.parameters[4].rawValue = val;
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
    initfirFilter(&pData->customCabFir);
    initfirFilter(&pData->frontmanFir);
    initfirFilter(&pData->hiwattFir);
    initfirFilter(&pData->voxAC15Fir);
    initMultiWaveShaper(&pData->waveshaper1,&multiWaveshaper1);
    initDelay(&pData->delay,getDelayMemoryPointer(),DELAY_LINE_LENGTH);
    initReverb(&pData->reverb,500);
}

static void fxProgram9Reset(void*data)
{
    FxProgram9DataType* pData = (FxProgram9DataType*)data;    

    pData->highpass_old_in = 0;
    pData->highpass_old_out = 0;

    if (pData->cabSimType == 0)
    {
        firFilterReset(&pData->hiwattFir);
    }
    else if (pData->cabSimType == 1)
    {
        secondOrderIirFilterReset(&pData->hiwattIir1);
        secondOrderIirFilterReset(&pData->hiwattIir2);
        secondOrderIirFilterReset(&pData->hiwattIir3);
    }
    else if (pData->cabSimType == 2)
    {
        firFilterReset(&pData->frontmanFir);
    }
    else if (pData->cabSimType == 3)
    {
        secondOrderIirFilterReset(&pData->frontmanIir1);
        secondOrderIirFilterReset(&pData->frontmanIir2);
        secondOrderIirFilterReset(&pData->frontmanIir3);
    }
    else if (pData->cabSimType == 4)
    {
        firFilterReset(&pData->voxAC15Fir);
    }
    else if (pData->cabSimType == 5)
    {
        secondOrderIirFilterReset(&pData->voxAC15Iir1);
        secondOrderIirFilterReset(&pData->voxAC15Iir2);
        secondOrderIirFilterReset(&pData->voxAC15Iir3);
    }
}


FxProgram9DataType fxProgram9data = {
    /* butterworth lowpass @ 6000Hz */
    /* butterworth lowpass @ 6000Hz */
    .customCabIir = {
        	//.coeffB = {3199, 6398, 3199},
            //.coeffA = {-30893, 10922},
            .coeffB = {1599, 3199, 1599},
            .coeffA = {-15446, 5461},
            .y1=0,
            .y2=0,
            .x1=0,
            .x2=0,
            .acc=0
    },
    .customCabFir = {
        .coefficients = {0x62c, 0x674, 0x7d6, 0xbc4, 0x1312, 0x1ea7, 0x2e33, 0x3b3a, 0x3a9e, 0x29bf, 0x15f5, 0x878, 0xf984, 0xebee, 0xe813, 0xe93e, 0xec34, 0xf3d3, 0xfd12, 0x312, 0x5bf, 0x6eb, 0x5da, 0x487, 0x614, 0x771, 0x837, 0x784, 0x299, 0xfc8e, 0xf9f8, 0xfbd5, 0x2b, 0x44c, 0x599, 0x2f3, 0x43, 0x2, 0xfe5b, 0xfc3c, 0xfd4d, 0xb7, 0x4ac, 0x823, 0xa3b, 0xa6a, 0x915, 0x74c, 0x69c, 0x6e5, 0x73e, 0x6cc, 0x4bf, 0x215, 0xffae, 0xfd9b, 0xfde5, 0xffd2, 0x222, 0x3cd, 0x50d, 0x5fa, 0x659, 0x61f}
    },
    .customIir1 = {
        .coeffA={-28250, 13700},
        .coeffB={229,459,229},
        .y1=0,
        .y2=0,
        .x1=0,
        .x2=0,
        .acc=0
    },
    .customIir2 = {
        .coeffA={-29836,14408},
        .coeffB={119,239,119},
        .y1=0,
        .y2=0,
        .x1=0,
        .x2=0,
        .acc=0
    },
    .customIir3 = {
        .coeffA={-32700,16325},
        .coeffB={2591,-5183,2591},
        .y1=0,
        .y2=0,
        .x1=0,
        .x2=0,
        .acc=0
    },
    .customIir4 = {
        .coeffA={-31856,15497},
        .coeffB={15934,-31868,15934},
        .y1=0,
        .y2=0,
        .x1=0,
        .x2=0,
        .acc=0
    },
    .hiwattFir = {
        .coefficients= {-42, -42, -6, 317, 1371, 3572, 7006, 11147, 14461, 14434, 9110, 4, -7520, -9233, -8098, -6638, -4065, -601, 2424, 3345, 1940, 242, 302, 976, 809, 424, -56, -633, -650, -448, -767, -1168, -598, 952, 2050, 1294, -1130, -3011, -3906, -3785, -2258, -467, 841, 1033, 81, -771, -368, 398, 469, 505, 527, -281, -1373, -2394, -3074, -2888, -2175, -1570, -877, 258, 821, 194, -528, -772}
        },
    .hiwattIir1 = 
    { 
        .coeffA = {-16557, 7532},
        .coeffB =  {-319, -722, -382},
        .y1=0,
        .y2=0,
        .x1=0,
        .x2=0,
        .acc=0
    },
    .hiwattIir2 = 
    { 
        .coeffA = {3428,3068},
        .coeffB = {4426,5564, 871},
        .y1=0,
        .y2=0,
        .x1=0,
        .x2=0,
        .acc=0
    },
    .hiwattIir3 = 
    { 
        .coeffA = {2860, 1804},
        .coeffB = {-10189, -4775, -2987},
        .y1=0,
        .y2=0,
        .x1=0,
        .x2=0,
        .acc=0
    },
    .frontmanFir = {
        .coefficients = {-513, -1756, -3336, -5308, -7784, -9918, -9872, -6752, -2173, 1032, 1765, 1104, 18, 210, 2189, 4246, 4499, 2834, 138, -2553, -4127, -3345, 241, 4895, 7840, 7047, 3855, 1730, 986, -265, -2714, -4937, -5139, -3073, -59, 2420, 3183, 2875, 2107, -511, -4239, -5117, -1723, 3057, 5753, 5872, 5194, 4838, 5248, 6182, 4671, 1171, -1765, -2505, -1594, -619, -843, -1589, -749, 2533, 6338, 6855, 3017, -236}
    },
    .frontmanIir1 = 
    { 
        .coeffA = {-16836, 5299},
        .coeffB = {-148, -389, -436},
        .y1=0,
        .y2=0,
        .x1=0,
        .x2=0,
        .acc=0
    },
    .frontmanIir2 = 
    { 
        .coeffA = {-2990, -1681},
        .coeffB = {-25230, -12670, 2490},
        .y1=0,
        .y2=0,
        .x1=0,
        .x2=0,
        .acc=0
    },
    .frontmanIir3 = 
    { 
        .coeffA = {-3343, -4287},
        .coeffB = {-7552, 3249, 6865},
        .y1=0,
        .y2=0,
        .x1=0,
        .x2=0,
        .acc=0
    },
    .voxAC15Fir = {
        .coefficients = {-1110, -3127, -5900, -8409, -10180, -10897, -12238, -13579, -10051, -88, 7780, 6320, 468, -2894, -2063, 35, 2166, 2992, 2402, 1810, 2334, 2426, -195, -2013, 2505, 5798, 2919, 1492, 579, -925, -373, 1467, 2484, 2947, 3693, 3601, 1725, 241, 410, 1402, 2221, 2607, 2216, 776, -954, -1518, -677, 129, 1350, 3138, 130, -1561, -88, 646, 888, 1493, 1431, 811, 1202, 1587, 390, 548, 1266, 56}
    },
    .voxAC15Iir1 = {
        .coeffB = {-465, -1525, -1652},
        .coeffA = {-10492, -1432},
        .y1=0,
        .y2=0,
        .x1=0,
        .x2=0,
        .acc=0
    },
    .voxAC15Iir2 = {
        .coeffB = {-12044, 17013, 3125},
        .coeffA = {-5755, 4143},
        .y1=0,
        .y2=0,
        .x1=0,
        .x2=0,
        .acc=0
    },
    .voxAC15Iir3 = {
        .coeffB = {-13053, -3329, -548},
        .coeffA = {-1159, -1059},
        .y1=0,
        .y2=0,
        .x1=0,
        .x2=0,
        .acc=0
    },
    .cabNames = {
        "Custom             ",
        "Custom IIR         ",
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
    .nWaveshapers = 1,
    .reverb.paramNr=0
};
FxProgramType fxProgram9 = {
    .name = "Amp High Gain",
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
            .increment = 32, // 4096/8
            .rawValue=0,
            .setParameter=&fxProgramParam2Callback,
            .getParameterValue=0,
            .getParameterDisplay=&fxProgramParam2Display
        },
        {
            .name="Mod Intensity  ",
            .control=2,
            .increment=32,
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
            .increment=256,
            .rawValue=0,
            .setParameter=&fxProgramParam5Callback,
            .getParameterValue=0,
            .getParameterDisplay=&fxProgramParam5Display
        }
    },
    .processSample = &fxProgramprocessSample,
    .setup = &fxProgramSetup,
    .reset = &fxProgram9Reset,
    .data = (void*)&fxProgram9data
};
#include <stdint.h>
#include "audio/fxprogram/fxProgram.h"
#include "stringFunctions.h"

#define P4_HIGHPASS 30000

static int16_t fxProgram4processSample(int16_t sampleIn,void*data)
{
    int32_t out;
    FxProgram4DataType* pData = (FxProgram4DataType*)data;

    pData->highpass_out = (((((1 << 15) + P4_HIGHPASS) >> 1)*(sampleIn - pData->highpass_old_in))>>15) + ((P4_HIGHPASS *pData->highpass_old_out) >> 15);
    pData->highpass_old_in = sampleIn;
    pData->highpass_old_out = pData->highpass_out;

    out = pData->highpass_out;
    //out = sampleIn;
    out = gainStageProcessSample(out, &pData->gainStage);

    for (uint8_t c=0;c<pData->nWaveshapers;c++)
    {
        out = waveShaperProcessSample(out,&pData->waveshaper1.waveshaper); 
    }

    out = out >> 1;
    if (pData->cabSimType == 0)
    {
        out >>= 2;
        out = firFilterProcessSample(out,&pData->hiwattFir);
    }
    else if (pData->cabSimType == 1)
    {
        out = secondOrderIirFilterProcessSample(out,&pData->hiwattIir1);
        out = secondOrderIirFilterProcessSample(out,&pData->hiwattIir2);
        out = secondOrderIirFilterProcessSample(out,&pData->hiwattIir3);
    }
    else if (pData->cabSimType == 2)
    {
        out >>= 2;
        out = firFilterProcessSample(out,&pData->frontmanFir);
    }
    else if (pData->cabSimType == 3)
    {
        out = secondOrderIirFilterProcessSample(out,&pData->frontmanIir1);
        out = secondOrderIirFilterProcessSample(out,&pData->frontmanIir2);
        out = secondOrderIirFilterProcessSample(out,&pData->frontmanIir3);
    }
    else if (pData->cabSimType == 4)
    {
        out >>= 2;
        out = firFilterProcessSample(out,&pData->voxAC15Fir);
    }
    else if (pData->cabSimType == 5)
    {
        out = secondOrderIirFilterProcessSample(out,&pData->voxAC15Iir1);
        out = secondOrderIirFilterProcessSample(out,&pData->voxAC15Iir2);
        out = secondOrderIirFilterProcessSample(out,&pData->voxAC15Iir3);
    }
    return out;
}


static void fxProgram4Param1Callback(uint16_t val,void*data) // gain
{
    FxProgram4DataType* pData = (FxProgram4DataType*)data;
    pData->gainStage.gain = ((val << 2) + 0x3F); //pData->gainStage.gain + ((FXPROGRAM6_DELAY_TIME_LOWPASS_T*(((val << 2) + 0x3F) - pData->gainStage.gain)) >> 8);
    val >>= 9;
    val += 1;
    //pData->nWaveshapers = val;
}

static void fxProgram4Param1Display(void*data,char*res)
{
    //FxProgramType * fData = (FxProgramType*)data;
    FxProgram4DataType* pData = (FxProgram4DataType*)data;
    fixedPointInt16ToChar(res,pData->gainStage.gain,8);
}


static void fxProgram4Param2Callback(uint16_t val,void*data) // offset
{
    FxProgram4DataType* pData = (FxProgram4DataType*)data;
    //pData->gainStage.offset = (val << 4) - 0x7FFF; 
    pData->gainStage.offset = ((val - 0x7FF) << 4);//pData->gainStage.offset + ((FXPROGRAM6_DELAY_TIME_LOWPASS_T*(((val - 0x7FF) << 4)  - pData->gainStage.offset)) >> 8);
}

static void fxProgram4Param2Display(void*data,char*res)
{
    //FxProgramType * fData = (FxProgramType*)data;
    FxProgram4DataType* pData = (FxProgram4DataType*)data;
    Int16ToChar(pData->gainStage.offset,res);
}


static void fxProgram4Param3Callback(uint16_t val,void*data) // choose cab sim filters
{
    FxProgram4DataType* pData = (FxProgram4DataType*)data;
    pData->cabSimType = (val >> 9) & 7;
}

static void fxProgram4Param3Display(void*data,char*res)
{
    FxProgram4DataType* pData = (FxProgram4DataType*)data;
    for(uint8_t c=0;c<24;c++)
    {
        *(res+c)=*(pData->cabNames[pData->cabSimType]+c);
    }
}

static void fxProgram4Param4Callback(uint16_t val,void*data)
{
    FxProgram4DataType* pData = (FxProgram4DataType*)data;
    pData->nWaveshapers = (val >> 10) + 1;
}

static void fxProgram4Param4Display(void*data,char*res)
{
    FxProgram4DataType* pData = (FxProgram4DataType*)data;
    UInt8ToChar(pData->nWaveshapers,res);
}


static void fxProgram4Param5Callback(uint16_t val,void*data)
{
    FxProgram4DataType* pData = (FxProgram4DataType*)data;
    pData->waveshaperType = (val >> 10);
    switch(pData->waveshaperType)
    {
        case 0:
            initWaveShaper(&pData->waveshaper1.waveshaper,&waveShaperDefaultOverdrive);
            break;
        case 1:
            initWaveShaper(&pData->waveshaper1.waveshaper,&waveShaperSoftOverdrive);
            break;
        case 2: 
            initWaveShaper(&pData->waveshaper1.waveshaper,&waveShaperDistortion);
            break;            
        case 3:
            initWaveShaper(&pData->waveshaper1.waveshaper,&waveShaperCurvedOverdrive);
            break;
        default:
            break;
    }
}

static void fxProgram4Param5Display(void*data,char*res)
{
    FxProgram4DataType* pData = (FxProgram4DataType*)data;
    for(uint8_t c=0;c<24;c++)
    {
        *(res+c) =  pData->waveShaperNames[pData->waveshaperType][c];
    }
}


static void fxProgram4Setup(void*data)
{
    FxProgram4DataType* pData = (FxProgram4DataType*)data;
    initWaveShaper(&pData->waveshaper1.waveshaper,&waveShaperAsymm);
    initfirFilter(&pData->frontmanFir);
    initfirFilter(&pData->hiwattFir);
    initfirFilter(&pData->voxAC15Fir);
}

FxProgram4DataType fxProgram4data = {
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
        "Hiwatt M412     (F)", 
        "Hiwatt M412     (I)",
        "Fender Frontman (F)",
        "Fender Frontman (I)",
        "Vox AC15        (F)",
        "Vox AC15        (I)"  
    },
    .waveShaperNames = {
        "Overdrive 1   ",
        "Soft Overdrive",
        "Distortion    ",
        "OVerdrive 2   "
    },
    .gainStage.gain=512,
    .nWaveshapers = 4,
    .waveshaperType=0,
    .cabSimType = 1,
    .waveshaper1 = {
        .oversamplingFilter = {
            .coeffB = {3199, 6398, 3199},
            .coeffA = {-30893, 10922}
        }
    }
};

FxProgramType fxProgram4 = {
    .name = "Amp Model 2          ",
    .nParameters=5,
    .parameters = {
        {
            .name = "Gain           ",
            .control=0,
            .increment=64,
            .rawValue=0,
            .getParameterDisplay=&fxProgram4Param1Display,
            .getParameterValue=0,
            .setParameter=&fxProgram4Param1Callback
        },
        {
            .name = "DC-Offset      ",
            .control=1,
            .increment=64,
            .rawValue=0,
            .getParameterDisplay=&fxProgram4Param2Display,
            .getParameterValue=0,
            .setParameter=&fxProgram4Param2Callback
        },   
        {
            .name = "Cab Type       ",
            .control=2,
            .increment=64,
            .rawValue=0,
            .getParameterDisplay=&fxProgram4Param3Display,
            .getParameterValue=0,
            .setParameter=&fxProgram4Param3Callback
        },
        {
            .name = "Gainstages     ",
            .control=3,
            .increment=1024,
            .rawValue=0,
            .getParameterDisplay=&fxProgram4Param4Display,
            .getParameterValue=0,
            .setParameter=&fxProgram4Param4Callback
        },
        {
            .name = "OD/Dist Type",
            .control = 4,
            .increment = 1024,
            .rawValue = 0,
            .getParameterDisplay=&fxProgram4Param5Display,
            .getParameterValue=0,
            .setParameter=&fxProgram4Param5Callback
        }     
    },
    .processSample = &fxProgram4processSample,
    .setup = &fxProgram4Setup,
    .data = (void*)&fxProgram4data
};

/*
        {
            .name = "Frequency      ",
            .control=0,
            .minValue=0,
            .maxValue=32767,
            .rawValue=0,
            .getParameterDisplay=&fxProgram2Param1Display,
            .getParameterValue=0,
            .setParameter=&fxProgram2Param1Callback
        },
        */
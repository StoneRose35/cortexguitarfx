#include <stdint.h>
#include "audio/fxprogram/fxProgram.h"
#include "stringFunctions.h"

#define P4_HIGHPASS 0.997f //0.9155552f

static float fxProgram4processSample(float sampleIn,void*data)
{
    float out;
    FxProgram4DataType* pData = (FxProgram4DataType*)data;



    out = sampleIn + pData->gainStage.offset;
    //out = gainStageProcessSample(out, &pData->gainStage);

    //for (uint8_t c=0;c<pData->nWaveshapers;c++)
    //{
        out = multiWaveShaperProcessSample(out,&pData->waveshaper1); 
    //}

    pData->highpass_out = ((((1.0f + P4_HIGHPASS)/2.0f)*(out - pData->highpass_old_in))) + ((P4_HIGHPASS *pData->highpass_old_out));
    pData->highpass_old_in = out;
    pData->highpass_old_out = pData->highpass_out;

    out = pData->highpass_out;

    if (pData->cabSimType == 0)
    {
        out /=4.0f;
        out = firFilterProcessSample(out,&pData->hiwattFir);
    }
    else if (pData->cabSimType == 1)
    {
        out = secondOrderIirFilterProcessSample(out,&pData->hiwattIir1);
        out = secondOrderIirFilterProcessSample(out,&pData->hiwattIir2);
        out = secondOrderIirFilterProcessSample(out,&pData->hiwattIir3);
        out=out*1.6f;
    }
    else if (pData->cabSimType == 2)
    {
        out /=4.0f;
        out = firFilterProcessSample(out,&pData->frontmanFir);
    }
    else if (pData->cabSimType == 3)
    {
        out = secondOrderIirFilterProcessSample(out,&pData->frontmanIir1);
        out = secondOrderIirFilterProcessSample(out,&pData->frontmanIir2);
        out = secondOrderIirFilterProcessSample(out,&pData->frontmanIir3);
        out=out*3.0f;
    }
    else if (pData->cabSimType == 4)
    {
        out /=4.0f;
        out = firFilterProcessSample(out,&pData->voxAC15Fir);
    }
    else if (pData->cabSimType == 5)
    {
        out = secondOrderIirFilterProcessSample(out,&pData->voxAC15Iir1);
        out = secondOrderIirFilterProcessSample(out,&pData->voxAC15Iir2);
        out = secondOrderIirFilterProcessSample(out,&pData->voxAC15Iir3);
        out=out*2.0f;
    }
    return out;
}


static void fxProgram4Param1Callback(uint16_t val,void*data) // gain
{
    FxProgram4DataType* pData = (FxProgram4DataType*)data;
    pData->gainStage.gain = ((float)val)/64.0f + 0.25f;//((val << 2) + 0x3F); 
    pData->waveshaper1.functionIndex = (uint8_t)(val >> 6);
}

static void fxProgram4Param1Display(void*data,char*res)
{
    //FxProgramType * fData = (FxProgramType*)data;
    FxProgram4DataType* pData = (FxProgram4DataType*)data;
    fixedPointInt16ToChar(res,(int)(pData->gainStage.gain*256.0f),8);
}


static void fxProgram4Param2Callback(uint16_t val,void*data) // offset
{
    FxProgram4DataType* pData = (FxProgram4DataType*)data;
    pData->gainStage.offset = ((float)(val - 0x7ff))/2048.0f*0.1f; //((val - 0x7FF) << 4);
}

static void fxProgram4Param2Display(void*data,char*res)
{
    //FxProgramType * fData = (FxProgramType*)data;
    FxProgram4DataType* pData = (FxProgram4DataType*)data;
    Int16ToChar((int)(pData->gainStage.offset*100.0f),res);
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

static void fxProgram4Setup(void*data)
{
    FxProgram4DataType* pData = (FxProgram4DataType*)data;
    initMultiWaveShaper(&pData->waveshaper1,&multiWaveshaper1);
    initfirFilter(&pData->frontmanFir);
    initfirFilter(&pData->hiwattFir);
    initfirFilter(&pData->voxAC15Fir);
}

FxProgram4DataType fxProgram4data = {
    .hiwattFir = {
        .coefficients={-0.0017023176900787217f, -0.0017005268239144186f, -0.0002766393406549576f, 0.01256757041350998f, 0.054334614505846146f, 0.14155816701882798f, 0.27757270218921326f, 0.4416399616001655f, 0.572964700555003f, 0.5718977731210754f, 0.36095148035982844f, 0.00016757643365051793f, -0.2979448304135627f, -0.3658070862900467f, -0.3208742968977083f, -0.26302463634936424f, -0.157722049401935f, -0.02285093562091892f, 0.09005723522672653f, 0.12150493898307178f, 0.06888695592185305f, 0.008413404785681415f, 0.010248397117904838f, 0.032237922081451605f, 0.026056849646679217f, 0.013304316297298205f, -0.0017226964920602041f, -0.018826221776161764f, -0.018798010475040147f, -0.012576004397197497f, -0.0209095685291596f, -0.030871892222157714f, -0.015317825458749804f, 0.023597736223356494f, 0.04907733102160324f, 0.02991884693117229f, -0.02520356224985369f, -0.06463771271320064f, -0.08060429017830055f, -0.07498587573631023f, -0.042868942066002856f, -0.008488205257352593f, 0.01459429004800938f, 0.017053010942626373f, 0.001283191421817481f, -0.011462552843207572f, -0.00516728180421877f, 0.005265678481851653f, 0.005808467642090153f, 0.0058450906443413995f, 0.005663194999486701f, -0.002789344927011968f, -0.012473611719398542f, -0.019760659686461873f, -0.02283930810937319f, -0.01907222795358099f, -0.012567329282514044f, -0.007775787898556212f, -0.0036223929087269386f, 0.0008532298181232878f, 0.0020345896097381053f, 0.0003218540337240706f, -0.00043645456294634326f, -0.0f,} 
    },
    .hiwattIir1 = 
    {
        .coeffA = {0.392478f, -0.347447f},
        .coeffB = {1.819060f, 0.200088f, -0.492629f},
        .w = {0.0f,0.0f,0.0f}
    },
    .hiwattIir2 = 
    { 
        .coeffA = {0.209262f, 0.187307f},
        .coeffB = {0.270218f, 0.339651f, 0.053222f},
        .w = {0.0f,0.0f,0.0f}
    },
    .hiwattIir3 = 
    { 
        .coeffA = {-0.589933f, 0.036490f},
        .coeffB = {0.057732f, 0.115819f, 0.088076f},
        .w = {0.0f,0.0f,0.0f}
    },
    .frontmanFir = {
        .coefficients = {0.03654710088402896f, 0.12497848156010063f, 0.23744999439478948f, 0.37782511137245767f, 0.5539792000325803f, 0.7058623283194672f, 0.7025699424907637f, 0.48055329442977496f, 0.15465290966172723f, -0.07344711329550871f, -0.12566720685376578f, -0.0785888017082614f, -0.0013375391453091816f, -0.014998537421773565f, -0.1557874444654182f, -0.3022287394132401f, -0.31352360787087086f, -0.19329877802963924f, -0.009262257862918807f, 0.16655312261285046f, 0.2631509214071784f, 0.20830956977580375f, -0.014708752479247415f, -0.2903324896545964f, -0.45335646247210765f, -0.3970672913784171f, -0.2115152616361239f, -0.09238389277348516f, -0.051203247125342195f, 0.013377898779717002f, 0.13283067934863552f, 0.2342441124287884f, 0.23620220014046953f, 0.13671648199047695f, 0.002551281468891945f, -0.10049826426558385f, -0.12745834838159972f, -0.11083852932702803f, -0.0781049183470196f, 0.018193368815422245f, 0.14456969589411553f, 0.1669405906902758f, 0.05366247930697673f, -0.09065520189471071f, -0.16208615659082248f, -0.1567196880226402f, -0.13092569004216256f, -0.11477356133531635f, -0.1167279232211738f, -0.1283357166278733f, -0.09004291511770321f, -0.02084243866307915f, 0.028789609749632985f, 0.037148502190216884f, 0.021275031774832878f, 0.007353872493733518f, 0.008750877801044861f, 0.014143969908971668f, 0.005557952048201961f, -0.015026531518557016f, -0.028193147157592347f, -0.020329722202209837f, -0.004474405470829572f, 0.0f,    }
    },
    .frontmanIir1 = 
    { 
        .coeffA = {0.124986f, -0.316441f},
        .coeffB = {0.064673f, 0.066463f, 0.059869f},
        .w = {0.0f,0.0f,0.0f}
    },
    .frontmanIir2 = 
    { 
        .coeffA = {-0.037770f, -0.028280f},
        .coeffB = {1.843560f, 0.092694f, -0.494359f},
        .w = {0.0f,0.0f,0.0f}
    },
    .frontmanIir3 = 
    {
        .coeffA = {0.004979f, 0.009143f},
        .coeffB = {0.304015f, 0.298545f, -0.009865f},
        .w = {0.0f,0.0f,0.0f}
    },
    .voxAC15Fir = {
        .coefficients = {0.03724827994368069f, 0.10487875571625818f, 0.19783821411486727f, 0.28195177253714304f, 0.3413370776873385f, 0.3653993599309571f, 0.4103625115498769f, 0.4553054675683791f, 0.33702738937440824f, 0.0029509882053394706f, -0.26086336487061024f, -0.21191956788051644f, -0.015711469641577935f, 0.09705382917850859f, 0.06918403498857952f, -0.0011907047269019996f, -0.07113829939904402f, -0.09616862542497139f, -0.07552560187323017f, -0.05564687108959401f, -0.07013370099713576f, -0.07118355047843393f, 0.005591029166646203f, 0.05626643541927986f, -0.06826196069496776f, -0.15391867780357454f, -0.0754463693673927f, -0.03752152593725975f, -0.014165850451240523f, 0.021981045675779767f, 0.008613012718055877f, -0.032797751222710454f, -0.05379780163108525f, -0.06176359167453138f, -0.07481361932426078f, -0.07043137764431681f, -0.0325466728564425f, -0.004382381950247787f, -0.007172435860249302f, -0.023513682021527017f, -0.035691525149307256f, -0.040075127759971255f, -0.03251828110348452f, -0.010849677105036754f, 0.012664443224462786f, 0.019086787300007544f, 0.008050711723813908f, -0.0014427082194869761f, -0.014146170194037026f, -0.030694149726135975f, -0.001181251672402884f, 0.013092782758554708f, 0.0006804200956639303f, -0.004513977997822664f, -0.005584217044809224f, -0.008348754222064516f, -0.006997893271239425f, -0.0034011476730993603f, -0.004199716109528998f, -0.004435452229991048f, -0.0008175760048738743f, -0.0007665823434054438f, -0.0008846862290813553f, -0.0f,    }
        },
    .voxAC15Iir1 = {
        .coeffB = {2.621320f, -0.026606f, 0.030547f},
        .coeffA = {0.013020f, 0.006639f},
        .w = {0.0f,0.0f,0.0f}
    },
    .voxAC15Iir2 = {
        .coeffB = {2.970959f, 0.017054f, -0.079366f},
        .coeffA = {0.010293f, 0.028765f},
        .w = {0.0f,0.0f,0.0f}
    },
    .voxAC15Iir3 = {
        .coeffB = {0.012242f, 0.018960f, 0.010911f},
        .coeffA = {-0.021792f, -0.029699f},
        .w = {0.0f,0.0f,0.0f}
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
    .cabSimType = 1
};

FxProgramType fxProgram4 = {
    .name = "Amp Model 2          ",
    .nParameters=3,
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
        }  
    },
    .processSample = &fxProgram4processSample,
    .setup = &fxProgram4Setup,
    .data = (void*)&fxProgram4data
};

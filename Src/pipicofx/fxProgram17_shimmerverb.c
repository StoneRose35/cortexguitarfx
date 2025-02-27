#include "pipicofx/fxPrograms.h"
#include "audio/audiotools.h"
#include "stringFunctions.h"
#include "romfunc.h"

int16_t fxProgram17processSample(int16_t sampleIn,void*data)
{
    int16_t sampleProc = sampleIn;
    FxProgram17DataType * pData = (FxProgram17DataType*)data;
    volatile uint32_t * audioStatePtr = getAudioStatePtr();
    int32_t summedDelay=0;
    summedDelay += delayLineProcessSample(sampleProc,pData->delays);
    summedDelay = clip(summedDelay,audioStatePtr);
    summedDelay += delayLineProcessSample(sampleProc,pData->delays+1);
    summedDelay = clip(summedDelay,audioStatePtr);
    summedDelay += delayLineProcessSample(sampleProc,pData->delays+2);
    summedDelay = clip(summedDelay,audioStatePtr);
    summedDelay += delayLineProcessSample(sampleProc,pData->delays+3);
    summedDelay = clip(summedDelay,audioStatePtr);
    sampleProc = (int16_t)summedDelay;
    sampleProc = morphingAllpassProcessSample(sampleProc,pData->allpasses,(AudioProcessor)pitchShifterProcessSample,&pData->pitchShifter,audioStatePtr);
    sampleProc = allpassProcessSample(sampleProc,pData->allpasses+1,audioStatePtr);

    return ((((1 << 15) - pData->mix)*sampleIn) >> 15) + ((pData->mix*sampleProc) >> 15);
}

static void fxProgramParam1Callback(uint16_t val,void*data) // Shimmer
{
    FxProgram17DataType* pData= (FxProgram17DataType*)data;
    pData->pitchShifter.delayIncrement = (val >> 9) - 4;
    if (pData->pitchShifter.delayIncrement>=0)
    {
        pData->pitchShifter.delayIncrement+=1;
    }
    fxProgram17.parameters[0].rawValue = val;
}

static void fxProgramParam1Display(void*data,char*res)
{
    FxProgram17DataType* pData= (FxProgram17DataType*)data;
    *res=0;
    switch (pData->pitchShifter.delayIncrement)
    {
    case -4:
        appendToString(res,"OctUp");
        break;
    case -3:
        appendToString(res,"MoreUp");
        break;
    case -2:
        appendToString(res,"HalfUp");
        break;
    case -1:
        appendToString(res,"LittleUp");
        break;
    case 1:
        appendToString(res,"LittleDown");
        break;
    case 2:
        appendToString(res,"HalfDown");
        break;
    case 3:
        appendToString(res,"MoreDown");
        break;
    case 4:
        appendToString(res,"OctDown");
        break;
    default:
        appendToString(res,"Static");
        break;
    }
}

static void fxProgramParam2Callback(uint16_t val,void*data) // Decay
{
    FxProgram17DataType* pData= (FxProgram17DataType*)data;
    (pData->delays+0)->feedback = val << 3;
    (pData->delays+1)->feedback = val << 3;
    (pData->delays+2)->feedback = val << 3;
    (pData->delays+3)->feedback = val << 3;
    fxProgram17.parameters[1].rawValue = val;
}

static void fxProgramParam2Display(void*data,char*res)
{
    float ffbk;
    int16_t t60;
    FxProgram17DataType* pData= (FxProgram17DataType*)data;
    ffbk = int2float(pData->delays->feedback)/32767.0f;
    if (ffbk < 0.0000305)
    {
        t60=0;
    }
    else
    {
        t60=(int16_t)float2int(-546.86396f/fln(ffbk)); // t60 in ms, as ln(0.001)/samplingFreq*delayInSamples*1000/ln(ffbk)
    }
    Int16ToChar(t60,res);
    appendToString(res," ms");
}


static void fxProgramParam3Callback(uint16_t val,void*data) // Mix
{
    FxProgram17DataType* pData= (FxProgram17DataType*)data;
    pData->mix=(val << 3);
    fxProgram17.parameters[1].rawValue = val;
}

static void fxProgramParam3Display(void*data,char*res)
{
    FxProgram17DataType* pData= (FxProgram17DataType*)data;
    int16_t mixpercent = (int16_t)(pData->mix/328);
    Int16ToChar(mixpercent,res);
    appendToString(res,"%");

}

FxProgram17DataType fxProgram17data=
{
    .pitchShifter.currentDelayPosition=0,
    .pitchShifter.delayIncrement=-2, // half up according to program 16
    .pitchShifter.buffersizePowerTwo = 11,  // longest buffer size in program 16
    .mix=0
};

void fxProgram17Setup(void*data)
{
    int16_t * delayMemoryPointer = getDelayMemoryPointer();
    FxProgram17DataType * pData = (FxProgram17DataType*)data;
    initPitchshifter(&pData->pitchShifter);

    initDelay(pData->delays,delayMemoryPointer+2048,4096);
    pData->delays[0].delayInSamples = 3943;
    initDelay(pData->delays+1,delayMemoryPointer+4096+2048,4096);
    pData->delays[1].delayInSamples = 3617;
    initDelay(pData->delays+2,delayMemoryPointer+2*4096+2048,4096);
    pData->delays[2].delayInSamples = 3943;
    initDelay(pData->delays+3,delayMemoryPointer+3*4096+2048,4096);
    pData->delays[3].delayInSamples = 3823;

    pData->allpasses[0].delayLineIn = delayMemoryPointer + 4*4096+2048;
    pData->allpasses[0].delayLineOut = delayMemoryPointer + 4*4096+ 1024+2048;
    pData->allpasses[0].coefficient = 16383;
    pData->allpasses[0].delayPtr = 0;
    pData->allpasses[0].oldValues = 0;
    pData->allpasses[0].delayInSamples=617;
    pData->allpasses[0].bufferSize = 0x3FF;

    pData->allpasses[1].delayLineIn = delayMemoryPointer + 4*4096+2*1024+2048;
    pData->allpasses[1].delayLineOut = delayMemoryPointer + 4*4096+ 3*1024+2048;
    pData->allpasses[1].coefficient = 16383;
    pData->allpasses[1].delayPtr = 0;
    pData->allpasses[1].oldValues = 0;
    pData->allpasses[1].delayInSamples=617;
    pData->allpasses[1].bufferSize = 0x3FF;
}

FxProgramType fxProgram17 = {
    .name = "ShimmerVerb",
    .nParameters=3,
    .parameters = {
        {
            .name="Shimmer",
            .control=0,
            .increment=512,
            .rawValue=0,
            .getParameterDisplay=&fxProgramParam1Display,
            .getParameterValue=0,
            .setParameter=&fxProgramParam1Callback
        },
        {
            .name="Decay",
            .control=1,
            .increment=16,
            .rawValue=0,
            .getParameterDisplay=&fxProgramParam2Display,
            .getParameterValue=0,
            .setParameter=&fxProgramParam2Callback
        },
        {
            .name="Mix",
            .control=2,
            .increment=16,
            .rawValue=0,
            .getParameterDisplay=&fxProgramParam3Display,
            .getParameterValue=0,
            .setParameter=&fxProgramParam3Callback
        }
    },
    .processSample = &fxProgram17processSample,
    .setup = &fxProgram17Setup,
    .reset = 0,
    .data = (void*)&fxProgram17data
};

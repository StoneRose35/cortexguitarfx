#include "pipicofx/fxPrograms.h"
#include "audio/audiotools.h"
#include "stringFunctions.h"
#include "romfunc.h"

int16_t fxProgram17processSample(int16_t sampleIn,void*data)
{
    
    FxProgram17DataType * pData = (FxProgram17DataType*)data;
    int32_t sampleProc = sampleIn;// + ((pData->feedback*pData->oldVal) >> 15);
    volatile uint32_t * audioStatePtr = getAudioStatePtr();
    int32_t summedDelay=0;
    
    summedDelay += delayLineProcessSample(sampleProc,pData->delays);
    summedDelay += delayLineProcessSample(sampleProc,pData->delays+1);
    summedDelay += delayLineProcessSample(sampleProc,pData->delays+2);
    summedDelay = clip(summedDelay,audioStatePtr);
    sampleProc = summedDelay;
    sampleProc = allpassProcessSample(sampleProc,pData->allpasses,audioStatePtr);
    sampleProc = allpassProcessSample(sampleProc,pData->allpasses+1,audioStatePtr);
    sampleProc = delayLineProcessSample(sampleProc,pData->delays+3);
    pData->oldVal = sampleProc;
    return ((((1 << 15) - pData->mix)*sampleIn) >> 15) + ((pData->mix*sampleProc) >> 15);
}

static void fxProgramParam1Callback(uint16_t val,void*data) // Shimmer
{
    FxProgram17DataType* pData= (FxProgram17DataType*)data;
    pData->pitchShifter.delayIncrement = (val >> 9) + 1;
    fxProgram17.parameters[0].rawValue = val;
}

static void fxProgramParam1Display(void*data,char*res)
{
    FxProgram17DataType* pData= (FxProgram17DataType*)data;
    *res=0;
    switch (pData->pitchShifter.delayIncrement)
    {
        case 1:
        appendToString(res,"2OctDown");
        break;
    case 2:
        appendToString(res,"OctDown");
        break;
    case 3:
        appendToString(res,"FourthDown");
        break;
    case 4:
        appendToString(res,"NoShift");
        break;
    case 5:
        appendToString(res,"ThirdUp");
        break;
    case 6:
        appendToString(res,"FifthUp");
        break;
    case 7:
        appendToString(res,"Devil666");
        break;
    case 8:
        appendToString(res,"OctUp");
        break;
    default:
        appendToString(res,"ERROR");
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
    pData->feedback = val << 3;
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
    .pitchShifter.delayIncrement=8,
    .pitchShifter.buffersizePowerTwo = 12,  // longest buffer size in program 16
    .pitchShifter.crossFadeWidthPwr2=10,
    .glitterTamer.alpha = 20000,
    .glitterTamer.oldVal = 0,
    .glitterTamer.oldXVal = 0,
    .mix=0,
    .oldVal = 0,
    .feedback = 0
};

int16_t unicornGlitter(int16_t sampleIn,Pitchshifter2DataType*data,volatile uint32_t * audioState)
{
    FxProgram17DataType* pData= (FxProgram17DataType*)data;
    sampleIn = pitchShifter2ProcessSample(sampleIn,&pData->pitchShifter,audioState);
    return firstOrderIirLowpassProcessSample(sampleIn,&pData->glitterTamer);
}

void fxProgram17Setup(void*data)
{
    int16_t * delayMemoryPointer = getDelayMemoryPointer();
    FxProgram17DataType * pData = (FxProgram17DataType*)data;
    initPitchshifter2(&pData->pitchShifter);

    initDelay(pData->delays,delayMemoryPointer+4096,256);
    pData->delays[0].delayInSamples = 149;
    pData->delays[0].mix = ((1 << 15) -1) ;
    initDelay(pData->delays+1,delayMemoryPointer+256+4096,512);
    pData->delays[1].delayInSamples = 337;
    pData->delays[1].mix = ((1 << 15) -1) ;
    initDelay(pData->delays+2,delayMemoryPointer+512+256+4096,2048);
    pData->delays[2].delayInSamples = 1597;
    pData->delays[2].mix = ((1 << 15) -1) ;
    initDelay(pData->delays+3,delayMemoryPointer+2048+512+256+4096,4096);
    pData->delays[3].delayInSamples = 3989;
    pData->delays[3].mix = ((1 << 15) -1) ;
    pData->delays[3].feedbackFunction = (AudioProcessor)unicornGlitter;
    pData->delays[3].feebackData = pData;

    pData->allpasses[0].delayLineIn = delayMemoryPointer + 4096+2048+512+256+4096;
    pData->allpasses[0].delayLineOut = delayMemoryPointer + 1024+4096+2048+512+256+4096;
    pData->allpasses[0].coefficient = 22936;
    pData->allpasses[0].delayPtr = 0;
    pData->allpasses[0].oldValues = 0;
    pData->allpasses[0].delayInSamples=617;
    pData->allpasses[0].bufferSize = 0x3FF;

    pData->allpasses[1].delayLineIn = delayMemoryPointer + 1024+1024+4096+2048+512+256+4096;
    pData->allpasses[1].delayLineOut = delayMemoryPointer + 1024+1024+1024+4096+2048+512+256+4096;
    pData->allpasses[1].coefficient = 22936;
    pData->allpasses[1].delayPtr = 0;
    pData->allpasses[1].oldValues = 0;
    pData->allpasses[1].delayInSamples=907;
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

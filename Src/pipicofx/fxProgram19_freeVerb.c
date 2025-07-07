#include "pipicofx/fxPrograms.h"
#include "stringFunctions.h"
#include "audio/delay.h"
#include "audio/reverbUtils.h"
#include "romfunc.h"

// implementation of the freeverb see https://ccrma.stanford.edu/~jos/pasp/Freeverb.html for reference


int16_t fxProgram19processSample(int16_t sampleIn,void*data)
{
    int32_t sampleOut;
    FxProgram19DataType* pData = (FxProgram19DataType*)data;
    volatile uint32_t * audioStatePtr = getAudioStatePtr();
    int32_t delaySum=0;
    for (uint8_t c=0;c<8;c++)
    {
        delaySum += delayLineWetProcessSample(sampleIn,pData->delays+c);
    }
    sampleOut = delaySum >> 3;
    for (uint8_t c=0;c<4;c++)
    {
        sampleOut = allpassProcessSample(sampleOut,pData->allpasses+c,audioStatePtr);
    }
    sampleOut = clip(((((1 << 15) - pData->mix)*sampleIn) >> 15) + ((pData->mix*sampleOut) >> 15),audioStatePtr);
    return gainStageProcessSample(sampleOut,&pData->presetVolume);
}

int16_t freeVerbLowpass(int16_t sampleIn,void * filterData,volatile uint32_t * audioStatePtr)
{
    return firstOrderIirLowpassProcessSample(sampleIn,(FirstOrderIirType*)filterData);
}

static void fxProgramParameter1Callback(uint16_t val,void*data) // Size
{
    FxProgram19DataType* pData = (FxProgram19DataType*)data;
    int32_t feedback = (((val << 3)*9175)>>15) + 22938;// val*0.28+0.7;
    for (uint8_t c=0;c<8;c++)
    {
        (pData->delays+c)->feedback = (int16_t)feedback;
    }
    fxProgram19.parameters[0].rawValue=val;
}

static void fxProgramParameter1Display(void*data,char*res)
{
    FxProgram19DataType* pData = (FxProgram19DataType*)data;
    float ffbk;
    int16_t t60;
    ffbk = int2float(pData->delays[0].feedback)/32767.0f;
    if (ffbk < 0.0000305)
    {
        t60=0;
    }
    else
    {
        t60=(int16_t)float2int(-589.03004f/fln(ffbk)); // t60 in ms
    }
    Int16ToChar(t60,res);
    appendToString(res," ms");
    
}

static void fxProgramParameter2Callback(uint16_t val,void*data)
{
    FxProgram19DataType* pData = (FxProgram19DataType*)data;
        for (uint8_t c=0;c<8;c++)
    {
        (pData->feedbackFilters+c)->alpha = 32767 - (val << 3);
    }
    fxProgram19.parameters[1].rawValue=val;
}

static void fxProgramParameter2Display(void*data,char*res)
{
    FxProgram19DataType* pData= (FxProgram19DataType*)data;
    int16_t damping = pData->feedbackFilters[0].alpha;
    Int16ToChar(damping/328,res);
    appendToString(res,"%");
}

static void fxProgramParam3Callback(uint16_t val,void*data) // Mix
{
    FxProgram19DataType* pData= (FxProgram19DataType*)data;
    pData->mix=(val << 3);
    fxProgram19.parameters[2].rawValue = val;
}

static void fxProgramParam3Display(void*data,char*res)
{
    FxProgram19DataType* pData= (FxProgram19DataType*)data;
    int16_t mixpercent = (int16_t)(pData->mix/328);
    Int16ToChar(mixpercent,res);
    appendToString(res,"%");

}

static void fxProgramPresetVolumeCallback(uint16_t val,void*data)
{
    FxProgram19DataType* pData = (FxProgram19DataType*)data;
    pData->presetVolume.gain = val >> 2; // 0 to 1024
    fxProgram18.parameters[4].rawValue=val;
}

static void fxProgramPresetVolumeDisplay(void*data,char*res)
{
    FxProgram19DataType* pData = (FxProgram19DataType*)data;
    int16_t dVal;
    dVal = pData->presetVolume.gain*39; // percent with two decimal points
    decimalInt16ToChar(dVal,res,2);
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

FxProgram19DataType fxProgram19data={
    .delays = {
        /* for 44100 Hz sampling rate
        {.delayInSamples = 1557,.feedbackFunction=freeVerbLowpass,.delayBufferLength=2048},
        {.delayInSamples = 1617,.feedbackFunction=freeVerbLowpass,.delayBufferLength=2048},
        {.delayInSamples = 1491,.feedbackFunction=freeVerbLowpass,.delayBufferLength=2048},
        {.delayInSamples = 1422,.feedbackFunction=freeVerbLowpass,.delayBufferLength=2048},
        {.delayInSamples = 1277,.feedbackFunction=freeVerbLowpass,.delayBufferLength=2048},
        {.delayInSamples = 1356,.feedbackFunction=freeVerbLowpass,.delayBufferLength=2048},
        {.delayInSamples = 1188,.feedbackFunction=freeVerbLowpass,.delayBufferLength=2048},
        {.delayInSamples = 1116,.feedbackFunction=freeVerbLowpass,.delayBufferLength=2048}*/
        // for 48kHz sampling rate
        {.delayInSamples = 1695,.feedbackFunction=freeVerbLowpass,.delayBufferLength=2048},
        {.delayInSamples = 1760,.feedbackFunction=freeVerbLowpass,.delayBufferLength=2048},
        {.delayInSamples = 1623,.feedbackFunction=freeVerbLowpass,.delayBufferLength=2048},
        {.delayInSamples = 1548,.feedbackFunction=freeVerbLowpass,.delayBufferLength=2048},
        {.delayInSamples = 1390,.feedbackFunction=freeVerbLowpass,.delayBufferLength=2048},
        {.delayInSamples = 1476,.feedbackFunction=freeVerbLowpass,.delayBufferLength=2048},
        {.delayInSamples = 1293,.feedbackFunction=freeVerbLowpass,.delayBufferLength=2048},
        {.delayInSamples = 1215,.feedbackFunction=freeVerbLowpass,.delayBufferLength=2048}
    },.allpasses = {
        /* for 44100 
        {.delayInSamples=225,
            .coefficient=16384,.bufferSize=1023},
        {.delayInSamples = 556,
            .coefficient=16384,.bufferSize=1023},
        {.delayInSamples = 441,
            .coefficient=16384,.bufferSize=1023},
        {.delayInSamples = 341,
            .coefficient=16384,.bufferSize=1023}*/
        // for 48 kHz sampling rate
        {.delayInSamples=245,
            .coefficient=16384,.bufferSize=1023},
        {.delayInSamples = 605,
            .coefficient=16384,.bufferSize=1023},
        {.delayInSamples = 480,
            .coefficient=16384,.bufferSize=1023},
        {.delayInSamples = 371,
            .coefficient=16384,.bufferSize=1023}
    },
    .feedbackFilters = {
        {
            .alpha = 6554,.oldVal=0,.oldXVal=0
        },
        {
            .alpha = 6554,.oldVal=0,.oldXVal=0
        },
        {
            .alpha = 6554,.oldVal=0,.oldXVal=0
        },
        {
            .alpha = 6554,.oldVal=0,.oldXVal=0
        },
        {
            .alpha = 6554,.oldVal=0,.oldXVal=0
        },
        {
            .alpha = 6554,.oldVal=0,.oldXVal=0
        },
        {
            .alpha = 6554,.oldVal=0,.oldXVal=0
        },
        {
            .alpha = 6554,.oldVal=0,.oldXVal=0
        }
    },
    .presetVolume = {
        .gain =0xff,
        .offset = 0
    },
    .mix = 0
};

void fxProgram19Setup(void*data)
{
    int16_t* delayMemPtr = getDelayMemoryPointer();
    FxProgram19DataType * pData = (FxProgram19DataType*)data;
    clearDelayLine();
    for (uint8_t c=0;c<8;c++)
    {
        initDelay(pData->delays+c,delayMemPtr+c*2048,2048);
        fxProgram19data.delays[c].feebackData = (void*)(fxProgram19data.feedbackFilters+c);
    }
    for (uint8_t c=0;c<4;c++)
    {
        fxProgram19data.allpasses[c].delayLineIn=delayMemPtr+8*2048+c*2048;
        fxProgram19data.allpasses[c].delayLineOut=delayMemPtr+8*2048+c*2048+1024;
    }

}

FxProgramType fxProgram19 = {
    .name = "FreeVerb",
    .nParameters=4,
    .parameters={
        {
            .name="Decay",
            .control=0,
            .increment=1,
            .rawValue=0,
            .setParameter=&fxProgramParameter1Callback,
            .getParameterValue=0,
            .getParameterDisplay=&fxProgramParameter1Display,
        },
        {
            .name="Damping",
            .control=1,
            .increment=1,
            .rawValue=0,
            .setParameter=&fxProgramParameter2Callback,
            .getParameterValue=0,
            .getParameterDisplay=&fxProgramParameter2Display,
        },
        {
            .name = "Mix            ",
            .control=2,
            .increment=1,
            .rawValue=0,
            .getParameterDisplay=&fxProgramParam3Display,
            .getParameterValue=0,
            .setParameter=&fxProgramParam3Callback
        },
        {
            .name="Volume",
            .control=0xff,
            .increment=1,
            .rawValue=0x3ff,
            .setParameter=&fxProgramPresetVolumeCallback,
            .getParameterValue=0,
            .getParameterDisplay=&fxProgramPresetVolumeDisplay
        }
    },
    .processSample = &fxProgram19processSample,
    .setup = &fxProgram19Setup,
    .reset = 0,
    .data = (void*)&fxProgram19data
};
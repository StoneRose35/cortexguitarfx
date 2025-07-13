
#include "pipicofx/fxPrograms.h"
#include "stringFunctions.h"

static int16_t fxProgramprocessSample(int16_t sampleIn,void*data)
{
    FxProgram10DataType* pData= (FxProgram10DataType*)data;
    sampleIn = gainStageProcessSample(sampleIn,&pData->presetVolume);
    return reverbProcessSample(sampleIn,&pData->reverb);
}

static void fxProgramParam1Callback(uint16_t val,void*data) // reverb time
{
    FxProgram10DataType* pData= (FxProgram10DataType*)data;
    pData->reverbTime = (((uint32_t)val*1900)>>12) + 100;
    setReverbTime(pData->reverbTime,&pData->reverb);
    fxProgram10.parameters[0].rawValue = val;
}

static void fxProgramParam1Display(void*data,char*res)
{
    FxProgram10DataType* pData= (FxProgram10DataType*)data;
    Int16ToChar(pData->reverbTime,res);
    appendToString(res," ms");
}

static void fxProgramParam2Callback(uint16_t val,void*data) // Mix
{
    FxProgram10DataType* pData= (FxProgram10DataType*)data;
    pData->reverb.mix=(val << 3);
    fxProgram10.parameters[1].rawValue = val;
}

static void fxProgramParam2Display(void*data,char*res)
{
    FxProgram10DataType* pData= (FxProgram10DataType*)data;
    int16_t mixpercent = (int16_t)(pData->reverb.mix/328);
    Int16ToChar(mixpercent,res);
    appendToString(res,"%");

}

static void fxProgramParam3Callback(uint16_t val,void*data) // Parameter
{
    FxProgram10DataType* pData= (FxProgram10DataType*)data;
    pData->reverb.paramNr=(val >> 10);
    initReverb(&pData->reverb,pData->reverbTime);
    fxProgram10.parameters[2].rawValue = val;
}

static void fxProgramParam3Display(void*data,char*res)
{
    FxProgram10DataType* pData= (FxProgram10DataType*)data;
    *res=0;
    appendToString(res,getReverbParameterSetName(&pData->reverb));
}


static void fxProgramPresetVolumeCallback(uint16_t val,void*data)
{
    FxProgram10DataType* pData = (FxProgram10DataType*)data;
    pData->presetVolume.gain = val >> 2; // 0 to 1024
    fxProgram10.parameters[3].rawValue=val;
}

static void fxProgramPresetVolumeDisplay(void*data,char*res)
{
    FxProgram10DataType* pData = (FxProgram10DataType*)data;
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

static void fxProgramSetup(void*data)
{
    FxProgram10DataType* pData= (FxProgram10DataType*)data;
    initReverb(&pData->reverb,pData->reverbTime);
}

FxProgram10DataType fxProgram10data=
{
    .reverbTime=300,
    .reverb.paramNr=0,
    .presetVolume = {
        .gain = 0xff,
        .offset = 0
    }
};

FxProgramType fxProgram10 = {
    .name = "Reverb",
    .nParameters=4,
    .parameters = {
        {
            .name = "Time           ",
            .control=0,
            .increment=1,
            .rawValue=0,
            .getParameterDisplay=&fxProgramParam1Display,
            .getParameterValue=0,
            .setParameter=&fxProgramParam1Callback
        },
        {
            .name = "Mix            ",
            .control=1,
            .increment=1,
            .rawValue=0,
            .getParameterDisplay=&fxProgramParam2Display,
            .getParameterValue=0,
            .setParameter=&fxProgramParam2Callback
        },
        {
            .name = "ParameterSet   ",
            .control=2,
            .increment=1024,
            .rawValue=0,
            .getParameterDisplay=&fxProgramParam3Display,
            .getParameterValue=0,
            .setParameter=&fxProgramParam3Callback
        },
        {
            .name="Volume",
            .control=0xff,
            .increment=1,
            .rawValue=0x3FF,
            .setParameter=fxProgramPresetVolumeCallback,
            .getParameterValue=0,
            .getParameterDisplay=fxProgramPresetVolumeDisplay
        }
    },
    .processSample = &fxProgramprocessSample,
    .setup = &fxProgramSetup,
    .reset = 0,
    .data = (void*)&fxProgram10data
};
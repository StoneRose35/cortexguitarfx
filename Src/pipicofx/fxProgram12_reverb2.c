
#include "pipicofx/fxPrograms.h"
#include "stringFunctions.h"

static int16_t fxProgramprocessSample(int16_t sampleIn,void*data)
{
    FxProgram12DataType* pData= (FxProgram12DataType*)data;
    sampleIn = gainStageProcessSample(sampleIn,&pData->presetVolume);
    return reverb2ProcessSample(sampleIn,&pData->reverb);
}

static void fxProgramParam1Callback(uint16_t val,void*data) // reverb time
{
    FxProgram12DataType* pData= (FxProgram12DataType*)data;
    pData->reverb.decay = val << 3;
    fxProgram12.parameters[0].rawValue = val;
}

static void fxProgramParam1Display(void*data,char*res)
{
    FxProgram12DataType* pData= (FxProgram12DataType*)data;
    Int16ToChar(pData->reverb.decay, res);
}

static void fxProgramParam2Callback(uint16_t val,void*data) // Mix
{
    FxProgram12DataType* pData= (FxProgram12DataType*)data;
    pData->reverb.mix=(val << 3);
    fxProgram12.parameters[1].rawValue = val;
}

static void fxProgramParam2Display(void*data,char*res)
{
    FxProgram12DataType* pData= (FxProgram12DataType*)data;
    int16_t mixpercent = (int16_t)(pData->reverb.mix/328);
    Int16ToChar(mixpercent,res);
    appendToString(res,"%");

}


static void fxProgramPresetVolumeCallback(uint16_t val,void*data)
{
    FxProgram12DataType* pData = (FxProgram12DataType*)data;
    pData->presetVolume.gain = val >> 2; // 0 to 1024
    fxProgram12.parameters[2].rawValue=val;
}

static void fxProgramPresetVolumeDisplay(void*data,char*res)
{
    FxProgram12DataType* pData = (FxProgram12DataType*)data;
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
    FxProgram12DataType* pData= (FxProgram12DataType*)data;
    initReverb2(&pData->reverb);
}

FxProgram12DataType fxProgram12data= {
    .presetVolume = {
        .gain =0xff,
        .offset=0
    }
};

FxProgramType fxProgram12 = {
    .name = "Allpass Reverb",
    .nParameters=3,
    .parameters = {
        {
            .name = "Decay          ",
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
    .data = (void*)&fxProgram12data
};

#include "pipicofx/fxPrograms.h"
#include "stringFunctions.h"

int16_t fxProgram3processSample(int16_t sampleIn,void*data)
{
    FxProgram3DataType* pData = (FxProgram3DataType*)data;
    return gainStageProcessSample(sampleIn,&pData->presetVolume);
}

static void fxProgramPresetVolumeCallback(uint16_t val,void*data)
{
    FxProgram3DataType* pData = (FxProgram3DataType*)data;
    pData->presetVolume.gain = val >> 2; // 0 to 1024
    fxProgram3.parameters[0].rawValue=val;
}

static void fxProgramPresetVolumeDisplay(void*data,char*res)
{
    FxProgram3DataType* pData = (FxProgram3DataType*)data;
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

void fxProgram3Setup(void*data)
{}

FxProgram3DataType fxProgram3data = {
    .presetVolume = {
        .gain = 0x100,
        .offset = 0
    }
};

FxProgramType fxProgram3 = {
    .name = "Off",
    .nParameters=1,
    .processSample = &fxProgram3processSample,
    .setup = &fxProgram3Setup,
    .reset = 0,
    .data = (void*)&fxProgram3data,
    .parameters = {
        {
            .name="Volume",
            .control=0x0,
            .increment=1,
            .rawValue=0x3FF,
            .setParameter=fxProgramPresetVolumeCallback,
            .getParameterValue=0,
            .getParameterDisplay=fxProgramPresetVolumeDisplay
        }
    }
};

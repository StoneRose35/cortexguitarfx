#include <stdint.h>
#include "pipicofx/fxPrograms.h"
#include "stringFunctions.h"
#include "audio/gainstage.h"

static int16_t fxProgram2processSample(int16_t sampleIn,void*data)
{
    FxProgram2DataType* pData = (FxProgram2DataType*)data;
    sampleIn >>= 1;
    sampleIn = simpleChorusProcessSample(sampleIn,&pData->chorusData);
    return gainStageProcessSample(sampleIn,&pData->presetVolume);
}

static void fxProgram2Param1Callback(uint16_t val,void*data) // frequency
{
    FxProgram2DataType* pData = (FxProgram2DataType*)data;
    // map 0 - 4095 to 1 1000
    val = ((val*250) >> 10) + 1;
    simpleChorusSetFrequency(val,&pData->chorusData);
    fxProgram2.parameters[0].rawValue = val;
}

static void fxProgram2Param1Display(void*data,char*res)
{
    FxProgram2DataType* pData = (FxProgram2DataType*)data;
    decimalInt16ToChar(pData->chorusData.frequency,res,2);
    appendToString(res," Hz");
}

static void fxProgram2Param2Callback(uint16_t val,void*data) // depth
{
    FxProgram2DataType* pData = (FxProgram2DataType*)data;
    // map to 0 to 255
    val >>= 4;
    pData->chorusData.depth = (uint8_t)val;
    fxProgram2.parameters[1].rawValue = val;
}

static void fxProgram2Param2Display(void*data,char*res)
{
    int16_t dVal;
    FxProgram2DataType* fData=(FxProgram2DataType*)data;
    dVal = (fData->chorusData.depth*100) >> 8;
    Int16ToChar(dVal,res);
    appendToString(res,"%");
}

static void fxProgram2Param3Callback(uint16_t val,void*data) // mix
{
    FxProgram2DataType* pData = (FxProgram2DataType*)data;
    // map to 0 to 255
    val >>= 4;
    pData->chorusData.mix = (uint8_t)val;
    fxProgram2.parameters[2].rawValue = val;
}

static void fxProgram2Param3Display(void*data,char*res)
{
    int16_t dVal;
    FxProgram2DataType* fData = (FxProgram2DataType*)data;
    dVal = (fData->chorusData.mix*100) >> 8;
    Int16ToChar(dVal,res);
    appendToString(res,"%");
}

static void fxProgramPresetVolumeCallback(uint16_t val,void*data)
{
    FxProgram2DataType* pData = (FxProgram2DataType*)data;
    pData->presetVolume.gain = val >> 2; // 0 to 1024
    fxProgram2.parameters[3].rawValue=val;
}

static void fxProgramPresetVolumeDisplay(void*data,char*res)
{
    FxProgram2DataType* pData = (FxProgram2DataType*)data;
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


static void fxProgram2Setup(void*data)
{
    FxProgram2DataType* pData = (FxProgram2DataType*)data;
    initSimpleChorus(&pData->chorusData);
}

FxProgram2DataType fxProgram2data = {
    .chorusData = {
        .mix = 128,
        .frequency = 500,
        .depth = 10
    },
    .presetVolume = {
        .gain=0xFF,
        .offset=0
    }
    
};

FxProgramType fxProgram2 = {
    .name = "Vibrato/Chorus",
    .nParameters=4,
    .processSample = &fxProgram2processSample,
    .parameters = {
        {
            .name = "Frequency      ",
            .control=0,
            .increment = 1,
            .rawValue=0,
            .getParameterDisplay=&fxProgram2Param1Display,
            .getParameterValue=0,
            .setParameter=&fxProgram2Param1Callback
        },
        {
            .name = "Depth          ",
            .control=1,
            .increment = 1,
            .rawValue=0,
            .getParameterDisplay=&fxProgram2Param2Display,
            .getParameterValue=0,
            .setParameter=&fxProgram2Param2Callback
        },
        {
            .name = "Mix            ",
            .control=2,
            .increment=1,
            .rawValue=0,
            .getParameterDisplay=&fxProgram2Param3Display,
            .getParameterValue=0,
            .setParameter=&fxProgram2Param3Callback
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
    .setup = &fxProgram2Setup,
    .reset = 0,
    .data = (void*)&fxProgram2data
};

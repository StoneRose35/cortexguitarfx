#include "pipicofx/fxPrograms.h"
#include "stringFunctions.h"

static int16_t fxProgram5processSample(int16_t sampleIn,void*data)
{
    FxProgram5DataType* pData= (FxProgram5DataType*)data;
    sampleIn = bitCrusherProcessSample(sampleIn,&pData->bitcrusher);
    sampleIn = gainStageProcessSample(sampleIn,&pData->presetVolume);
    return sampleIn;
}

static void fxProgram5Param1Callback(uint16_t val,void*data) // set bit mask
{
    FxProgram5DataType* pData= (FxProgram5DataType*)data;
    uint16_t resolution;
    resolution = (4096 - val) >> 8;
    setBitMask((uint8_t)resolution,&pData->bitcrusher);
    fxProgram5.parameters[0].rawValue = val;
}

static void fxProgram5Param1Display(void*data,char*res)
{
    //FxProgram5DataType *  fData = (FxProgram5DataType*)data;
    uint8_t nbits = (4096-fxProgram5.parameters[0].rawValue) >> 8;
    UInt8ToChar(nbits,res);
    appendToString(res,"-bits");
}

static void fxProgramPresetVolumeCallback(uint16_t val,void*data)
{
    FxProgram5DataType* pData = (FxProgram5DataType*)data;
    pData->presetVolume.gain = val >> 2; // 0 to 1024
    fxProgram5.parameters[1].rawValue=val;
}

static void fxProgramPresetVolumeDisplay(void*data,char*res)
{
    FxProgram5DataType* pData = (FxProgram5DataType*)data;
    uint16_t dVal;
    dVal = pData->presetVolume.gain*39; // percent with two decimal points
    decimalUInt16ToChar(dVal,res,2);
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

void fxProgram5Setup(void*data)
{

}

FxProgram5DataType fxProgram5data = {
    .bitcrusher = {
        .bitmask = 0x8000
    },
    .presetVolume = {
        .gain=0xFF,
        .offset=0
    }
};

FxProgramType fxProgram5 = {
    .name = "Monstercrusher",
    .nParameters=2,
    .parameters = {
        {
            .name = "Resolution",
            .control=0,
            .increment=256,
            .rawValue=0,
            .getParameterDisplay=&fxProgram5Param1Display,
            .getParameterValue=0,
            .setParameter=&fxProgram5Param1Callback
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
    .processSample = &fxProgram5processSample,
    .setup = &fxProgram5Setup,
    .reset = 0,
    .data = (void*)&fxProgram5data
};
#include "audio/fxprogram/fxProgram.h"
#include "stringFunctions.h"

static int16_t fxProgram5processSample(int16_t sampleIn,void*data)
{
    FxProgram5DataType* pData= (FxProgram5DataType*)data;
    return bitCrusherProcessSample(sampleIn,&pData->bitcrusher);
}

static void fxProgram5Param1Callback(uint16_t val,void*data) // set bit mask
{
    FxProgram5DataType* pData= (FxProgram5DataType*)data;
    uint16_t resolution;
    resolution = (4096 - val) >> 8;
    setBitMask((uint8_t)resolution,&pData->bitcrusher);
}

static void fxProgram5Param1Display(void*data,char*res)
{
    uint16_t resolution;
    FxProgram5DataType *  fData = (FxProgram5DataType*)data;
    
    resolution = fData->bitcrusher.bitmask;
    UInt16ToChar(resolution,res);
}


void fxProgram5Setup(void*data)
{

}

FxProgram5DataType fxProgram5data = {
    .bitcrusher = {
        .bitmask = 0x8000
    }
};

FxProgramType fxProgram5 = {
    .name = "Monstercrusher       ",
    .nParameters=1,
    .parameters = {
        {
            .name = "Bit Reduction  ",
            .control=0,
            .increment=64,
            .rawValue=0,
            .getParameterDisplay=&fxProgram5Param1Display,
            .getParameterValue=0,
            .setParameter=&fxProgram5Param1Callback
        }
    },
    .processSample = &fxProgram5processSample,
    .setup = &fxProgram5Setup,
    .data = (void*)&fxProgram5data
};
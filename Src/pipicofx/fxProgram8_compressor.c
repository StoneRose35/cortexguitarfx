#include <stdint.h>
#include "pipicofx/fxPrograms.h"
#include "stringFunctions.h"
#include "ln.h"

__attribute__ ((section (".qspi_code")))
static float fxProgramProcessSample(float sampleIn,void*data)
{
    FxProgram8DataType * pData=(FxProgram8DataType*)data;
    switch (pData->compressorType)
    {
        case 0:
            sampleIn = compressorProcessSample(sampleIn,&pData->compressor);
            break;
        case 1:
            sampleIn = compressor2ProcessSample(sampleIn,&pData->compressor);
            break;
        case 2:
            sampleIn = compressor3ProcessSample(sampleIn,&pData->compressor);
            break;
    }
    sampleIn = gainStageProcessSample(sampleIn,&pData->makeupGain);
    return sampleIn;
}

__attribute__ ((section (".qspi_code")))
static void fxProgramP1Callback(uint16_t val,void*data) 
{
    FxProgram8DataType * pData=(FxProgram8DataType*)data;
    pData->compressor.avgLowpass.alphaRising = 1.0f - 2.0f/32768.0f -val/64.0f/32768.0;
    fxProgram8.parameters[0].rawValue = val;
}

__attribute__ ((section (".qspi_code")))
static void fxProgramP1Display(void*data,char*res)
{
    FxProgram8DataType * pData=(FxProgram8DataType*)data;
    float attackFloat;
    float t60;
    int32_t ival;
    int16_t i16val;
    attackFloat = pData->compressor.avgLowpass.alphaRising;
    t60 = -0.143911568f/fln(attackFloat);

    ival = (int)t60;
    i16val = (int16_t)ival;
    Int16ToChar(i16val,res);
    appendToString(res, " ms");
}

__attribute__ ((section (".qspi_code")))
static void fxProgramP2Callback(uint16_t val,void*data) 
{
    FxProgram8DataType * pData=(FxProgram8DataType*)data;
    pData->compressor.avgLowpass.alphaFalling = 1.0f - 2.0f/32768.0f - val/64.0f/32768.0f;
    fxProgram8.parameters[1].rawValue = val;
}

__attribute__ ((section (".qspi_code")))
static void fxProgramP2Display(void*data,char*res)
{
    FxProgram8DataType * pData=(FxProgram8DataType*)data;
    float attackFloat;
    float t60;
    int32_t ival;
    int16_t i16val;
    attackFloat = pData->compressor.avgLowpass.alphaFalling;
    t60 = -0.143911568f/fln(attackFloat);

    ival = (int)t60;
    i16val = (int16_t)ival;
    Int16ToChar(i16val,res);
    appendToString(res, " ms");
}

__attribute__ ((section (".qspi_code")))
static void fxProgramP3Callback(uint16_t val,void*data) 
{
    FxProgram8DataType * pData=(FxProgram8DataType*)data;
    uint16_t enumVal = (val >> 9) + 1;
    if (enumVal > 5)
    {
        enumVal=5;
    }
    fxProgram8.parameters[2].rawValue = val;
    switch (enumVal)
    {
        case 1:
            pData->compressor.gainFunction.gainReduction = 2.0f;
            break;
        case 2:
            pData->compressor.gainFunction.gainReduction = 4.0f;
            break;
        case 3:
            pData->compressor.gainFunction.gainReduction = 8.0f;
            break;
        case 4:
            pData->compressor.gainFunction.gainReduction = 16.0f;
            break;
        default:
            pData->compressor.gainFunction.gainReduction = 1000.0f;
            break;
    }
}

__attribute__ ((section (".qspi_code")))
static void fxProgramP3Display(void*data,char*res)
{
    FxProgram8DataType * pData=(FxProgram8DataType*)data;
    uint16_t c=0,cres=0;
    const char* infDisplay="Inf          ";
    int32_t gainReductionInt = (int32_t)pData->compressor.gainFunction.gainReduction;
    *(res+cres++)='1';
    *(res+cres++)=':';
    if (gainReductionInt > 16)
    {
        while(*(infDisplay+c) !=0)
        {
            *(res+cres++)=*(infDisplay+c++);
        }
    }
    else
    {
        cres +=  decimalInt16ToChar((int16_t)(gainReductionInt*10),res+cres,1);
        for (c=0;c<16-cres;c++)
        {
            *(res+cres++)=' ';
        }
    }
}

__attribute__ ((section (".qspi_code")))
static void fxProgramP4Callback(uint16_t val,void*data) 
{
    FxProgram8DataType * pData=(FxProgram8DataType*)data;
    pData->compressor.gainFunction.threshhold = ((float)val)/4095.0f*0.99f+0.01f;
    fxProgram8.parameters[3].rawValue = val;
}

__attribute__ ((section (".qspi_code")))
static void fxProgramP4Display(void*data,char*res)
{
    int16_t dbval;
    FxProgram8DataType * pData=(FxProgram8DataType*)data;
    dbval = 20.0f* fln(pData->compressor.gainFunction.threshhold)/fln(10.0f);
    decimalInt16ToChar(dbval,res,1);
    appendToString(res," dB");
}

__attribute__ ((section (".qspi_code")))
static void fxProgramP5Callback(uint16_t val,void*data) 
{
    FxProgram8DataType * pData=(FxProgram8DataType*)data;
    pData->makeupGain.gain = (float)val/4095.0f + 1.0f;
    fxProgram8.parameters[4].rawValue = val;
}

__attribute__ ((section (".qspi_code")))
static void fxProgramP5Display(void*data,char*res)
{
    FxProgram8DataType * pData=(FxProgram8DataType*)data;
    uint32_t dval;
    dval = pData->makeupGain.gain*100.0f;
    decimalInt16ToChar((int16_t)dval,res,2);
}

__attribute__ ((section (".qspi_code")))
static void fxProgramP6Callback(uint16_t val,void*data) 
{
    uint8_t intermVal;
    FxProgram8DataType * pData=(FxProgram8DataType*)data;
    intermVal = val >> 10;
    if (intermVal == 3)
    {
        intermVal = 2;
    }
    pData->compressorType = intermVal;
    fxProgram8.parameters[5].rawValue = val;
}

__attribute__ ((section (".qspi_code")))
static void fxProgramP6Display(void*data,char*res)
{
    FxProgram8DataType * pData=(FxProgram8DataType*)data;
    *res=0;
    switch (pData->compressorType)
    {
        case 0:
            appendToString(res,"Dirty");
            break;
        case 1:
            appendToString(res,"Snappy");
            break;
        case 2:
            appendToString(res,"Pumpy");
            break;
    }
}


FxProgram8DataType fxProgram8Data =
{
    .compressor.avgLowpass.alphaFalling = 10.0f/32768.0f,
    .compressor.avgLowpass.alphaRising = 10.0f/32768.0f,
    .compressor.avgLowpass.oldVal = 0.0f,
    .compressor.avgLowpass.oldXVal = 0.0f,
    .compressor.currentAvg = 0.0f,
    .compressor.gainFunction.gainReduction = 2.0f,
    .compressor.gainFunction.threshhold = 1.0f,
    .makeupGain.gain = 1.0f,
    .makeupGain.offset = 0.0f
};

__attribute__ ((section (".qspi_code")))
static void fxProgramReset(void*data)
{
    FxProgram8DataType * pData=(FxProgram8DataType*)data;
    compressorReset(&pData->compressor);
} 


FxProgramType fxProgram8 = {
    .data = (void*)&fxProgram8Data,
    .name = "Compressor",
    .nParameters=6,
    .parameters = {
        {
            .name="Threshhold     ",
            .control=0x0,
            .getParameterDisplay=&fxProgramP4Display,
            .setParameter=&fxProgramP4Callback,
            .increment=32,
            .rawValue=0
        },
        {
            .name="Ratio          ",
            .control=1,
            .getParameterDisplay=&fxProgramP3Display,
            .setParameter=&fxProgramP3Callback,
            .increment=512,
            .rawValue=0
        },
        {
            .name="Makeup Gain    ",
            .control=2,
            .getParameterDisplay=&fxProgramP5Display,
            .setParameter=&fxProgramP5Callback,
            .increment=32,
            .rawValue=0,
        },
        {
            .name="Attack         ",
            .control=0xff,
            .getParameterDisplay=&fxProgramP1Display,
            .setParameter=&fxProgramP1Callback,
            .increment=32,
            .rawValue=0,
        },
        {
            .name="Release        ",
            .control=0xff,
            .getParameterDisplay=&fxProgramP2Display,
            .setParameter=&fxProgramP2Callback,
            .increment=32,
            .rawValue=0,
        },
        {
            .name="Flavor        ",
            .control=0xff,
            .getParameterDisplay=&fxProgramP6Display,
            .setParameter=&fxProgramP6Callback,
            .increment=1024,
            .rawValue=0
        }
    },
    .processSample=&fxProgramProcessSample,
    .reset=&fxProgramReset,
    .setup=0
};

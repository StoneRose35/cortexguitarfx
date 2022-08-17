#include "audio/fxprogram/fxProgram.h"
#include "stringFunctions.h"

static int16_t fxProgram6processSample(int16_t sampleIn,void*data)
{
    FxProgram6DataType* pData= (FxProgram6DataType*)data;
    return delayLineProcessSample(sampleIn, pData->delay);
}

static void fxProgram6Param1Callback(uint16_t val,void*data) // Delay Time
{
    FxProgram6DataType* pData= (FxProgram6DataType*)data;
    int32_t wVal;
    wVal = val;
    wVal <<= 4;
    pData->delay->delayInSamples = wVal; //pData->delay->delayInSamples + ((FXPROGRAM6_DELAY_TIME_LOWPASS_T*(wVal - pData->delay->delayInSamples)) >> 8);
}

static void fxProgram6Param1Display(void*data,char*res)
{
    int16_t dval;
    FxProgram6DataType* pData = (FxProgram6DataType*)data;
    dval = pData->delay->delayInSamples/48; // in ms
    Int16ToChar(dval,res);
    for (uint8_t c=0;c<PARAMETER_NAME_MAXLEN-2;c++)
    {
        if(*(res+c)==0)
        {
            *(res+c)='m';
            *(res+c+1)='s';
            *(res+c+2)=(char)0;
            break;
        }
    }
}

static void fxProgram6Param2Callback(uint16_t val,void*data) // Feedback
{
    FxProgram6DataType* pData= (FxProgram6DataType*)data;
    uint32_t wVal;
    wVal = val;
    wVal <<= 3;
    pData->delay->feedback=(int16_t)wVal;
}

static void fxProgram6Param2Display(void*data,char*res)
{
    FxProgram6DataType* pData = (FxProgram6DataType*)data;
    Int16ToChar(pData->delay->feedback/328,res);
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

static void fxProgram6Param3Callback(uint16_t val,void*data) // Mix
{
    FxProgram6DataType* pData= (FxProgram6DataType*)data;
    int16_t wVal;
    wVal = val;
    wVal <<= 3;
    pData->delay->mix = wVal;
}

static void fxProgram6Param3Display(void*data,char*res)
{
    FxProgram6DataType* pData = (FxProgram6DataType*)data;
    Int16ToChar(pData->delay->mix/328,res);
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

static void fxProgram6Setup(void*data)
{
    FxProgram6DataType* pData= (FxProgram6DataType*)data;
    pData->delay = getDelayData();
    initDelay(pData->delay);
}

FxProgram6DataType fxProgram6data;

FxProgramType fxProgram6 = {
    .name = "Delay                ",
    .nParameters=3,
    .parameters = {
        {
            .name = "Time           ",
            .control=0,
            .increment=64,
            .rawValue=0,
            .getParameterDisplay=&fxProgram6Param1Display,
            .getParameterValue=0,
            .setParameter=&fxProgram6Param1Callback
        },
        {
            .name = "Feedback       ",
            .control=1,
            .increment=64,
            .rawValue=0,
            .getParameterDisplay=&fxProgram6Param2Display,
            .getParameterValue=0,
            .setParameter=&fxProgram6Param2Callback
        },
        {
            .name = "Mix            ",
            .control=2,
            .increment=64,
            .rawValue=0,
            .getParameterDisplay=&fxProgram6Param3Display,
            .getParameterValue=0,
            .setParameter=&fxProgram6Param3Callback
        }
    },
    .processSample = &fxProgram6processSample,
    .setup = &fxProgram6Setup,
    .data = (void*)&fxProgram6data
};
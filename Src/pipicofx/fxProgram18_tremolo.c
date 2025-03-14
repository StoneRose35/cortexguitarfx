#include "pipicofx/fxPrograms.h"
#include "audio/audiotools.h"
#include "audio/phaseDistortedSineSquare.h"
#include "stringFunctions.h"
#include "romfunc.h"

int16_t fxProgram18processSample(int16_t sampleIn,void*data)
{
    FxProgram18DataType* pData= (FxProgram18DataType*)data;

    return tremoloProcessSample(sampleIn,&pData->tremolo);
}

static void fxProgramParam1Callback(uint16_t val,void*data) // Rate
{
    FxProgram18DataType* pData= (FxProgram18DataType*)data;
    float rateVal = ((float)val)/200.f + 0.2f;
    phaseDistortedSineSquareSetFrequency(rateVal,&pData->tremolo.modulator);
    fxProgram18.parameters[0].rawValue = val;
}

static void fxProgramParam1Display(void*data,char*res)
{
    FxProgram18DataType* pData= (FxProgram18DataType*)data;
    float f = phaseDistortedSineSquareGetFrequency(&pData->tremolo.modulator)*8.0f;
    uint16_t intf = (uint16_t)float2int(f);
    fixedPointInt16ToChar(res,intf,3);
    appendToString(res," Hz");

}

static void fxProgramParam2Callback(uint16_t val,void*data) // Depth
{
    FxProgram18DataType* pData= (FxProgram18DataType*)data;
    pData->tremolo.depth = val << 3;
    fxProgram18.parameters[1].rawValue = val;
}

static void fxProgramParam2Display(void*data,char*res)
{
    FxProgram18DataType* pData= (FxProgram18DataType*)data;
    int16_t depth = pData->tremolo.depth;
    Int16ToChar(depth/328,res);
    appendToString(res,"%");
}


static void fxProgramParam3Callback(uint16_t val,void*data) // Shape
{
    FxProgram18DataType* pData= (FxProgram18DataType*)data;
    pData->tremolo.modulator.squareRatio = val >> 4;
    fxProgram18.parameters[2].rawValue = val;
}

static void fxProgramParam3Display(void*data,char*res)
{
    FxProgram18DataType* pData= (FxProgram18DataType*)data;
    Int16ToChar(pData->tremolo.modulator.squareRatio,res);
}

static void fxProgramParam4Callback(uint16_t val,void*data) // Pulse Width
{
    FxProgram18DataType* pData= (FxProgram18DataType*)data;
    pData->tremolo.modulator.pulseWidth = ((int16_t)val - 2048) << 4;
    phaseDistortedSineSquarePulseWidth(pData->tremolo.modulator.pulseWidth,&pData->tremolo.modulator);
    fxProgram18.parameters[3].rawValue = val;
}

static void fxProgramParam4Display(void*data,char*res)
{
    FxProgram18DataType* pData= (FxProgram18DataType*)data;
    Int16ToChar(pData->tremolo.modulator.pulseWidth,res);
}

FxProgram18DataType fxProgram18data=
{
    .tremolo.currentLfoVal=0,
    .tremolo.depth=0,
    .tremolo.modulator.currentPhase=0,
    .tremolo.modulator.phaseIncrement=131,
    .tremolo.modulator.phaseIncrementCorrection1=0,
    .tremolo.modulator.pulseWidth=0,
    .tremolo.modulator.squareRatio=0
};

void fxProgram18Setup(void*data)
{
    FxProgram18DataType * pData = (FxProgram18DataType*)data;
    initTremolo(&pData->tremolo);
}

FxProgramType fxProgram18 = {
    .name = "Tremolo",
    .nParameters=4,
    .parameters = {
        {
            .name="Rate",
            .control=0,
            .increment=16,
            .rawValue=0,
            .getParameterDisplay=&fxProgramParam1Display,
            .getParameterValue=0,
            .setParameter=&fxProgramParam1Callback
        },
        {
            .name="Depth",
            .control=1,
            .increment=16,
            .rawValue=0,
            .getParameterDisplay=&fxProgramParam2Display,
            .getParameterValue=0,
            .setParameter=&fxProgramParam2Callback
        },
        {
            .name="Shape",
            .control=2,
            .increment=16,
            .rawValue=0,
            .getParameterDisplay=&fxProgramParam3Display,
            .getParameterValue=0,
            .setParameter=&fxProgramParam3Callback
        },
        {
            .name="PulseWidth",
            .control=255,
            .increment=16,
            .rawValue=0,
            .getParameterDisplay=&fxProgramParam4Display,
            .getParameterValue=0,
            .setParameter=&fxProgramParam4Callback
        }
    },
    .processSample = &fxProgram18processSample,
    .setup = &fxProgram18Setup,
    .reset = 0,
    .data = (void*)&fxProgram18data
};

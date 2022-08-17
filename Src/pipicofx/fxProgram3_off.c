#include "audio/fxprogram/fxProgram.h"

int16_t fxProgram3processSample(int16_t sampleIn,void*data)
{
    return sampleIn;
}

void fxProgram3Setup(void*data)
{}

FxProgramType fxProgram3 = {
    .name = "Off                  ",
    .nParameters=0,
    .processSample = &fxProgram3processSample,
    .setup = &fxProgram3Setup,
    .data = (void*)0
};

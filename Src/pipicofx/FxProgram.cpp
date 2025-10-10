#include "pipicofx/FxProgram.hpp"
#include "memoryRegions.h"

using namespace PiPicoFX;

__QSPI_CODE
PiPicoFX::FxProgram::FxProgram(uint8_t nParams,const char * name,uint32_t memUseage)
{
    nParameters=nParams;
    paramCnt=0;
    programName = name;
    memoryUseage = memUseage;
    parameters = new FxProgramParameter*[nParameters];
}

__QSPI_CODE
PiPicoFX::FxProgram::~FxProgram()
{
    for (uint8_t c=0;c<this->nParameters;c++)
    {
        delete parameters[c];
    }
    delete parameters;
}

__QSPI_CODE
void FxProgram::setup(void){}

__QSPI_CODE
uint8_t FxProgram::addParameter(FxProgramParameter*p){

    if (paramCnt < nParameters)
    {
        parameters[paramCnt++] = p;
        return 0;
    }
    return 1;
}

__QSPI_CODE
uint8_t FxProgram::getParameterCount(void){
    return nParameters;
}

__QSPI_CODE
FxProgramParameter* FxProgram::getParameter(uint8_t pos){
    if (pos < nParameters)
    {
        return parameters[pos];
    }
    return nullptr;
}

__QSPI_CODE
float FxProgram::processSample(float sampleIn)
{
    return sampleIn;
}

__QSPI_CODE
uint32_t FxProgram::getDelayMemoryUseage(void)
{
    return this->memoryUseage;
}

__QSPI_CODE
const char * FxProgram::getName()
{
    return programName;
}
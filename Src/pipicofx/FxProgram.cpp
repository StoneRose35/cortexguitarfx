#include "pipicofx/FxProgram.hpp"

using namespace PiPicoFX;

PiPicoFX::FxProgram::FxProgram(uint8_t nParams,const char * name)
{
    nParameters=nParams;
    paramCnt=0;
    programName = name;
    parameters = new FxProgramParameter*[nParameters];
}


void FxProgram::setup(void){}
uint8_t FxProgram::addParameter(FxProgramParameter*p){

    if (paramCnt < nParameters)
    {
        parameters[paramCnt++] = p;
        return 0;
    }
    return 1;
}
uint8_t FxProgram::getParameterCount(void){
    return nParameters;
}
FxProgramParameter* FxProgram::getParameter(uint8_t pos){
    if (pos < nParameters)
    {
        return parameters[pos];
    }
    return nullptr;
}

int16_t FxProgram::processSample(int16_t sampleIn)
{
    return sampleIn;
}

const char * FxProgram::getName()
{
    return programName;
}
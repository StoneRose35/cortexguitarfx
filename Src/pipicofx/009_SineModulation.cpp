
#include "pipicofx/009_SineModulation.hpp"
extern "C" {
#include "stringFunctions.h"
#include "audio/gainstage.h"
}
using namespace PiPicoFX;

int16_t SineModulation::SineModulation::processSample(int16_t sampleIn)
{
    // TODO implement audio processor
    return 0;
}

void SineModulation::SineModulation::setup()
{
    // TODO complete setup function
    this->addParameter(new Param1(this));
    this->addParameter(new Param2(this));
    this->addParameter(new Param3(this));
    this->addParameter(new Param4(this));
    this->addParameter(new Param5(this));
    this->addParameter(new Param6(this));

}

void SineModulation::Param1::parameterCallback(uint16_t val)
{
    // TODO affect the state of the fx program in any way based on 'val'
    this->rawValue = val; 
}

void SineModulation::Param1::parameterDisplay(char*res)
{
    // TODO put some content descripting the actual parameter value in res
}
void SineModulation::Param2::parameterCallback(uint16_t val)
{
    // TODO affect the state of the fx program in any way based on 'val'
    this->rawValue = val; 
}

void SineModulation::Param2::parameterDisplay(char*res)
{
    // TODO put some content descripting the actual parameter value in res
}
void SineModulation::Param3::parameterCallback(uint16_t val)
{
    // TODO affect the state of the fx program in any way based on 'val'
    this->rawValue = val; 
}

void SineModulation::Param3::parameterDisplay(char*res)
{
    // TODO put some content descripting the actual parameter value in res
}
void SineModulation::Param4::parameterCallback(uint16_t val)
{
    // TODO affect the state of the fx program in any way based on 'val'
    this->rawValue = val; 
}

void SineModulation::Param4::parameterDisplay(char*res)
{
    // TODO put some content descripting the actual parameter value in res
}
void SineModulation::Param5::parameterCallback(uint16_t val)
{
    // TODO affect the state of the fx program in any way based on 'val'
    this->rawValue = val; 
}

void SineModulation::Param5::parameterDisplay(char*res)
{
    // TODO put some content descripting the actual parameter value in res
}
void SineModulation::Param6::parameterCallback(uint16_t val)
{
    // TODO affect the state of the fx program in any way based on 'val'
    this->rawValue = val; 
}

void SineModulation::Param6::parameterDisplay(char*res)
{
    // TODO put some content descripting the actual parameter value in res
}
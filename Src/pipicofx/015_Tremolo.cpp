
#include "pipicofx/015_Tremolo.hpp"
extern "C" {
#include "stringFunctions.h"
#include "audio/gainstage.h"
}
using namespace PiPicoFX;

int16_t Tremolo::Tremolo::processSample(int16_t sampleIn)
{
    // TODO implement audio processor
    return 0;
}

void Tremolo::Tremolo::setup()
{
    // TODO complete setup function
    this->addParameter(new Param1(this));
    this->addParameter(new Param2(this));
    this->addParameter(new Param3(this));
    this->addParameter(new Param4(this));
    this->addParameter(new Param5(this));

}

void Tremolo::Param1::parameterCallback(uint16_t val)
{
    // TODO affect the state of the fx program in any way based on 'val'
    this->rawValue = val; 
}

void Tremolo::Param1::parameterDisplay(char*res)
{
    // TODO put some content descripting the actual parameter value in res
}
void Tremolo::Param2::parameterCallback(uint16_t val)
{
    // TODO affect the state of the fx program in any way based on 'val'
    this->rawValue = val; 
}

void Tremolo::Param2::parameterDisplay(char*res)
{
    // TODO put some content descripting the actual parameter value in res
}
void Tremolo::Param3::parameterCallback(uint16_t val)
{
    // TODO affect the state of the fx program in any way based on 'val'
    this->rawValue = val; 
}

void Tremolo::Param3::parameterDisplay(char*res)
{
    // TODO put some content descripting the actual parameter value in res
}
void Tremolo::Param4::parameterCallback(uint16_t val)
{
    // TODO affect the state of the fx program in any way based on 'val'
    this->rawValue = val; 
}

void Tremolo::Param4::parameterDisplay(char*res)
{
    // TODO put some content descripting the actual parameter value in res
}
void Tremolo::Param5::parameterCallback(uint16_t val)
{
    // TODO affect the state of the fx program in any way based on 'val'
    this->rawValue = val; 
}

void Tremolo::Param5::parameterDisplay(char*res)
{
    // TODO put some content descripting the actual parameter value in res
}
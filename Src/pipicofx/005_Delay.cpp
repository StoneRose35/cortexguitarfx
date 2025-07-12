
#include "pipicofx/005_Delay.hpp"
extern "C" {
#include "stringFunctions.h"
#include "audio/gainstage.h"
}
using namespace PiPicoFX;

int16_t Delay::Delay::processSample(int16_t sampleIn)
{
    // TODO implement audio processor
    return 0;
}

void Delay::Delay::setup()
{
    // TODO complete setup function
    this->addParameter(new Param1(this));
    this->addParameter(new Param2(this));
    this->addParameter(new Param3(this));
    this->addParameter(new Param4(this));

}

void Delay::Param1::parameterCallback(uint16_t val)
{
    // TODO affect the state of the fx program in any way based on 'val'
    this->rawValue = val; 
}

void Delay::Param1::parameterDisplay(char*res)
{
    // TODO put some content descripting the actual parameter value in res
}
void Delay::Param2::parameterCallback(uint16_t val)
{
    // TODO affect the state of the fx program in any way based on 'val'
    this->rawValue = val; 
}

void Delay::Param2::parameterDisplay(char*res)
{
    // TODO put some content descripting the actual parameter value in res
}
void Delay::Param3::parameterCallback(uint16_t val)
{
    // TODO affect the state of the fx program in any way based on 'val'
    this->rawValue = val; 
}

void Delay::Param3::parameterDisplay(char*res)
{
    // TODO put some content descripting the actual parameter value in res
}
void Delay::Param4::parameterCallback(uint16_t val)
{
    // TODO affect the state of the fx program in any way based on 'val'
    this->rawValue = val; 
}

void Delay::Param4::parameterDisplay(char*res)
{
    // TODO put some content descripting the actual parameter value in res
}